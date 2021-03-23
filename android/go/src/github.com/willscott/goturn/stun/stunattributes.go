// Package stun represents the attributes defined in the STUN RFC Standard.
package stun

import (
	"github.com/willscott/goturn/common"
)

var (
	// StunAttributes represents the AttributeSet of STUN defined attributes for
	// use when parsing STUN messages.
	StunAttributes = stun.AttributeSet{
		ErrorCode:         NewErrorCodeAttribute,
		Fingerprint:       NewFingerprintAttribute,
		MappedAddress:     NewMappedAddressAttribute,
		MessageIntegrity:  NewMessageIntegrityAttribute,
		Nonce:             NewNonceAttribute,
		Realm:             NewRealmAttribute,
		Software:          NewSoftwareAttribute,
		UnknownAttributes: NewUnknownAttributesAttribute,
		Username:          NewUsernameAttribute,
		XorMappedAddress:  NewXorMappedAddressAttribute,
	}
)
