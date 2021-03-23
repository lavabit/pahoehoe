package stun

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"github.com/willscott/goturn/common"
	"hash/crc32"
)

const (
	crcXOR uint32 = 0x5354554e
)
const (
	Fingerprint stun.AttributeType = 0x8028
)

type FingerprintAttribute struct {
	CRC uint32
}

func NewFingerprintAttribute() stun.Attribute {
	return stun.Attribute(new(FingerprintAttribute))
}

func (h *FingerprintAttribute) Type() stun.AttributeType {
	return Fingerprint
}

func (h *FingerprintAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)

	// Calculate partial message
	var partialMsg stun.Message
	partialMsg.Header = msg.Header
	partialMsg.Credentials = msg.Credentials
	partialMsg.Attributes = make([]stun.Attribute, len(msg.Attributes))
	copy(partialMsg.Attributes, msg.Attributes)

	// Fingerprint must be last attribute.
	partialMsg.Attributes = partialMsg.Attributes[0 : len(partialMsg.Attributes)-1]

	// Add a new attribute w/ same length as msg integrity
	dummy := stun.UnknownStunAttribute{Fingerprint, make([]byte, 4)}
	partialMsg.Attributes = append(partialMsg.Attributes, &dummy)
	// calcualte the byte string
	msgBytes, err := partialMsg.Serialize()
	if err != nil {
		return nil, err
	}

	crc := crc32.ChecksumIEEE(msgBytes[0:len(msgBytes)-8]) ^ crcXOR
	err = binary.Write(buf, binary.BigEndian, crc)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *FingerprintAttribute) Decode(data []byte, length uint16, p *stun.Parser) error {
	if length != 4 || len(data) < 4 {
		return errors.New("Truncated Fingerprint Attribute")
	}
	h.CRC = binary.BigEndian.Uint32(data[0:4])

	// Calculate partial message
	// Assumes there isn't a need to twiddle header length, since fingerprint must
	// be last attribute.
	partialMessage := p.Data[0:p.Offset]

	crc := crc32.ChecksumIEEE(partialMessage) ^ crcXOR

	if crc != h.CRC {
		return errors.New(fmt.Sprintf("Invalid Fingerprint value. calculated %x, but was %x", crc, h.CRC))
	}

	return nil
}

func (h *FingerprintAttribute) Length(_ *stun.Message) uint16 {
	return 4
}
