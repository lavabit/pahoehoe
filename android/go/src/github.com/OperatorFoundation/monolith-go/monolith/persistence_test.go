package monolith

import (
	"fmt"
	"reflect"
	"testing"
)

func TestInstance_Persistence(t *testing.T) {
	parts := make([]Monolith, 0)
	part := BytesPart{
		Items: []ByteType{FixedByteType{Byte: 0x0A}},
	}
	parts = append(parts, part)
	desc := Description{parts}

	instance := Instance{
		Desc: desc,
		Args: nil,
	}

	result, marshalError := instance.Encode()
	if marshalError != nil {
		t.Fail()
		return
	}
	fmt.Println("result", result)

	i2, unmarshalError := DecodeInstance(result)
	if unmarshalError != nil {
		t.Fail()
		return
	}

	if !reflect.DeepEqual(instance, *i2) {
		fmt.Println("ne", instance, i2)
		fmt.Println(instance.Args, i2.Args, reflect.DeepEqual(instance.Args, i2.Args))
		t.Fail()
		return
	}
}

func TestDescription_Persistence(t *testing.T) {
	parts := make([]Monolith, 0)
	part := BytesPart{
		Items: []ByteType{FixedByteType{Byte: 0x0A}},
	}
	parts = append(parts, part)
	desc := Description{parts}

	result, marshalError := desc.Encode()
	if marshalError != nil {
		t.Fail()
		return
	}
	fmt.Println("result", result)

	d2, unmarshalError := DecodeDescription(result)
	if unmarshalError != nil {
		t.Fail()
		return
	}

	if !reflect.DeepEqual(desc, *d2) {
		fmt.Println("ne", desc, d2)
		t.Fail()
		return
	}
}
