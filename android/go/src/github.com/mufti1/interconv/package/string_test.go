package interconv

import (
	"errors"
	"fmt"
	"testing"
)

func TestParseString(t *testing.T) {
	tt := []struct {
		Description string
		Input       interface{}
		Result      string
		Err         error
	}{
		{"empty value", "", "", nil},
		{"valid value", "valid value", "valid value", nil},
		{"nil value", nil, "", errors.New("unable to casting number <nil> (type <nil>)")},
		{"different type", 1, "", errors.New("unable to casting number 1 (type int)")},
	}

	for _, tc := range tt {
		t.Run(tc.Description, func(t *testing.T) {
			result, err := ParseString(tc.Input)
			if result != tc.Result {
				t.Fatalf("return must be %v instead of %v", tc.Result, result)
			}

			if fmt.Sprint(tc.Err) != fmt.Sprint(err) {
				t.Fatalf("error must be %v instead of %v", tc.Err, err)
			}
		})
	}
}
