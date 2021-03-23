// Package client provides a high-level interface for connecting and querying STUN and TURN services.
package client

import (
	"bufio"
	"errors"
	"github.com/willscott/goturn"
	"github.com/willscott/goturn/common"
	stunattrs "github.com/willscott/goturn/stun"
	turnattrs "github.com/willscott/goturn/turn"
	"net"
	"time"
)

// StunClient maintains state on a connection with a stun/turn server.
// New StunClient's should be created either by wrapping an existing net.Conn
// Connection to a Stun Server (as shown in the getIP and reflexiveTurn
// examples), or can be implicitly created through the Dialer interface.
type StunClient struct {
	// The connection transport for communication with the server. This connection
	// comes from net.Dial, and can be over UDP, TCP, or TCP over TLS as supported
	// by the server.
	net.Conn

	// A buffered reader is used to read from the connection. All calls to read
	// will be called on reader. if reader is nil, a new bufio.Reader will be
	// created to wrap the current net.Conn.
	reader *bufio.Reader

	// A net.Dialer is used to provide additional flexibility when Connect() is
	// called, since a new connection to the server will need to be created. The
	// StunClient defaults to using the same transport method for these subsequent
	// connections, but this can be further specified with a custom Dialer.
	*net.Dialer

	// Credentials used for authenticating communication with the server.
	*stun.Credentials

	// Timeout until the active connection expires.
	Timeout time.Duration

	// Time until the next message must be received.
	Deadline time.Time
}

// deriveConnection creates a new connection to the same remote endpoint,
// sharing credentials with the current connection.
func (s *StunClient) deriveConnection() (*StunClient, error) {
	other := new(StunClient)
	other.Dialer = s.Dialer
	other.Credentials = s.Credentials.ForNewConnection()
	other.Timeout = s.Timeout

	conn, err := s.Dialer.Dial(s.Conn.RemoteAddr().Network(), s.Conn.RemoteAddr().String())
	if err != nil {
		return nil, err
	}
	other.Conn = conn
	return other, nil
}

// send transmits a message from the client.
func (s *StunClient) send(packet *stun.Message, err error) error {
	if err != nil {
		return err
	}
	packet.Credentials = *s.Credentials

	message, err := packet.Serialize()
	if err != nil {
		return err
	}

	// send message
	// TODO: handle not all of message being written.
	if _, err = s.Conn.Write(message); err != nil {
		return err
	}
	return nil
}

// readStunPacket Reads the next packet off of the connection abstracted by the
// client.
// Returns either the next message, or an error if the next set of bytes
// do not represent a valid message.
func (s *StunClient) readStunPacket() (*stun.Message, error) {
	// Set up timeouts for reading.
	if s.reader == nil {
		s.reader = bufio.NewReader(s.Conn)
	}
	if s.Timeout > 0 {
		// Initial Deadline
		if s.Deadline.IsZero() {
			s.Deadline = time.Now().Add(s.Timeout)
		}
		s.Conn.SetReadDeadline(s.Deadline)
	}

	// Start by reading the header to learn the length of the packet.
	h, err := s.reader.Peek(20)
	if err != nil {
		return nil, err
	}
	header := stun.Header{}
	if err = header.Decode(h); err != nil {
		return nil, err
	}
	if header.Length == 0 {
		return goturn.ParseTurn(h, s.Credentials)
	}
	if header.Length > 2048 {
		return nil, errors.New("Packet length too long.")
	}
	buffer := make([]byte, 20+header.Length)
	n, err := s.reader.Read(buffer)
	if err != nil || uint16(n) != 20+header.Length {
		return nil, err
	}

	if s.Timeout > 0 {
		s.Deadline = time.Now().Add(s.Timeout)
	}

	return goturn.ParseTurn(buffer, s.Credentials)
}

// Bind Requests a Stun "Binding" to retrieve the Internet-visible address of
// the connection with the server. This is the function provided by the STUN
// RFC - to learn how a remote machine sees an active UDP connection created
// by the client, so that you can offer that endpoint for other machines to
// connect to.
//
// A trivial Binding usage would be:
//
//  // Wrap a connection with a StunClient.
//  client := client.StunClient{Conn: c}
//
//  // Request the Binding.
//  address, err := client.Bind()
//  if err != nil {
//    log.Fatal("Failed bind:", err)
//  }
//
//  fmt.Printf("My address is: %s", address.String())
func (s *StunClient) Bind() (net.Addr, error) {
	// construct a binding request message
	packet, err := goturn.NewBindingRequest()
	if err != nil {
		return nil, err
	}

	message, err := packet.Serialize()
	if err != nil {
		return nil, err
	}

	// send the message and read the response
	if _, err = s.Conn.Write(message); err != nil {
		return nil, err
	}

	response, err := s.readStunPacket()
	if err != nil {
		return nil, err
	}

	if response.Header.Type != goturn.BindingResponse {
		return nil, errors.New("Unexpected response type.")
	}
	attr := response.GetAttribute(stunattrs.MappedAddress)
	port := uint16(0)
	address := net.IP{}

  // extract the address if there is one.
	if attr != nil {
		addr := (*attr).(*stunattrs.MappedAddressAttribute)
		port = addr.Port
		address = addr.Address
	} else {
		attr = response.GetAttribute(stunattrs.XorMappedAddress)
		if attr == nil {
			return nil, errors.New("No Mapped Address provided.")
		}
		addr := (*attr).(*stunattrs.XorMappedAddressAttribute)
		port = addr.Port
		address = addr.Address
	}
	return stun.NewAddress(s.Conn.RemoteAddr().Network(), address, port), nil
}

// allocateUnauthenticated sends a TURN Allocate request (A request for
// permission to send and receive data through the TURN server), but without
// any credentials. By the RFC, this request must fail, but the error response
// is used to populate the Nonce and Realm used by the server, so that subsequent
// requests can be properly authenticated.
func (s *StunClient) allocateUnauthenticated() error {
	// make a simple allocation message
	creds := s.Credentials
	s.Credentials = &stun.Credentials{}
	if err := s.send(goturn.NewAllocateRequest(s.Conn.RemoteAddr().Network(), false)); err != nil {
		return err
	}
	s.Credentials = creds

	response, err := s.readStunPacket()
	if err != nil {
		return err
	}

	if response.Credentials.Nonce != nil {
		s.Credentials.Nonce = response.Credentials.Nonce
	}
	if len(response.Credentials.Realm) > 0 {
		s.Credentials.Realm = response.Credentials.Realm
	}
	msgerr := stunattrs.GetError(response)
	if msgerr.Error() > 0 && msgerr.Error() != 401 {
		return errors.New("Initial Connection failed " + msgerr.String())
	}
	return nil
}

// Allocate Requests to connect to a TURN server. The TURN protocol uses the
// term allocation to refer to an authenticated connection with the server.
// Returns the bound address.
func (s *StunClient) Allocate(c *stun.Credentials) (net.Addr, error) {
	s.Credentials = c

	if s.Credentials.Nonce == nil {
		if err := s.allocateUnauthenticated(); err != nil {
			return nil, err
		}
	}
	if err := s.send(goturn.NewAllocateRequest(s.Conn.RemoteAddr().Network(), true)); err != nil {
		return nil, err
	}
	response, err := s.readStunPacket()
	if err != nil {
		return nil, err
	}

	if response.Header.Type != goturn.AllocateResponse {
		msgerr := stunattrs.GetError(response)
		if msgerr.Error() == 442 {
			// TODO: bad transport; retry w/ other protocol.
		}
		return nil, errors.New("Connection failed: " + msgerr.String())
	}

	relayAddr := response.GetAttribute(turnattrs.XorRelayedAddress)
	relayAddress := (*relayAddr).(*turnattrs.XorRelayedAddressAttribute)

	return stun.NewAddress(s.Conn.RemoteAddr().Network(), relayAddress.Address, relayAddress.Port), nil
}

// RequestPermission secures permission to send data with a remote address. The
// Client should already have an authenticated connection with the server, using
// Allocate, for this request to succeed.
func (s *StunClient) RequestPermission(with net.Addr) error {
	addr := stun.Address{with}
	if err := s.send(goturn.NewPermissionRequest(addr.HostPart())); err != nil {
		return err
	}
	response, err := s.readStunPacket()
	if err != nil {
		return err
	}

	if response.Header.Type != goturn.CreatePermissionResponse {
		return errors.New("Connection failed: " + stunattrs.GetError(response).String())
	}
	return nil
}

// Connect creates a new connection to 'to', relayed through the TURN server.
// The remote address must be pre-negotiated using RequestPermission for the
// proxied connection to be permitted.
func (s *StunClient) Connect(to net.Addr) (net.Conn, error) {
	if err := s.send(goturn.NewConnectRequest(to)); err != nil {
		return nil, err
	}
	response, err := s.readStunPacket()
	if err != nil {
		return nil, err
	}

	if response.Header.Type != goturn.ConnectResponse {
		return nil, errors.New("Connection failed: " + stunattrs.GetError(response).String())
	}

	// extract Connection-id
	connID := response.GetAttribute(turnattrs.ConnectionId)
	if connID == nil {
		return nil, errors.New("No Connection ID provided.")
	}
	connectionID := (*connID).(*turnattrs.ConnectionIdAttribute).ConnectionId

	// create the data connection.
	conn, err := s.deriveConnection()
	if err != nil {
		return nil, err
	}

	if err := conn.send(goturn.NewConnectionBindRequest(connectionID)); err != nil {
		conn.Conn.Close()
		return nil, err
	}

	response, err = conn.readStunPacket()
	if err != nil {
		conn.Conn.Close()
		return nil, err
	}

	// Need to get nonce for the new connection first.
	if response.Credentials.Nonce != nil {
		conn.Credentials.Nonce = response.Credentials.Nonce
	}

	if err := conn.send(goturn.NewConnectionBindRequest(connectionID)); err != nil {
		conn.Conn.Close()
		return nil, err
	}

	response, err = conn.readStunPacket()
	if err != nil {
		conn.Conn.Close()
		return nil, err
	}

	if response.Header.Type != goturn.ConnectionBindResponse {
		conn.Conn.Close()
		return nil, errors.New("Connection failed: " + stunattrs.GetError(response).String())
	}

	return conn.Conn, nil
}
