package turn

import (
	"bytes"
	"fmt"
	common "github.com/willscott/goturn/common"
	"github.com/willscott/goturn/stun"
	"net"
)

const (
	XorRelayedAddress common.AttributeType = 0x16
)

type XorRelayedAddressAttribute struct {
	Family  uint16
	Port    uint16
	Address net.IP
}

func (x *XorRelayedAddressAttribute) String() string {
	return net.JoinHostPort(x.Address.String(), fmt.Sprintf("%d", x.Port))
}

func NewXorRelayedAddressAttribute() common.Attribute {
	return common.Attribute(new(XorRelayedAddressAttribute))
}

func (x *XorRelayedAddressAttribute) Type() common.AttributeType {
	return XorRelayedAddress
}

func (h *XorRelayedAddressAttribute) Encode(msg *common.Message) ([]byte, error) {
	buf := new(bytes.Buffer)
	if err := common.WriteAttributeHeader(buf, common.Attribute(h), msg); err != nil {
		return nil, err
	}
	mapped := stun.XorMappedAddressAttribute(*h)
	bytes, err := stun.XorAddressData(&mapped, msg)
	if err != nil {
		return nil, err
	}
	buf.Write(bytes)
	return buf.Bytes(), nil
}

func (h *XorRelayedAddressAttribute) Decode(data []byte, length uint16, p *common.Parser) error {
	mapped := stun.XorMappedAddressAttribute(*h)
	if err := mapped.Decode(data, length, p); err != nil {
		return err
	}
	h.Family = mapped.Family
	h.Port = mapped.Port
	h.Address = mapped.Address
	return nil
}

func (h *XorRelayedAddressAttribute) Length(_ *common.Message) uint16 {
	if h.Family == 1 {
		return 8
	} else {
		return 20
	}
}
