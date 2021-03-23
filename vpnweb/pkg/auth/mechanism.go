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
	"0xacab.org/leap/vpnweb/pkg/auth/anon"
	"0xacab.org/leap/vpnweb/pkg/auth/sip2"
	"0xacab.org/leap/vpnweb/pkg/config"
	"log"
)

func GetAuthenticator(opts *config.Opts, skipConnect bool) Authenticator {
	switch opts.Auth {
	case anon.Label:
		return anon.GetAuthenticator(opts, skipConnect)
	case sip2.Label:
		doAuthenticationChecks(opts)
		return sip2.GetAuthenticator(opts, skipConnect)
	default:
		bailOnBadAuthModule(opts.Auth)
	}
	return nil
}

func doAuthenticationChecks(opts *config.Opts) {
	if opts.AuthSecret == "" {
		log.Fatal("Need to provide an AuthSecret value for SIP Authentication")
	}
	if len(opts.AuthSecret) < 20 {
		log.Fatal("Please provider an AuthSecret longer than 20 chars")
	}
}

func bailOnBadAuthModule(module string) {
	log.Fatal("Unknown auth module: '", module, "'. Should be one of: ", sip2.Label, ", ", anon.Label, ".")
}
