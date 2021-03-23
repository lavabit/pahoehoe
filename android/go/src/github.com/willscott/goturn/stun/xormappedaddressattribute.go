package stun

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"github.com/willscott/goturn/common"
	"net"
)

const (
	XorMappedAddress stun.AttributeType = 0x20
)

type XorMappedAddressAttribute struct {
	Family  uint16
	Port    uint16
	Address net.IP
}

func NewXorMappedAddressAttribute() stun.Attribute {
	return stun.Attribute(new(XorMappedAddressAttribute))
}

func (h XorMappedAddressAttribute) String() string {
	return net.JoinHostPort(h.Address.String(), fmt.Sprintf("%d", h.Port))
}

func (h *XorMappedAddressAttribute) Type() stun.AttributeType {
	return XorMappedAddress
}

func XorAddressData(h *XorMappedAddressAttribute, msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.BigEndian, h.Family)
	xport := h.Port ^ uint16(stun.MagicCookie>>16)
	err = binary.Write(buf, binary.BigEndian, xport)

	var xoraddress []byte
	if h.Family == 1 {
		xoraddress = make([]byte, 4)
		binary.BigEndian.PutUint32(xoraddress, stun.MagicCookie)
		h.Address = h.Address.To4()
	} else {
		xoraddress = make([]byte, 16)
		binary.BigEndian.PutUint32(xoraddress, stun.MagicCookie)
		copy(xoraddress[4:16], msg.Header.Id[:])
		h.Address = h.Address.To16()
	}
	for i, _ := range xoraddress {
		xoraddress[i] ^= h.Address[i]
	}
	err = binary.Write(buf, binary.BigEndian, xoraddress)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *XorMappedAddressAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	if err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg); err != nil {
		return nil, err
	}
	bytes, err := XorAddressData(h, msg)
	if err != nil {
		return nil, err
	}
	buf.Write(bytes)
	return buf.Bytes(), nil
}

func (h *XorMappedAddressAttribute) Decode(data []byte, _ uint16, p *stun.Parser) error {
	if data[0] != 0 && data[1] != 1 && data[0] != 2 {
		return errors.New("Incorrect Mapped Address Family.")
	}
	h.Family = uint16(data[1])
	if (h.Family == 1 && len(data) < 8) || (h.Family == 2 && len(data) < 20) {
		return errors.New("Mapped Address Attribute unexpectedly Truncated.")
	}
	h.Port = uint16(data[2])<<8 + uint16(data[3])
	// X-port is XOR'ed with the 16 most significant bits of the magic Cookie
	h.Port ^= uint16(stun.MagicCookie >> 16)

	var xoraddress []byte
	if h.Family == 1 {
		xoraddress = make([]byte, 4)
		binary.BigEndian.PutUint32(xoraddress, stun.MagicCookie)
		h.Address = make([]byte, 4)
		copy(h.Address, data[4:8])
	} else {
		xoraddress = make([]byte, 16)
		binary.BigEndian.PutUint32(xoraddress, stun.MagicCookie)
		copy(xoraddress[4:16], p.Message.Header.Id[:])
		h.Address = make([]byte, 16)
		copy(h.Address, data[4:20])
	}
	for i, _ := range xoraddress {
		h.Address[i] ^= xoraddress[i]
	}
	return nil
}

func (h *XorMappedAddressAttribute) Length(_ *stun.Message) uint16 {
	if h.Family == 1 {
		return 8
	} else {
		return 20
	}
}
