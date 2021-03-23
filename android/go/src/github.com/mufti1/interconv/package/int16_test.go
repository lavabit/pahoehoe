package interconv_test

import (
	"encoding/json"
	"testing"

	interconv "github.com/mufti1/interconv/package"
)

func TestParseInt16(t *testing.T) {
	t.Run("Nil", func(t *testing.T) {
		convertedNum, _ := interconv.ParseInt16(nil)
		if convertedNum != 0 {
			t.Fatalf("return must be 0 instead of %v", convertedNum)
		}
	})
	t.Run("Json Number", func(t *testing.T) {
		convertedNum, _ := interconv.ParseInt16(json.Number("100"))
		if convertedNum != 100.0 {
			t.Fatalf("return must be 100.0 instead of %v", convertedNum)
		}
	})
	t.Run("Error value", func(t *testing.T) {
		_, err := interconv.ParseInt16("test")
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}
