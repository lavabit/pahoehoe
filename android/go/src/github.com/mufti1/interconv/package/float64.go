package interconv

import (
	"encoding/json"
	"fmt"
)

// ParseFloat64 function to convert other type data to float64
func ParseFloat64(val interface{}) (float64, error) {
	switch val.(type) {
	case nil:
		return 0, nil
	case json.Number:
		return val.(json.Number).Float64()
	default:
		return 0, fmt.Errorf("unable to casting number %v (type %T)", val, val)
	}
}

// ParseFloat32 function to convert other type data to float32
func ParseFloat32(val interface{}) (float32, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return float32(0), err
	}
	return float32(number), nil
}
