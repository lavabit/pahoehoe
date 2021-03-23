package stun

import (
	"bytes"
	"encoding/binary"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	UnknownAttributes stun.AttributeType = 0xA
)

type UnknownAttributesAttribute struct {
	Attributes []uint16
}

func NewUnknownAttributesAttribute() stun.Attribute {
	return stun.Attribute(new(UnknownAttributesAttribute))
}

func (h *UnknownAttributesAttribute) Type() stun.AttributeType {
	return UnknownAttributes
}

func (h *UnknownAttributesAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	for _, att := range h.Attributes {
		if err := binary.Write(buf, binary.BigEndian, att); err != nil {
			return nil, err
		}
	}

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *UnknownAttributesAttribute) Decode(data []byte, length uint16, _ *stun.Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated Unknown Attributes Attribute")
	}

	for i := 0; uint16(i) < length; i += 2 {
		h.Attributes = append(h.Attributes, uint16(data[i])<<8+uint16(data[i+1]))
	}

	return nil
}

func (h *UnknownAttributesAttribute) Length(_ *stun.Message) uint16 {
	return uint16(2 * len(h.Attributes))
}
