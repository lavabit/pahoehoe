package goturn

import (
	"crypto/rand"
	common "github.com/willscott/goturn/common"
	"github.com/willscott/goturn/stun"
)

// STUN (RFC 5389) defined message types.
const (
	BindingRequest       common.HeaderType = 0x0001
	SharedSecretRequest                    = 0x0002
	BindingResponse                        = 0x0101
	SharedSecretResponse                   = 0x0102
	BindingError                           = 0x0111
	SharedSecretError                      = 0x0112
)

// Deprecated: Should live in individual stun attribute implementations.
const (
	AlternateServer common.AttributeType = 0x8023
)

// ParseStun parses a message in RFC 5389 STUN format. Attributes defined in
// subsequent standards will not be parsed.
func ParseStun(data []byte) (*common.Message, error) {
	return common.Parse(data, nil, stun.StunAttributes)
}

// NewBindingRequest creates a STUN message for a client binding request.
func NewBindingRequest() (*common.Message, error) {
	message := common.Message{
		Header: common.Header{
			Type: BindingRequest,
		},
	}
	_, err := rand.Read(message.Header.Id[:])
	return &message, err
}
