// Package stun represents the common data structures used in creating and parsing STUN messages.
package stun

import (
	"fmt"
	"net"
)

// Address represents an Internet host and port
type Address struct {
	net.Addr
}

// HostPart provides an address representing the same host, but with a port of 0.
func (a *Address) HostPart() net.Addr {
	host, _, err := net.SplitHostPort(a.String())
	if err != nil {
		return a
	} else {
		addr, _ := net.ResolveIPAddr("ip", host)
		return addr
	}
}

// Family provides the STUN family of an address. A value of 1 if IPv4 and 2 if IPv6
func (a *Address) Family() uint16 {
	switch a.Network() {
	case "ip6", "udp6", "tcp6":
		return uint16(2)
	default:
		return uint16(1)
	}
}

// Port provides the Port of an address, or 0 if there is no port.
func (a *Address) Port() uint16 {
	switch a.Network() {
	case "tcp", "tcp4", "tcp6":
		addr, _ := net.ResolveTCPAddr(a.Network(), a.String())
		return uint16(addr.Port)
	case "udp", "udp4", "udp6":
		addr, _ := net.ResolveUDPAddr(a.Network(), a.String())
		return uint16(addr.Port)
	default:
		return uint16(0)
	}
}

// Host provides the net.IP of the Internet Host.
func (a *Address) Host() net.IP {
	switch a.Network() {
	case "tcp", "tcp4", "tcp6":
		addr, _ := net.ResolveTCPAddr(a.Network(), a.String())
		return addr.IP
	case "udp", "udp4", "udp6":
		addr, _ := net.ResolveUDPAddr(a.Network(), a.String())
		return addr.IP
	case "ip", "ip4", "ip6":
		addr, _ := net.ResolveIPAddr(a.Network(), a.String())
		return addr.IP
	default:
		return net.IP{}
	}
}

// NewAddress creates a new address representing a given host and port.
func NewAddress(network string, host net.IP, port uint16) Address {
	hostport := net.JoinHostPort(host.String(), fmt.Sprintf("%d", port))
	return NewAddressFromString(network, hostport)
}

// NewAddressFromString creates a new address from a string representation.
func NewAddressFromString(network, address string) Address {
	switch network {
	case "tcp", "tcp4", "tcp6":
		addr, _ := net.ResolveTCPAddr(network, address)
		return Address{addr}
	case "udp", "udp4", "udp6":
		addr, _ := net.ResolveUDPAddr(network, address)
		return Address{addr}
	default:
		return Address{}
	}
}
