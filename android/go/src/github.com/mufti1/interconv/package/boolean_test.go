package interconv_test

import (
	"testing"

	"github.com/mufti1/interconv/package"
)

func TestParseBoolean(t *testing.T) {
	t.Run("Nil input", func(t *testing.T) {
		var i interface{}
		convertedBool, _ := interconv.ParseBoolean(i)
		if convertedBool != false {
			t.Fatalf("return must be false instead of %v", convertedBool)
		}
	})
	t.Run("Zero Number", func(t *testing.T) {
		var i interface{} = 0
		convertedBool, _ := interconv.ParseBoolean(i)
		if convertedBool != false {
			t.Fatalf("return must be false instead of %v", convertedBool)
		}
	})
	t.Run("String 0", func(t *testing.T) {
		var i interface{} = "0"
		convertedBool, _ := interconv.ParseBoolean(i)
		if convertedBool != false {
			t.Fatalf("return must be false instead of %v", convertedBool)
		}
	})
	t.Run("One Number", func(t *testing.T) {
		var i interface{} = 1
		convertedBool, _ := interconv.ParseBoolean(i)
		if convertedBool != true {
			t.Fatalf("return must be true instead of %v", convertedBool)
		}
	})
	t.Run("String 1", func(t *testing.T) {
		var i interface{} = "1"
		convertedBool, _ := interconv.ParseBoolean(i)
		if convertedBool != true {
			t.Fatalf("return must be true instead of %v", convertedBool)
		}
	})
	t.Run("NString", func(t *testing.T) {
		var i interface{} = "test string"
		_, err := interconv.ParseBoolean(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
	t.Run("NNumber", func(t *testing.T) {
		var i interface{} = 9999
		_, err := interconv.ParseBoolean(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}
