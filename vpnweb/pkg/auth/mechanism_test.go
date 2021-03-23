// Copyright (C) 2019 LEAP
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

package auth

import (
	"0xacab.org/leap/vpnweb/pkg/config"
	"os"
	"testing"
)

func TestGetAuthenticator(t *testing.T) {
	opts := &config.Opts{}
	opts.Auth = "anon"

	a := GetAuthenticator(opts, true)
	if a.GetLabel() != "anon" {
		t.Errorf("expected anon authenticator")
	}

	/* TODO test no secret */
	/* TODO test short secret */
	/* TODO refactor init to return proper errors */
	/* TODO test invalid auth method */

	os.Setenv("VPNWEB_SIP_USER", "user")
	os.Setenv("VPNWEB_SIP_PASS", "pass")
	os.Setenv("VPNWEB_SIP_LIBR_LOCATION", "test")
	opts.Auth = "sip2"
	opts.AuthSecret = "sikret000000000000000000000"
	a = GetAuthenticator(opts, true)
	if a.GetLabel() != "sip2" {
		t.Errorf("expected sip authenticator")
	}
}
