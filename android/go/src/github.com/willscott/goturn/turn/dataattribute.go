package turn

import (
	"bytes"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	Data stun.AttributeType = 0x13
)

type DataAttribute struct {
	Data []byte
}

func NewDataAttribute() stun.Attribute {
	return stun.Attribute(new(DataAttribute))
}

func (h *DataAttribute) Type() stun.AttributeType {
	return Data
}

func (h *DataAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	buf.Write(h.Data)

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *DataAttribute) Decode(data []byte, length uint16, _ *stun.Parser) error {
	if uint16(len(data)) < length {
		return errors.New("Truncated Data Attribute")
	}
	h.Data = make([]byte, length)
	copy(h.Data, data[:])
	return nil
}

func (h *DataAttribute) Length(_ *stun.Message) uint16 {
	return uint16(len(h.Data))
}
