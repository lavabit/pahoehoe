// Package goturn is a native go language interface to STUN and TURN.
package goturn

import (
	"crypto/rand"

	common "github.com/willscott/goturn/common"
	"github.com/willscott/goturn/stun"
	"github.com/willscott/goturn/turn"

	"net"
)

// TURN (RFC 5766) defined message types.
const (
	AllocateRequest             common.HeaderType = 0x0003
	RefreshRequest                                = 0x0004
	CreatePermissionRequest                       = 0x0008
	ChannelBindRequest                            = 0x0009
	ConnectRequest                                = 0x000a
	ConnectionBindRequest                         = 0x000b
	SendIndication                                = 0x0016
	DataIndication                                = 0x0017
	ConnectionAttemptIndication                   = 0x001c
	AllocateResponse                              = 0x0103
	RefreshResponse                               = 0x0104
	CreatePermissionResponse                      = 0x0108
	ChannelBindResponse                           = 0x0109
	ConnectResponse                               = 0x010a
	ConnectionBindResponse                        = 0x010b
	AllocateError                                 = 0x0113
	RefreshError                                  = 0x0114
	CreatePermissionError                         = 0x0118
	ChannelBindError                              = 0x0119
	ConnectError                                  = 0x011a
	ConnectionBindError                           = 0x011b
)

// Deprecated: Should live in individual turn attribute implementations.
const (
	EvenPort         common.AttributeType = 0x18
	DontFragment                          = 0x1A
	ReservationToken                      = 0x22
)

// ParseTurn Parses data per the RFC 5766 TURN specification. Undefined attribute
// types will be left un-parsed. Credentials, when provided, are used to validate
// provided message integrity (used for authenticity) of the parsed message.
func ParseTurn(data []byte, credentials *common.Credentials) (*common.Message, error) {
	return common.Parse(data, credentials, turn.AttributeSet())
}

// newMsg creates a new STUN message with a new message ID.
func newMsg(htype common.HeaderType) (*common.Message, error) {
	message := common.Message{
		Header: common.Header{
			Type: htype,
		},
	}
	_, err := rand.Read(message.Header.Id[:])
	return &message, err
}

// NewAllocateRequest creates a new message requesting authorization with a
// remote server. The allocation request specifies the type of remote network
// that the client wishes interact with.
func NewAllocateRequest(network string, authenticated bool) (*common.Message, error) {
	message, err := newMsg(AllocateRequest)

	net := uint8(17)
	if network == "tcp" || network == "tcp4" || network == "tcp6" {
		net = 6
	}

	if authenticated == true {
		message.Attributes = []common.Attribute{&turn.RequestedTransportAttribute{net},
			&stun.NonceAttribute{},
			&stun.UsernameAttribute{},
			&stun.RealmAttribute{},
			&stun.MessageIntegrityAttribute{},
			&stun.FingerprintAttribute{}}
	} else {
		message.Attributes = []common.Attribute{&turn.RequestedTransportAttribute{net}}
	}

	return message, err
}

// NewPermissionRequest creates a message requesting permission from the server
// to allow sending and receiving data with a remote Address.
func NewPermissionRequest(to net.Addr) (*common.Message, error) {
	message, err := newMsg(CreatePermissionRequest)

	message.Attributes = []common.Attribute{&stun.NonceAttribute{},
		&stun.UsernameAttribute{},
		&stun.RealmAttribute{},
		&stun.MessageIntegrityAttribute{},
		&stun.FingerprintAttribute{}}

	turn.AddXorPeerAddressAttribute(message, to)

	return message, err
}

// NewConnectRequest creates a message representing a request to create a new
// TCP connection for exchanging data with a remote address, Per RFC 6062.
func NewConnectRequest(to net.Addr) (*common.Message, error) {
	message, err := newMsg(ConnectRequest)

	message.Attributes = []common.Attribute{
		&stun.NonceAttribute{},
		&stun.UsernameAttribute{},
		&stun.RealmAttribute{},
		&stun.MessageIntegrityAttribute{},
		&stun.FingerprintAttribute{}}

	turn.AddXorPeerAddressAttribute(message, to)

	return message, err
}

// NewConnectionBindRequest creates a message representing a request to turn
// the current connection with the server into a TCP connection relayed to a
// remote peer specified by a previously generated ConnectionID. Per RFC 6062.
func NewConnectionBindRequest(connectionID uint32) (*common.Message, error) {
	message, err := newMsg(ConnectionBindRequest)

	message.Attributes = []common.Attribute{
		&turn.ConnectionIdAttribute{connectionID},
		&stun.NonceAttribute{},
		&stun.UsernameAttribute{},
		&stun.RealmAttribute{},
		&stun.MessageIntegrityAttribute{},
		&stun.FingerprintAttribute{},
	}
	return message, err
}

// NewSendIndication creates a message representing a request to send a message
// of data over an existing allocation.
func NewSendIndication(host net.IP, port uint16, data []byte) (*common.Message, error) {
	message, err := newMsg(SendIndication)

	family := uint16(1)
	if host.To4() == nil {
		family = 2
	}
	message.Attributes = []common.Attribute{&turn.XorPeerAddressAttribute{family, port, host},
		&turn.DataAttribute{data}}

	return message, err
}
