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

package anon

import (
	"0xacab.org/leap/vpnweb/pkg/auth/creds"
	"0xacab.org/leap/vpnweb/pkg/config"
)

const Label string = "anon"

// AnonAuthenticator will allow anyone to get access to a protected resource (Like VPN certificates).
// Used by RiseupVPN
type Authenticator struct {
}

func (a *Authenticator) GetLabel() string {
	return Label
}

func (a *Authenticator) CheckCredentials(cred *creds.Credentials) (bool, error) {
	return true, nil
}

func (a *Authenticator) NeedsCredentials() bool {
	return false
}

func GetAuthenticator(opts *config.Opts, skipInit bool) *Authenticator {
	return &Authenticator{}
}
