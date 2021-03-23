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

package config

import (
	"flag"
	"log"
	"os"
)

const (
	DefaultAuthenticationModule string = "anon"
	DefaultPasswordPolicy       string = "require"
)

type Opts struct {
	Tls            bool
	CaCrt          string
	CaKey          string
	TlsCrt         string
	TlsKey         string
	Port           string
       Address        string
       Redirect       string
	MetricsPort    string
	Auth           string
	AuthSecret     string
	ApiPath        string
	ProviderCaPath string
	PasswordPolicy string
}

func checkPathExists(path string) bool {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		return false
	}
	return true
}

func FallbackToEnv(variable *string, envVar, defaultVar string) {

	if *variable == "" {
		val, exists := os.LookupEnv(envVar)
		if exists && val != "" {
			*variable = val
		} else {
			*variable = defaultVar
		}
	}
}

func doCaFilesSanityCheck(caCrt string, caKey string) {
	if _, err := os.Stat(caCrt); os.IsNotExist(err) {
		log.Fatal("cannot find caCrt file")
	}
	if _, err := os.Stat(caKey); os.IsNotExist(err) {
		log.Fatal("cannot find caKey file")
	}
}

func doTlsFilesSanityCheck(tlsCrt string, tlsKey string) {
	if _, err := os.Stat(tlsCrt); os.IsNotExist(err) {
		log.Fatal("cannot find tlsCrt file")
	}
	if _, err := os.Stat(tlsKey); os.IsNotExist(err) {
		log.Fatal("cannot find tlsKey file")
	}
}

func NewOpts() *Opts {
	opts := new(Opts)
	initializeFlags(opts)
	checkConfigurationOptions(opts)
	return opts
}

func initializeFlags(opts *Opts) {
	flag.StringVar(&opts.CaCrt, "vpnCaCrt", "", "Path to the CA public key used for VPN certificates")
	flag.StringVar(&opts.CaKey, "vpnCaKey", "", "Path to the CA private key used for VPN certificates")
	flag.BoolVar(&opts.Tls, "tls", false, "Enable TLS on the service")
	flag.StringVar(&opts.TlsCrt, "tlsCrt", "", "Path to the cert file for TLS")
	flag.StringVar(&opts.TlsKey, "tlsKey", "", "Path to the key file for TLS")
	flag.StringVar(&opts.Port, "port", "", "Port where the server will listen (default: 8000)")
       flag.StringVar(&opts.Address, "address", "", "Listen for connections on a specific IP address or leave empty to listen on all IP address (default: empty)")
       flag.StringVar(&opts.Redirect, "redirect", "", "Redirect any unhandled/unecpected URLs to this address")
	flag.StringVar(&opts.MetricsPort, "metricsPort", "", "Port where the metrics server will listen (default: 8001)")
	flag.StringVar(&opts.Auth, "auth", "", "Authentication module (anon, sip2)")
	flag.StringVar(&opts.ApiPath, "apiPath", "", "Path to the API public files")
	flag.StringVar(&opts.ProviderCaPath, "providerCaCrt", "", "Path to the provider CA certificate")
	flag.StringVar(&opts.PasswordPolicy, "passwordPolicy", "", "Password policy, if used  (require|ignore)")
	flag.Parse()

	FallbackToEnv(&opts.CaCrt, "VPNWEB_CACRT", "")
	FallbackToEnv(&opts.CaKey, "VPNWEB_CAKEY", "")
	FallbackToEnv(&opts.TlsCrt, "VPNWEB_TLSCRT", "")
	FallbackToEnv(&opts.TlsKey, "VPNWEB_TLSKEY", "")
	FallbackToEnv(&opts.Port, "VPNWEB_PORT", "8000")
       FallbackToEnv(&opts.Address, "VPNWEB_ADDRESS", "")
       FallbackToEnv(&opts.Redirect, "VPNWEB_REDIRECT", "")
	FallbackToEnv(&opts.MetricsPort, "VPNWEB_METRICS_PORT", "8001")
	FallbackToEnv(&opts.Auth, "VPNWEB_AUTH", DefaultAuthenticationModule)
	FallbackToEnv(&opts.AuthSecret, "VPNWEB_AUTH_SECRET", "")
	FallbackToEnv(&opts.ApiPath, "VPNWEB_API_PATH", "/etc/leap/config/vpn")
	FallbackToEnv(&opts.ProviderCaPath, "VPNWEB_PROVIDER_CA", "/etc/leap/ca/ca.crt")
	FallbackToEnv(&opts.PasswordPolicy, "VPNWEB_PASSWORD_POLICY", DefaultPasswordPolicy)
}

func checkConfigurationOptions(opts *Opts) {
	if opts.CaCrt == "" {
		log.Fatal("missing caCrt parameter")
	}
	if opts.CaKey == "" {
		log.Fatal("missing caKey parameter")
	}

	if opts.Tls == true {
		if opts.TlsCrt == "" {
			log.Fatal("missing tls_crt parameter")
		}
		if opts.TlsKey == "" {
			log.Fatal("missing tls_key parameter")
		}
	}

	doCaFilesSanityCheck(opts.CaCrt, opts.CaKey)
	if opts.Tls == true {
		doTlsFilesSanityCheck(opts.TlsCrt, opts.TlsKey)
	}

	if !checkPathExists(opts.ApiPath) {
		log.Fatal("Configured API path does not exist: ", opts.ApiPath)
	}
	if !checkPathExists(opts.ProviderCaPath) {
		log.Fatal("Configured provider CA path does not exist: ", opts.ProviderCaPath)
	}

	log.Println("Authentication module:", opts.Auth)
	if opts.Auth != DefaultAuthenticationModule {
		log.Println("Password policy:", opts.PasswordPolicy)
	}
}
