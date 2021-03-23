package stun

import (
	"bytes"
	"crypto/hmac"
	"crypto/md5"
	"crypto/sha1"
	"encoding/binary"
	"errors"
	"fmt"
	"github.com/willscott/goturn/common"
)

const (
	MessageIntegrity stun.AttributeType = 0x8
)

type MessageIntegrityAttribute struct {
}

func NewMessageIntegrityAttribute() stun.Attribute {
	return stun.Attribute(new(MessageIntegrityAttribute))
}

func (h *MessageIntegrityAttribute) Type() stun.AttributeType {
	return MessageIntegrity
}

func makeKey(cred *stun.Credentials) []byte {
	if len(cred.Username) > 0 {
		key := make([]byte, 16)
		sum := md5.Sum([]byte(cred.Username + ":" + cred.Realm + ":" + cred.Password))
		copy(key[:], sum[0:16])
		return key
	} else if len(cred.Password) > 0 {
		return []byte(cred.Password)
	} else {
		return nil
	}
}

func (h *MessageIntegrityAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	if err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg); err != nil {
		return nil, err
	}

	key := makeKey(&msg.Credentials)
	if key == nil {
		return nil, errors.New("Cannot sign request without credentials.")
	}

	// Calculate partial message
	var partialMsg stun.Message
	partialMsg.Header = msg.Header
	partialMsg.Credentials = msg.Credentials

	partialMsg.Attributes = make([]stun.Attribute, len(msg.Attributes))
	copy(partialMsg.Attributes, msg.Attributes)

	// Remove either 1 (msg integrity) or 2 (fingerprint and msg integrity) attributes
	partialMsg.Attributes = partialMsg.Attributes[0 : len(partialMsg.Attributes)-1]
	if len(partialMsg.Attributes) > 0 &&
		partialMsg.Attributes[len(partialMsg.Attributes)-1].Type() == MessageIntegrity {
		partialMsg.Attributes = partialMsg.Attributes[0 : len(partialMsg.Attributes)-1]
	}
	// Add a new attribute w/ same length as msg integrity
	dummy := stun.UnknownStunAttribute{MessageIntegrity, make([]byte, 20)}
	partialMsg.Attributes = append(partialMsg.Attributes, &dummy)
	// calcualte the byte string
	msgBytes, err := partialMsg.Serialize()
	if err != nil {
		return nil, err
	}

	//hmac all but the dummy attribute
	mac := hmac.New(sha1.New, key)
	mac.Write(msgBytes[0 : len(msgBytes)-24])
	hash := mac.Sum(nil)

	err = binary.Write(buf, binary.BigEndian, hash)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *MessageIntegrityAttribute) Decode(data []byte, length uint16, p *stun.Parser) error {
	if length != 20 || len(data) < 20 {
		return errors.New("Truncated MessageIntegrity Attribute")
	}

	key := makeKey(p.Credentials)
	if key == nil {
		return errors.New("No credentials to decrypt MessageIntegrity Attribute")
	}

	msgBytes := p.Data[0:p.Offset]
	// Twiddle length to where it would be at the point of this attribute
	var header stun.Header
	if err := header.Decode(msgBytes); err != nil {
		return err
	}
	oldLength := header.Length
	header.Length = uint16(p.Offset - 20 + 24)
	newhead, err := header.Encode()
	if err != nil {
		return err
	}
	copy(msgBytes[0:20], newhead[0:20])

	mac := hmac.New(sha1.New, key)
	mac.Write(msgBytes[0:len(msgBytes)])
	hash := mac.Sum(nil)

	//unfiddle w/ header length
	header.Length = oldLength
	newhead, _ = header.Encode()
	copy(msgBytes[0:20], newhead[0:20])

	if !bytes.Equal(hash, data[0:20]) {
		return errors.New(fmt.Sprintf("Invalid Message Integrity value. Calculated %x, but was %x", hash, data[0:20]))
	}

	return nil
}

func (h *MessageIntegrityAttribute) Length(_ *stun.Message) uint16 {
	return 20
}
