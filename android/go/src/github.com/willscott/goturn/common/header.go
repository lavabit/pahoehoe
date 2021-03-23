package stun

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
)

const (
	// MagicCookie is a STUN defined constant for wire representation of messages.
	MagicCookie uint32 = 0x2112A442
)

// HeaderType is a numeric representation of the type of STUN message and whether
// it is a request, response, indication, or error.
type HeaderType uint16

// Header represents the header of a STUN message.
type Header struct {
	// The Purpose of a STUN message is denoted by its Type.
	Type HeaderType
	// Length represents the total length of the Message Attributes.
	Length uint16
	// Each STUN message has a unique Id.
	Id [12]byte
}

// String provides a textual representation of a STUN header for logging or debugging.
func (h Header) String() string {
	return fmt.Sprintf("Header(%#v id=%x len=%d)", h.Type, h.Id, h.Length)
}

// Encode the byte representation of a STUN header.
func (h *Header) Encode() ([]byte, error) {
	buf := new(bytes.Buffer)

	err := binary.Write(buf, binary.BigEndian, h.Type)
	err = binary.Write(buf, binary.BigEndian, h.Length)
	err = binary.Write(buf, binary.BigEndian, MagicCookie)
	err = binary.Write(buf, binary.BigEndian, h.Id)

	if len(h.Id) != 12 {
		return nil, errors.New("Unsupported Transaction ID Length")
	}

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

// Decode parses a header from its byte representation.
func (h *Header) Decode(data []byte) error {
	if len(data) < 20 {
		return errors.New("Header Length Too Short")
	}

	// Correctness checks.
	if binary.BigEndian.Uint16(data[0:])>>14 != 0 {
		return errors.New("First 2 bits are not 0")
	}

	if binary.BigEndian.Uint32(data[4:]) != MagicCookie {
		return errors.New("Bad Magic Cookie")
	}

	if binary.BigEndian.Uint16(data[2:])&3 != 0 {
		return errors.New("Message Length is not a multiple of 4")
	}

	h.Type = HeaderType(binary.BigEndian.Uint16(data[0:]))
	h.Length = binary.BigEndian.Uint16(data[2:])
	copy(h.Id[:], data[8:20])

	return nil
}
