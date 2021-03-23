package interconv_test

import (
	"encoding/json"
	"testing"

	"github.com/mufti1/interconv/package"
)

func TestParseInt64(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseInt64(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseInt64(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}
