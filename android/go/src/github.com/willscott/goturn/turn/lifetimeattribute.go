package turn

import (
	"bytes"
	"encoding/binary"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	Lifetime stun.AttributeType = 0xD
)

type LifetimeAttribute struct {
	Lifetime uint32
}

func NewLifetimeAttribute() stun.Attribute {
	return stun.Attribute(new(LifetimeAttribute))
}

func (h *LifetimeAttribute) Type() stun.AttributeType {
	return Lifetime
}

func (h *LifetimeAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	err = binary.Write(buf, binary.BigEndian, h.Lifetime)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *LifetimeAttribute) Decode(data []byte, length uint16, _ *stun.Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated Lifetime Attribute")
	}
	h.Lifetime = binary.BigEndian.Uint32(data[0:4])
	return nil
}

func (h *LifetimeAttribute) Length(_ *stun.Message) uint16 {
	return 4
}
