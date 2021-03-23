package stun

import (
	common "github.com/willscott/goturn/common"
	"testing"
)

func TestShortTermIntegrityRoundtrip(t *testing.T) {
	credentials := common.Credentials{Password: "1234567890"}
	m := common.Message{}
	m.Credentials = credentials
	m.Attributes = []common.Attribute{&MessageIntegrityAttribute{}}

	msg, err := m.Serialize()
	if err != nil {
		t.Fatalf("Could not serialize message with integrity attribute: %s", err)
	}

	newm, err := common.Parse(msg, credentials, common.AttributeSet{
		MessageIntegrity: NewMessageIntegrityAttribute})
	if err != nil {
		t.Fatal("Could not re-parse encoded message.")
	}

	if newm.Attributes[0].Type() != MessageIntegrity {
		t.Error("Re-instantiated message didn't check integrity")
	}
}

func TestLongTermIntegrityRoundtrip(t *testing.T) {
	credentials := common.Credentials{Username: "me:time", Realm: "example.com", Password: "1234567890"}
	m := common.Message{}
	m.Credentials = credentials
	m.Attributes = []common.Attribute{&MessageIntegrityAttribute{}}

	msg, err := m.Serialize()
	if err != nil {
		t.Fatalf("Could not serialize message with integrity attribute: %s", err)
	}

	newm, err := common.Parse(msg, credentials, common.AttributeSet{
		MessageIntegrity: NewMessageIntegrityAttribute})
	if err != nil {
		t.Fatal("Could not re-parse encoded message.")
	}

	if newm.Attributes[0].Type() != MessageIntegrity {
		t.Error("Re-instantiated message didn't check integrity")
	}
}

func TestIntegrityWithFingerprintRoundtrip(t *testing.T) {
	credentials := common.Credentials{Password: "1234567890"}
	m := common.Message{}
	m.Credentials = credentials
	m.Attributes = []common.Attribute{&MessageIntegrityAttribute{},
		&FingerprintAttribute{}}

	msg, err := m.Serialize()
	if err != nil {
		t.Fatalf("Could not serialize message with integrity attribute: %s", err)
	}

	newm, err := common.Parse(msg, credentials, common.AttributeSet{
		MessageIntegrity: NewMessageIntegrityAttribute,
		Fingerprint:      NewFingerprintAttribute})
	if err != nil {
		t.Fatal("Could not re-parse encoded message.")
	}

	if newm.Attributes[0].Type() != MessageIntegrity {
		t.Error("Re-instantiated message didn't check integrity")
	}
}
