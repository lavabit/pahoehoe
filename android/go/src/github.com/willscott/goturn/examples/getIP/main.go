// Package main Example requesting STUN binding from a remote server.
package main

import (
	"flag"
	"github.com/willscott/goturn/client"
	"log"
	"net"
)

var server = flag.String("server", "stun.l.google.com:19302", "Remote Stun Server")

func main() {
	flag.Parse()

	// Create a UDP connection with the specified server.
	c, err := net.Dial("udp", *server)
	if err != nil {
		log.Fatal("Could open UDP Connection:", err)
	}
	defer c.Close()

	// Wrap the connection with a StunClient.
	client := client.StunClient{Conn: c}

	// Request the STUN Binding.
	address, err := client.Bind()
	if err != nil {
		log.Fatal("Failed bind:", err)
	}

	log.Printf("My address is: %s", address.String())
}
