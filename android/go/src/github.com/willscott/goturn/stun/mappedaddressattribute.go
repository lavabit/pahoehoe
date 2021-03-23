package stun

import (
	"bytes"
	"encoding/binary"
	"errors"
	"github.com/willscott/goturn/common"
	"net"
)

const (
	MappedAddress stun.AttributeType = 0x1
)

type MappedAddressAttribute struct {
	Family  uint16
	Port    uint16
	Address net.IP
}

func NewMappedAddressAttribute() stun.Attribute {
	return stun.Attribute(new(MappedAddressAttribute))
}

func (h *MappedAddressAttribute) Type() stun.AttributeType {
	return MappedAddress
}

func (h *MappedAddressAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	err = binary.Write(buf, binary.BigEndian, h.Family)
	err = binary.Write(buf, binary.BigEndian, h.Port)
	err = binary.Write(buf, binary.BigEndian, h.Address)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *MappedAddressAttribute) Decode(data []byte, _ uint16, _ *stun.Parser) error {
	if data[0] != 0 && data[1] != 1 && data[0] != 2 {
		return errors.New("Incorrect Mapped Address Family.")
	}
	h.Family = uint16(data[1])
	if (h.Family == 1 && len(data) < 8) || (h.Family == 2 && len(data) < 20) {
		return errors.New("Mapped Address Attribute unexpectedly Truncated.")
	}
	h.Port = uint16(data[2])<<8 + uint16(data[3])
	if h.Family == 1 {
		h.Address = data[4:8]
	} else {
		h.Address = data[4:20]
	}
	return nil
}

func (h *MappedAddressAttribute) Length(_ *stun.Message) uint16 {
	if h.Family == 1 {
		return 8
	} else {
		return 20
	}
}
