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
	//"github.com/reiver/go-telnet"
	/* TODO this one exposes DialTimeout */
	"github.com/linxiaozhi/go-telnet"
	"log"
	"net"
	"time"
)

// The terminator can be configured differently for different SIP endpoints.
// This gets set in sip2.auth according to an environment variable

var telnetTerminator string

var telnetTimeout string = "2000ms"

func telnetRead(conn gote.Connection) (string, error) {
	var buffer [1]byte
	recvData := buffer[:]
	var n int
	var err error
	var out string

	for {
		setDeadline(conn)
		n, err = conn.Read(recvData)
		if n <= 0 || err != nil {
			if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
				log.Println("telnet: read timeout:", err)
				return "", err
			} else {
				// some other error, do something else, for example create new conn
				/* TODO -- should modify attributes of the error, or raise a non-recoverable one */
				/* ie, broken pipe errors will not be solved by retries */
				log.Println("telnet: read error:", err)
				return "", err
			}
		} else {
			out += string(recvData)
		}
		if len(out) > 1 && out[len(out)-len(telnetTerminator):] == telnetTerminator {
			break
		}
	}
	return out, nil
}

func telnetSend(conn gote.Connection, command string) error {
	var commandBuffer []byte

	setDeadline(conn)

	for _, char := range command {
		commandBuffer = append(commandBuffer, byte(char))
	}

	var crlfBuffer [2]byte = [2]byte{'\r', '\n'}
	crlf := crlfBuffer[:]

	_, err := conn.Write(commandBuffer)
	if err != nil {
		return err
	}

	_, err = conn.Write(crlf)
	if err != nil {
		return err
	}
	return nil
}

func setDeadline(conn gote.Connection) error {
	/* TODO do we need to set deadline explicitely here? */
	t, err := time.ParseDuration(telnetTimeout)
	if err != nil {
		log.Println("telnet: error parsing duration")
		return err
	}
	err = conn.SetDeadline(time.Now().Add(t * time.Second))
	if err != nil {
		log.Println("telnet: error setting deadline")
		return err
	}
	return nil
}
