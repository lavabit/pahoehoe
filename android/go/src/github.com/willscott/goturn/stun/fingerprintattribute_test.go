package stun

import (
	common "github.com/willscott/goturn/common"
	"testing"
)

func TestFingerprintRoundtrip(t *testing.T) {
	m := common.Message{}
	m.Attributes = []common.Attribute{&FingerprintAttribute{}}

	msg, err := m.Serialize()
	if err != nil {
		t.Fatalf("Could not serialize message with fingerprint attribute: %s", err)
	}

	newm, err := common.Parse(msg, common.Credentials{}, common.AttributeSet{
		Fingerprint: NewFingerprintAttribute})
	if err != nil {
		t.Fatal("Could not re-parse encoded message.")
	}

	if newm.Attributes[0].Type() != Fingerprint {
		t.Error("Re-instantiated message didn't Fingerprint")
	}
}
