package turn

import (
	"bytes"
	"encoding/binary"
	"errors"
	"github.com/willscott/goturn/common"
)

const (
	RequestedTransport stun.AttributeType = 0x19
)

type RequestedTransportAttribute struct {
	Transport uint8
}

func NewRequestedTransportAttribute() stun.Attribute {
	return stun.Attribute(new(RequestedTransportAttribute))
}

func (h *RequestedTransportAttribute) Type() stun.AttributeType {
	return RequestedTransport
}

func (h *RequestedTransportAttribute) Encode(msg *stun.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := stun.WriteAttributeHeader(buf, stun.Attribute(h), msg)
	err = binary.Write(buf, binary.BigEndian, h.Transport)
	err = binary.Write(buf, binary.BigEndian, uint16(0))
	err = binary.Write(buf, binary.BigEndian, uint8(0))

	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (h *RequestedTransportAttribute) Decode(data []byte, length uint16, _ *stun.Parser) error {
	if length != 4 || uint16(len(data)) < length {
		return errors.New("Truncated RequestedTransport Attribute")
	}
	h.Transport = uint8(data[0])
	if h.Transport != 17 {
		return errors.New("RequestedTransport Attribute was not UDP as expected.")
	}
	return nil
}

func (h *RequestedTransportAttribute) Length(_ *stun.Message) uint16 {
	return 4
}
