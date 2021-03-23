package turn

import (
	"bytes"
	"encoding/binary"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	ConnectionId stun.AttributeType = 0x2a
)

type ConnectionIdAttribute struct {
	ConnectionId uint32
}

func NewConnectionIdAttribute() stun.Attribute {
	return stun.Attribute(new(ConnectionIdAttribute))
}

func (h *ConnectionIdAttribute) Type() stun.AttributeType {
	return ConnectionId
}

func (h *ConnectionIdAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	err = binary.Write(buf, binary.BigEndian, h.ConnectionId)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *ConnectionIdAttribute) Decode(data []byte, length uint16, _ *stun.Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated ConnectionID Attribute")
	}
	h.ConnectionId = binary.BigEndian.Uint32(data[0:4])
	return nil
}

func (h *ConnectionIdAttribute) Length(_ *stun.Message) uint16 {
	return 4
}
