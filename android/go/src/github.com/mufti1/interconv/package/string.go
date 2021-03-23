package interconv

import (
	"fmt"
)

// ParseString function to convert other type data to string
func ParseString(value interface{}) (string, error) {
	switch value.(type) {
	case string:
		return value.(string), nil
	default:
		return "", fmt.Errorf("unable to casting number %v (type %T)", value, value)
	}
}
