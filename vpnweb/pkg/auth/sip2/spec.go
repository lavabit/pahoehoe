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
	"errors"
	"log"
	"strconv"
	"strings"
)

const (
	yes                  string = "Y"
	no                   string = "N"
	trueVal              string = "1"
	falseVal             string = "0"
	okVal                string = "ok"
	language             string = "language"
	patronStatus         string = "patron status"
	onlineStatus         string = "on-line status"
	checkinOk            string = "checkin ok"
	checkoutOk           string = "checkout ok"
	renewalPolicy        string = "acs renewal policy"
	statusUpdate         string = "status update ok"
	offlineOK            string = "offline ok"
	timeoutPeriod        string = "timeout period"
	retriesAllowed       string = "retries allowed"
	dateTimeSync         string = "date/time sync"
	date                 string = "transaction date"
	patronIdentifier     string = "patron identifier"
	patronPassword       string = "patron password"
	protocolVersion      string = "protocol version"
	personalName         string = "personal name"
	screenMessage        string = "screen message"
	institutionID        string = "institution id"
	validPatron          string = "valid patron"
	validPatronPassword  string = "valid patron password"
	loginResponse        string = "Login Response"
	ascStatus            string = "ASC Status"
	patronStatusResponse string = "Patron Status Response"
)

type fixedFieldSpec struct {
	length int
	label  string
}

type fixedField struct {
	spec  fixedFieldSpec
	value string
}

type variableFieldSpec struct {
	id    string
	label string
}

type variableField struct {
	spec  variableFieldSpec
	value string
}

type messageSpec struct {
	id     int
	label  string
	fields []fixedFieldSpec
}

func toBool(val string) (bool, error) {
	var ret bool
	switch val {
	case trueVal:
		ret = true
	case falseVal:
		ret = false
	case yes:
		ret = true
	case no:
		ret = false
	default:
		return false, errors.New("cannot parse value")
	}
	return ret, nil
}

type message struct {
	fields      []variableField
	fixedFields []fixedField
	msgTxt      string
}

func (m *message) getFieldValue(field string) (string, bool) {
	for _, v := range m.fields {
		if v.spec.label == field {
			return v.value, true
		}
	}
	return "", false
}

func (m *message) getFixedFieldValue(field string) (string, bool) {
	for _, v := range m.fixedFields {
		if v.spec.label == field {
			return v.value, true
		}
	}
	return "", false
}

func (m *message) getValueByCode(code string) (string, bool) {
	for _, v := range m.fields {
		if v.spec.id == code {
			return v.value, true
		}
	}
	return "", false
}

type Parser struct {
	msgByCodeMap           map[int]messageSpec
	variableFieldByCodeMap map[string]variableFieldSpec
}

func getParser() *Parser {

	languageSpec := fixedFieldSpec{3, language}
	patronStatusSpec := fixedFieldSpec{14, patronStatus}
	dateSpec := fixedFieldSpec{18, date}
	okSpec := fixedFieldSpec{1, okVal}

	onlineStatusSpec := fixedFieldSpec{1, onlineStatus}
	checkinOkSpec := fixedFieldSpec{1, checkinOk}
	checkoutOkSpec := fixedFieldSpec{1, checkoutOk}
	renewalSpec := fixedFieldSpec{1, renewalPolicy}
	stUpdateSpec := fixedFieldSpec{1, statusUpdate}
	offlineOkSpec := fixedFieldSpec{1, offlineOK}
	timeoutSpec := fixedFieldSpec{3, timeoutPeriod}
	retriesSpec := fixedFieldSpec{3, retriesAllowed}
	dateTimeSyncSpec := fixedFieldSpec{18, dateTimeSync}
	protoSpec := fixedFieldSpec{4, protocolVersion}

	msgByCodeMap := map[int]messageSpec{
		24: messageSpec{24, patronStatusResponse, []fixedFieldSpec{patronStatusSpec, languageSpec, dateSpec}},
		94: messageSpec{94, loginResponse, []fixedFieldSpec{okSpec}},
		98: messageSpec{98, ascStatus, []fixedFieldSpec{onlineStatusSpec, checkinOkSpec, checkoutOkSpec, renewalSpec, stUpdateSpec, offlineOkSpec, timeoutSpec, retriesSpec, dateTimeSyncSpec, protoSpec}},
	}

	variableFieldByCodeMap := map[string]variableFieldSpec{
		"AA": variableFieldSpec{"AA", patronIdentifier},
		"AD": variableFieldSpec{"AD", patronPassword},
		"AE": variableFieldSpec{"AE", personalName},
		"AF": variableFieldSpec{"AF", screenMessage},
		"AO": variableFieldSpec{"AO", institutionID},
		"BL": variableFieldSpec{"BL", validPatron},
		"CQ": variableFieldSpec{"CQ", validPatronPassword},
	}

	return &Parser{msgByCodeMap, variableFieldByCodeMap}
}

func (p *Parser) getMessageSpecByCode(code int) messageSpec {
	return p.msgByCodeMap[code]
}

func (p *Parser) getVariableFieldByCode(code string) variableFieldSpec {
	return p.variableFieldByCodeMap[code]
}

func (p *Parser) getFixedFieldValue(msg *message, field string) (string, bool) {
	for _, v := range msg.fixedFields {
		if v.spec.label == field {
			return v.value, true
		}
	}
	return "", false
}

func (p *Parser) parseMessage(msg string) (*message, error) {
	if len(msg) == 0 {
		return &message{}, errors.New("empty message")
	}
	if len(msg) < (2 + len(telnetTerminator)) {
		return &message{}, errors.New("parseMessage: message too short")
	}
	txt := msg[:len(msg)-len(telnetTerminator)]
	code, err := strconv.Atoi(txt[:2])
	if nil != err {
		log.Printf("Error parsing integer: %s\n", txt[:2])
		return &message{}, errors.New("parseMessage: error parsing integer")
	}
	spec := p.getMessageSpecByCode(code)
	txt = txt[2:]

	message := new(message)
	for _, sp := range spec.fields {
		value := txt[:sp.length]
		txt = txt[sp.length:]
		message.fixedFields = append(message.fixedFields, fixedField{sp, value})
	}
	if len(txt) == 0 {
		return message, nil
	}
	for _, part := range strings.Split(txt, "|") {
		if len(part) > 0 {
			partSpec := p.getVariableFieldByCode(part[:2])
			value := part[2:]
			message.fields = append(message.fields, variableField{partSpec, value})
		}
	}
	return message, nil
}
