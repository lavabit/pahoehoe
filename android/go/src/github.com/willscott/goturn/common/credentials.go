package stun

import (
	"fmt"
)

// Credentials represent knowledge about a STUN conversation that is more general
// than a single message.
type Credentials struct {
	// Each conversation has a server-provided Nonce, learned in the first server
	// response.
	Nonce []byte
	// Conversations with a long-term identity will have a Username provided
	// out-of-band.
	Username string
	// Conversations with a long-term identity will have a Realm provided by the
	// server.
	Realm string
	// Conversations validated with a message integrity attribute must have a
	// password provided out-of-band.
	Password string
}

// String represents credentials as a string for debugging.
func (c Credentials) String() string {
	return fmt.Sprintf("%s:%s@%s [nonce %s]", c.Username, c.Password, c.Realm, c.Nonce)
}

// ForNewConnection derives a new set of credentials with same authorization,
// as an existing set, but expecting a new Nonce to be provided.
func (c *Credentials) ForNewConnection() *Credentials {
	creds := new(Credentials)
	creds.Username = c.Username
	creds.Realm = c.Realm
	creds.Password = c.Password
	return creds
}
