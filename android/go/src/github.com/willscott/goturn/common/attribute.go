package stun

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
)

// AttributeType is the numeric representation of a STUN attribute.
type AttributeType uint16

// Attribute represents a single serializable STUN attribute.
type Attribute interface {
	// Type provides the type of an attribute for encoding it into a STUN message.
	Type() AttributeType
	// Encode returns the byte array of the contents of the attribute when it
	// should be appended to a given STUN message. The message is provided because
	// some attributes must calculate a checksum based upon the previous contents
	// of the message.
	Encode(*Message) ([]byte, error)
	// Decode instantiates an Attribute from a []byte, when being parsed out of
	// a byte stream represented by a Parser. The attribute is assumed to have
	// been parsed correctly when a nil error is returned.
	Decode([]byte, uint16, *Parser) error
	// Length returns the length of the encoded attribute body when it would be
	// encoded as part of a STUN message.
	Length(*Message) uint16
}

// AttributeSet represents the mapping of known attribute types that a parser
// will use when parsing a STUN message.
type AttributeSet map[AttributeType]func() Attribute

// WriteHeader will append a STUN attribute header onto a byte buffer for a
// given attribute and message pair.
func WriteAttributeHeader(buf *bytes.Buffer, a Attribute, msg *Message) error {
	attributeType := uint16(a.Type())
	header := (uint32(attributeType) << 16) + uint32(a.Length(msg))
	return binary.Write(buf, binary.BigEndian, header)
}

// DecodeAttribute returns a parsed Attribute representation of data based
// upon the known AttributeType's mapped by attrs.
func DecodeAttribute(data []byte, attrs AttributeSet, parser *Parser) (*Attribute, error) {
	attributeType := binary.BigEndian.Uint16(data)
	length := binary.BigEndian.Uint16(data[2:])
	attrConstructor, ok := attrs[AttributeType(attributeType)]
	if !ok {
		attrConstructor = NewUnknownAttribute
	}
	result := attrConstructor()

	err := result.Decode(data[4:], length, parser)
	if err != nil {
		return nil, err
	} else if result.Length(parser.Message) != length {
		return nil, errors.New(fmt.Sprintf("Incorrect Length Specified for %T", result))
	}
	return &result, nil
}
