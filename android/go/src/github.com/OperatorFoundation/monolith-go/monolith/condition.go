package monolith

import (
	"errors"
	"reflect"
)

type Condition interface {
	Evaluate(value interface{}) bool
}

type EqualsCondition struct {
	other interface{}
}

func (cond EqualsCondition) Evaluate(value interface{}) bool {
	return reflect.DeepEqual(value, cond.other)
}

type GreaterCondition struct {
	other interface{}
}

func (cond GreaterCondition) Evaluate(value interface{}) bool {
	a, aError := convertToInt64(value)
	if aError != nil {
		return false
	}

	b, bError := convertToInt64(cond.other)
	if bError != nil {
		return false
	}

	return a > b
}

type LesserCondition struct {
	other interface{}
}

func (cond LesserCondition) Evaluate(value interface{}) bool {
	a, aError := convertToInt64(value)
	if aError != nil {
		return false
	}

	b, bError := convertToInt64(cond.other)
	if bError != nil {
		return false
	}

	return a < b
}

func convertToInt64(value interface{}) (int64, error) {
	switch value.(type) {
	case int:
		return int64(value.(int)), nil
	case uint:
		return int64(value.(uint)), nil
	case int8:
		return int64(value.(int8)), nil
	case uint8:
		return int64(value.(uint8)), nil
	case int16:
		return int64(value.(int16)), nil
	case uint16:
		return int64(value.(uint16)), nil
	case int32:
		return int64(value.(int32)), nil
	case uint32:
		return int64(value.(uint32)), nil
	case int64:
		return value.(int64), nil
	case uint64:
		return int64(value.(uint64)), nil
	default:
		return 0, errors.New("could not convert to int64")
	}
}

