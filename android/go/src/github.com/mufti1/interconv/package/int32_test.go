package interconv_test

import (
	"encoding/json"
	"reflect"
	"testing"

	interconv "github.com/mufti1/interconv/package"
)

func TestParseInt32(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseInt32(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseInt32(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
	t.Run("should be int32", func(t *testing.T) {
		var expectedDataType = reflect.Int32
		var i interface{} = 20

		val, err := interconv.ParseInt32(i)
		actualDataType := reflect.TypeOf(val).Kind()

		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}

		if actualDataType != expectedDataType {
			t.Fatalf("Expects to be type of %v, but got type of %v.", expectedDataType, actualDataType)
		}
	})

}
