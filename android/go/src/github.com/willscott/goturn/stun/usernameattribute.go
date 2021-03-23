package stun

import (
	"bytes"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	Username stun.AttributeType = 0x6
)

type UsernameAttribute struct {
}

func NewUsernameAttribute() stun.Attribute {
	return stun.Attribute(new(UsernameAttribute))
}

func (h *UsernameAttribute) Type() stun.AttributeType {
	return Username
}

func (h *UsernameAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	buf.WriteString(msg.Credentials.Username)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *UsernameAttribute) Decode(data []byte, length uint16, p *stun.Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated Username Attribute")
	}
	p.Message.Credentials.Username = string(data[0:length])
	return nil
}

func (h *UsernameAttribute) Length(msg *stun.Message) uint16 {
	return uint16(len(msg.Credentials.Username))
}
