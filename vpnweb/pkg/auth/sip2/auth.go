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

package sip2

import (
	"log"
	"os"

	"0xacab.org/leap/vpnweb/pkg/config"
)

const (
	sipUserVar           string = "VPNWEB_SIP_USER"
	sipPassVar           string = "VPNWEB_SIP_PASS"
	sipPortVar           string = "VPNWEB_SIP_PORT"
	sipHostVar           string = "VPNWEB_SIP_HOST"
	sipLibrLocVar        string = "VPNWEB_SIP_LIBR_LOCATION"
	sipTerminatorVar     string = "VPNWEB_SIP_TERMINATOR"
	sipDefaultTerminator string = "\r\n"
)

func getConfigFromEnv(envVar, defaultVar string) string {
	val, exists := os.LookupEnv(envVar)
	if !exists {
		if defaultVar == "" {
			log.Fatal("Need to set required env var: ", envVar)
		} else {
			return defaultVar
		}
	}
	return val
}

func setupTerminatorFromEnv() {
	config.FallbackToEnv(&telnetTerminator, sipTerminatorVar, sipDefaultTerminator)
	if telnetTerminator == "\\r" {
		telnetTerminator = "\r"
	} else if telnetTerminator == "\\r\\n" {
		telnetTerminator = "\r\n"
	}
}

func initializeSipConnection(skipConnect bool, passwordPolicy string) (sipClient, error) {
	log.Println("Initializing SIP2 authenticator")

	user := getConfigFromEnv(sipUserVar, "")
	pass := getConfigFromEnv(sipPassVar, "")
	host := getConfigFromEnv(sipHostVar, "localhost")
	port := getConfigFromEnv(sipPortVar, "6001")
	loc := getConfigFromEnv(sipLibrLocVar, "")

	setupTerminatorFromEnv()

	sip := newClient(host, port, loc, passwordPolicy)

	if skipConnect {
		// for testing purposes
		return sip, nil
	}

	sip.setCredentials(user, pass)
	_, err := sip.doConnectAndReact()
	if err != nil {
		return sip, err
	}
	return sip, nil
}

func GetAuthenticator(opts *config.Opts, skipConnect bool) *sipClient {

	sip, err := initializeSipConnection(skipConnect, opts.PasswordPolicy)
	if err != nil {
		log.Fatal("Cannot initialize sip:", err)
	}
	return &sip
}
