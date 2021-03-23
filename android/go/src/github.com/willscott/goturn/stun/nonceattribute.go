package stun

import (
	"bytes"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	Nonce stun.AttributeType = 0x15
)

type NonceAttribute struct {
}

func NewNonceAttribute() stun.Attribute {
	return stun.Attribute(new(NonceAttribute))
}

func (h *NonceAttribute) Type() stun.AttributeType {
	return Nonce
}

func (h *NonceAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	buf.Write(msg.Credentials.Nonce)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *NonceAttribute) Decode(data []byte, length uint16, p *stun.Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated Nonce Attribute")
	}
	if length > 763 {
		return errors.New("Nonce Length is too long")
	}
	p.Message.Credentials.Nonce = make([]byte, length)
	copy(p.Message.Credentials.Nonce, data[0:length])
	return nil
}

func (h *NonceAttribute) Length(msg *stun.Message) uint16 {
	return uint16(len(msg.Credentials.Nonce))
}
