// Package turn represents the attributes defined in the TURN RFC Standard.
package turn

import (
	common "github.com/willscott/goturn/common"
	"github.com/willscott/goturn/stun"
)

var (
	TurnAttributes = common.AttributeSet{
		ChannelNumber:      NewChannelNumberAttribute,
		ConnectionId:       NewConnectionIdAttribute,
		Data:               NewDataAttribute,
		Lifetime:           NewLifetimeAttribute,
		RequestedTransport: NewRequestedTransportAttribute,
		XorPeerAddress:     NewXorPeerAddressAttribute,
		XorRelayedAddress:  NewXorRelayedAddressAttribute,
	}
)

// Registry of stun and turn attributes as the default set to work with when
// decoding messages.
func AttributeSet() common.AttributeSet {
	set := make(common.AttributeSet)
	for key, value := range stun.StunAttributes {
		set[key] = value
	}
	for key, value := range TurnAttributes {
		set[key] = value
	}
	return set
}
