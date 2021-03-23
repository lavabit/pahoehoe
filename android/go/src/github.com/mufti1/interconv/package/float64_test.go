package interconv_test

import (
	"encoding/json"
	"reflect"
	"testing"

	"github.com/mufti1/interconv/package"
)

func TestParseFloat64(t *testing.T) {
	t.Run("Nil input", func(t *testing.T) {
		var i interface{}
		convertedNum, _ := interconv.ParseFloat64(i)
		if convertedNum != 0 {
			t.Fatalf("return must be 0 instead of %v", convertedNum)
		}
	})
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20.4")
		convertedNum, _ := interconv.ParseFloat64(i)
		if convertedNum != 20.4 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20.4
		_, err := interconv.ParseFloat64(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}

func TestParseFloat32(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseFloat32(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseFloat32(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
	t.Run("should be int", func(t *testing.T) {
		var expectedDataType = reflect.Float32
		var i interface{} = 20

		val, err := interconv.ParseFloat32(i)
		actualDataType := reflect.TypeOf(val).Kind()

		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}

		if actualDataType != expectedDataType {
			t.Fatalf("Expects to be type of %v, but got type of %v.", expectedDataType, actualDataType)
		}
	})
}
