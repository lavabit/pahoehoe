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

package web

import (
	"0xacab.org/leap/vpnweb/pkg/auth/creds"
	"0xacab.org/leap/vpnweb/pkg/config"
	"0xacab.org/leap/vpnweb/pkg/metrics"
	"encoding/json"
	"github.com/auth0/go-jwt-middleware"
	"github.com/dgrijalva/jwt-go"
	"log"
	"net/http"
	"os"
	"strings"
	"time"
)

const debugAuth string = "VPNWEB_DEBUG_AUTH"

func isDebugAuthEnabled(s string) bool {
	if strings.ToLower(s) == "yes" || strings.ToLower(s) == "true" {
		return true
	}
	return false
}

func AuthMiddleware(authenticationFunc func(*creds.Credentials) (bool, error), opts *config.Opts) http.HandlerFunc {
	debugFlag, exists := os.LookupEnv(debugAuth)
	if !exists {
		debugFlag = "false"
	}
	var authHandler = http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		var c creds.Credentials
		err := json.NewDecoder(r.Body).Decode(&c)
		if err != nil {
			log.Println("Auth request did not send valid json")
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}

		if opts.PasswordPolicy == "ignore" {
			if c.User == "" {
				log.Println("Auth request did not include user")
				http.Error(w, "Missing username", http.StatusBadRequest)
				return
			}
		} else {
			if c.User == "" || c.Password == "" {
				log.Println("Auth request did not include user/password")
				http.Error(w, "Missing username or password", http.StatusBadRequest)
				return
			}
		}

		valid, err := authenticationFunc(&c)

		if !valid {
			if err != nil {
				metrics.UnavailableLogins.Inc()
				log.Println("Error while checking credentials: ", err)
				http.Error(w, "503: Auth service unavailable", http.StatusServiceUnavailable)
				return
			} else {
				metrics.FailedLogins.Inc()
				if isDebugAuthEnabled(debugFlag) {
					log.Println("Wrong credentials for user", c.User)
				}
				http.Error(w, "401: Wrong user and/or password", http.StatusUnauthorized)
				return
			}
		}

		metrics.SuccessfulLogins.Inc()

		if isDebugAuthEnabled(debugFlag) {
			log.Println("Valid auth for user", c.User)
		}
		token := jwt.New(jwt.SigningMethodHS256)
		claims := token.Claims.(jwt.MapClaims)
		claims["expiration"] = time.Now().Add(time.Hour * 24).Unix()
		tokenString, _ := token.SignedString([]byte(opts.AuthSecret))
		w.Write([]byte(tokenString))
	})
	return authHandler
}

func RestrictedMiddleware(shouldProtect func() bool, handler func(w http.ResponseWriter, r *http.Request), opts *config.Opts) http.Handler {

	jwtMiddleware := jwtmiddleware.New(jwtmiddleware.Options{
		ValidationKeyGetter: func(token *jwt.Token) (interface{}, error) {
			return []byte(opts.AuthSecret), nil
		},
		SigningMethod: jwt.SigningMethodHS256,
	})

	switch shouldProtect() {
	case false:
		return http.HandlerFunc(handler)
	case true:
		return jwtMiddleware.Handler(http.HandlerFunc(handler))
	}
	return nil
}
