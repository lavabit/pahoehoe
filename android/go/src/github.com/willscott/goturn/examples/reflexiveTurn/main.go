// Package main Example relays a message through a TURN server using the
// low-level interface.
package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"github.com/willscott/goturn"
	"github.com/willscott/goturn/stun"
	"github.com/willscott/goturn/turn"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"net/url"
	"time"
)

var credentialURL = flag.String("credentials", "https://computeengineondemand.appspot.com/turn?username=prober&key=4080218913", "credential URL")

type Credentials struct {
	Username string   `json:"username"`
	Password string   `json:"password"`
	Uris     []string `json:"uris"`
}

func main() {
	flag.Parse()

	// get & parse credentials
	resp, err := http.Get(*credentialURL)
	if err != nil {
		log.Fatal("Could not fetch URL:", err)
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Fatal("Could not read response:", err)
	}

	var creds Credentials
	if err := json.Unmarshal(body, &creds); err != nil {
		log.Fatal("Could not parse response:", err)
	}

	// Use the first credential provided.
	server, err := url.Parse(creds.Uris[0])
	if err != nil {
		log.Fatal("Invalid server URI:", err)
	}

	log.Printf("Negotiating with %s", server.Opaque)

	// dial
	raddr, err := net.ResolveUDPAddr("udp", server.Opaque)
	if err != nil {
		log.Fatal("Could resolve remote address:", err)
	}

	c, err := net.ListenUDP("udp", &net.UDPAddr{IP: net.IPv4(0, 0, 0, 0), Port: 0})
	if err != nil {
		log.Fatal("Could open UDP Connection:", err)
	}
	defer c.Close()

	// construct allocate message
	packet, err := goturn.NewAllocateRequest("udp", false)
	if err != nil {
		log.Fatal("Failed to generate request packet:", err)
	}

	message, err := packet.Serialize()
	if err != nil {
		log.Fatal("Failed to serialize packet: ", err)
	}

	// send message
	_, err = c.WriteToUDP(message, raddr)
	if err != nil {
		log.Fatal("Failed to send message: ", err)
	}

	// listen for response
	c.SetReadDeadline(time.Now().Add(1000 * time.Millisecond))
	b := make([]byte, 2048)
	n, err := c.Read(b)
	if err != nil || n == 0 || n > 2048 {
		log.Fatal("Failed to read response: ", err)
	}

	// response is going to tell us we're unauthorized, but will provide
	// a nonce and realm.
	response, err := goturn.ParseTurn(b[0:n], nil)
	if err != nil {
		log.Fatal("Could not parse response as a STUN message:", err)
	}

	if response.Header.Type != goturn.AllocateError {
		log.Fatal("Response message was not requesting credentials", response.Header)
	}

	// Allocate with credentials
	packet, err = goturn.NewAllocateRequest("udp", true)
	if err != nil {
		log.Fatal("Failed to generate request packet:", err)
	}
	credentials := response.Credentials
	credentials.Username = creds.Username
	credentials.Password = creds.Password
	packet.Credentials = credentials

	message, err = packet.Serialize()
	if err != nil {
		log.Fatal("Failed to serialize packet: ", err)
	}

	// send message
	_, err = c.WriteToUDP(message, raddr)
	if err != nil {
		log.Fatal("Failed to send message: ", err)
	}

	// listen for response
	c.SetReadDeadline(time.Now().Add(1000 * time.Millisecond))
	n, err = c.Read(b)
	if err != nil || n == 0 || n > 2048 {
		log.Fatal("Failed to read response: ", err)
	}

	authResponse, err := goturn.ParseTurn(b[0:n], &packet.Credentials)
	if err != nil {
		log.Fatal("Could not parse authorized AllocateResponse: ", err)
	}

	if authResponse.Header.Type != goturn.AllocateResponse {
		log.Fatal("Response message was not responding to allocation: ", authResponse.Header)
	}
	log.Printf("Authenticated and granted Port allocation.")

	// Request to send back to ourselves.
	mappedAddr := authResponse.GetAttribute(stun.XorMappedAddress)
	myReflexiveAddress := (*mappedAddr).(*stun.XorMappedAddressAttribute)
	peerAddr, err := net.ResolveUDPAddr("udp", myReflexiveAddress.String())
	packet, err = goturn.NewPermissionRequest(peerAddr)
	packet.Credentials = credentials

	message, err = packet.Serialize()
	if err != nil {
		log.Fatal("Failed to serialize packet: ", err)
	}

	// send message
	_, err = c.WriteToUDP(message, raddr)
	if err != nil {
		log.Fatal("Failed to send message: ", err)
	}

	// listen for response
	c.SetReadDeadline(time.Now().Add(1000 * time.Millisecond))
	n, err = c.Read(b)
	if err != nil || n == 0 || n > 2048 {
		log.Fatal("Failed to read response: ", err)
	}

	permissionResponse, err := goturn.ParseTurn(b[0:n], &packet.Credentials)
	if err != nil {
		log.Fatal("Could not parse PermissionResponse: ", err)
	}

	if permissionResponse.Header.Type != goturn.CreatePermissionResponse {
		log.Fatal("Response message was not okay with permission request: ", permissionResponse.Header)
	}
	log.Printf("Granted Permission to send to %s.", myReflexiveAddress.Address)

	// Send some data.
	packet, err = goturn.NewSendIndication(myReflexiveAddress.Address, myReflexiveAddress.Port, []byte("Hello World."))
	message, err = packet.Serialize()
	if err != nil {
		log.Fatal("Failed to serialize packet: ", err)
	}

	// Figure out relay address
	relayAddr := authResponse.GetAttribute(turn.XorRelayedAddress)
	relayAddress := (*relayAddr).(*turn.XorRelayedAddressAttribute)
	relayUDPAddr, _ := net.ResolveUDPAddr("udp", relayAddress.String())
	_, err = c.WriteToUDP(message, relayUDPAddr)
	if err != nil {
		log.Fatal("Failed to send message: ", err)
	}

	// listen for response
	c.SetReadDeadline(time.Now().Add(1000 * time.Millisecond))
	n, err = c.Read(b)
	if err != nil || n == 0 || n > 2048 {
		log.Fatal("Failed to read response: ", err)
	}

	// Response should be a data indication.
	dataResponse, err := goturn.ParseTurn(b[0:n], &packet.Credentials)
	if err != nil {
		log.Fatal("Could not parse Data Indication: ", err)
	}

	if dataResponse.Header.Type != goturn.DataIndication {
		log.Fatal("Did not Receive data after sending: ", dataResponse.Header)
	}
	dataPtr := dataResponse.GetAttribute(turn.Data)
	if dataPtr == nil {
		log.Fatal("No Data Attribute in send response: ", dataResponse.Header)
	}
	dataAttr := (*dataPtr).(*turn.DataAttribute)
	if bytes.Compare(dataAttr.Data, message) == 0 {
		log.Printf("Successfully sent and received \"hello world\".")
	} else {
		log.Fatal("Received data didn't match what was expected. Got: %s.", dataAttr.Data)
	}
}
