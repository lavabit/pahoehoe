package stun

import (
	"bytes"
	"errors"
)

// UnknownStunAttribute represents a STUN attribute found in a message for which
// there is no parsed attribute type available.
type UnknownStunAttribute struct {
	// ClaimedType holds the AttributeType value of the unknown attribute.
	ClaimedType AttributeType
	// Data contains the unparsed body of the Attribute.
	Data []byte
}

// NewUnknownAttribute Instantiates a new instance of an UnknownAttribute cast
// to the Attribute Interface.
func NewUnknownAttribute() Attribute {
	return Attribute(new(UnknownStunAttribute))
}

func (h *UnknownStunAttribute) Type() AttributeType {
	return h.ClaimedType
}

func (h *UnknownStunAttribute) Encode(msg *Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := WriteAttributeHeader(buf, Attribute(h), msg)
	buf.Write(h.Data)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *UnknownStunAttribute) Decode(data []byte, length uint16, _ *Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated Attribute")
	}
	h.Data = data[0:length]
	return nil
}

func (h *UnknownStunAttribute) Length(_ *Message) uint16 {
	return uint16(len(h.Data))
}
