package stun

import (
	"bytes"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	Software stun.AttributeType = 0x8022
)

type SoftwareAttribute struct {
	Software string
}

func NewSoftwareAttribute() stun.Attribute {
	return stun.Attribute(new(SoftwareAttribute))
}

func (h *SoftwareAttribute) Type() stun.AttributeType {
	return Software
}

func (h *SoftwareAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	buf.WriteString(h.Software)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *SoftwareAttribute) Decode(data []byte, length uint16, _ *stun.Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated Software Attribute")
	}
	if length > 763 {
		return errors.New("Software Length is too long")
	}
	h.Software = string(data[0:length])
	return nil
}

func (h *SoftwareAttribute) Length(_ *stun.Message) uint16 {
	return uint16(len(h.Software))
}
