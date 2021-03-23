package sip2

import (
	"testing"
)

const (
	invalidCard = "24YYYY          00020200220    173142AE|AAaaaa|BLN|AFInvalid cardnumber|AOtestlibrary|"
	invalidPass = "24              00020200221    185454AE MrUser|AA01000|BLY|CQN|AFGreetings from Koha.  -- Invalid password|AOtestlibrary|"
	authOK      = "24              00020200224    172540AE MrUser|AA01000|BLY|CQY|AFGreetings from Koha. |AOtestlibrary|"
	statusOK    = "98YYYYNN10000520200221    1853422.00AOtestlibrary|BXYYYYYYYYYYYNYYYY|"
)

func doParse(txt string) (*message, error) {
	p := getParser()
	msg, err := p.parseMessage(txt)
	return msg, err
}

func TestInvalidCard(t *testing.T) {
	msg, err := doParse(invalidCard)
	if err != nil {
		t.Fatal("unexpected error", err)
	}
	validUser, err := isValidUser(msg)
	if validUser == true {
		t.Fatal("expected invalid user")
	}
}

func TestInvalidPass(t *testing.T) {
	msg, err := doParse(invalidPass)
	if err != nil {
		t.Fatal("unexpected error", err)
	}
	validPass, err := isValidPassword(msg)
	if validPass == true {
		t.Fatal("expected invalid pas")
	}
}

func TestAuthOK(t *testing.T) {
	msg, err := doParse(authOK)
	if err != nil {
		t.Fatal("unexpected error", err)
	}
	validUser, err := isValidUser(msg)
	if validUser != true {
		t.Fatal("expected valid user")
	}
	validPass, err := isValidPassword(msg)
	if validPass != true {
		t.Fatal("expected valid pass")
	}
}

func TestStatusOK(t *testing.T) {
	msg, err := doParse(statusOK)
	if err != nil {
		t.Fatal("unexpected error", err)
	}
	proto, _ := msg.getFixedFieldValue(protocolVersion)
	if proto != "2.00" {
		t.Fatal("expected protocol 2.00")
	}
}
