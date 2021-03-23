package stun

import (
	"bytes"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	Realm stun.AttributeType = 0x14
)

type RealmAttribute struct {
}

func NewRealmAttribute() stun.Attribute {
	return stun.Attribute(new(RealmAttribute))
}

func (h *RealmAttribute) Type() stun.AttributeType {
	return Realm
}

func (h *RealmAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	buf.WriteString(msg.Credentials.Realm)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *RealmAttribute) Decode(data []byte, length uint16, p *stun.Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated Realm Attribute")
	}
	if length > 763 {
		return errors.New("Realm Length is too long")
	}
	p.Message.Credentials.Realm = string(data[0:length])
	return nil
}

func (h *RealmAttribute) Length(msg *stun.Message) uint16 {
	return uint16(len(msg.Credentials.Realm))
}
