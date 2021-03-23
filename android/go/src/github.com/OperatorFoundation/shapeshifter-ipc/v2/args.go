package pt

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"sort"
	"strings"
)

// Return the index of the next unescaped byte in s that is in the term set, or
// else the length of the string if no terminators appear. Additionally return
// the unescaped string up to the returned index.
func indexUnescaped(s string, term []byte) (int, string, error) {
	var i int
	unesc := make([]byte, 0)
	for i = 0; i < len(s); i++ {
		b := s[i]
		// A terminator byte?
		if bytes.IndexByte(term, b) != -1 {
			break
		}
		if b == '\\' {
			i++
			if i >= len(s) {
				return 0, "", fmt.Errorf("nothing following final escape in %q", s)
			}
			b = s[i]
		}
		unesc = append(unesc, b)
	}
	return i, string(unesc), nil
}

func ParsePT2ClientParameters(s string) (map[string]interface{}, error) {
	if len(s) == 0 {
		return nil, errors.New("cannot use empty string")
	}

	decoder := json.NewDecoder(strings.NewReader(s))
	var result map[string]interface{}
	if err := decoder.Decode(&result); err != nil {
		fmt.Errorf("Error decoding JSON %q", err)
		return nil, err
	}
	return result, nil
}

func ParsePT2ServerParameters(s string) (params map[string]map[string]interface{}, err error) {
	if len(s) == 0 {
		return nil, errors.New("cannot use an empty string")
	}

	decoder := json.NewDecoder(strings.NewReader(s))
	var result map[string]map[string]interface{}
	if err := decoder.Decode(&result); err != nil {
		fmt.Errorf("Error decoding JSON %q", err)
		return nil, err
	}

	return result, nil
}

// Escape backslashes and all the bytes that are in set.
func backslashEscape(s string, set []byte) string {
	var buf bytes.Buffer
	for _, b := range []byte(s) {
		if b == '\\' || bytes.IndexByte(set, b) != -1 {
			buf.WriteByte('\\')
		}
		buf.WriteByte(b)
	}
	return buf.String()
}

// Encode a name–value mapping so that it is suitable to go in the ARGS option
// of an SMETHOD line. The output is sorted by key. The "ARGS:" prefix is not
// added.
//
// "Equal signs and commas [and backslashes] must be escaped with a backslash."
func encodeSmethodArgs(args map[string]interface{}) string {
	if args == nil {
		return ""
	}

	keys := make([]string, 0, len(args))
	for key := range args {
		keys = append(keys, key)
	}
	sort.Strings(keys)

	escape := func(s string) string {
		return backslashEscape(s, []byte{'=', ','})
	}

	var pairs []string
	for _, key := range keys {
		value:= args[key]
		jsonByte, jsonError := json.Marshal(value)
		if jsonError != nil {
			continue
		}
		jsonString:= string(jsonByte)
		pairs = append(pairs, escape(key)+"="+escape(jsonString))

	}

	return strings.Join(pairs, ",")
}
