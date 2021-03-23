package interconv_test

import (
	"encoding/json"
	"testing"

	"github.com/mufti1/interconv/package"
)

func TestParseUint(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseUint(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseUint(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}

func TestParseUint8(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseUint8(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseUint8(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}
func TestParseUint16(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseUint16(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseUint16(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}

func TestParseUint32(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseUint32(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseUint32(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}

func TestParseUint64(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseUint64(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseUint64(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}

func TestParseUintptr(t *testing.T) {
	t.Run("Json Number", func(t *testing.T) {
		var i interface{} = json.Number("20")
		convertedNum, _ := interconv.ParseUintptr(i)
		if convertedNum != 20 {
			t.Fatalf("return must be 20.4 instead of %v", convertedNum)
		}
	})
	t.Run("NOK", func(t *testing.T) {
		var i interface{} = 20
		_, err := interconv.ParseUintptr(i)
		if err == nil {
			t.Fatalf("it should be error: %v", err)
		}
	})
}
