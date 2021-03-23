package monolith

import "errors"

type Args struct {
	Values []interface{}
	Index  int
}

func NewEmptyArgs() *Args {
	return &Args{
		Values: make([]interface{}, 0),
		Index:  0,
	}
}

func NewArgs(values []interface{}) *Args {
	return &Args{
		Values: values,
		Index:  0,
	}
}

func (args Args) Empty() bool {
	return len(args.Values) <= 0
}

func (args Args) Pop() (interface{}, error) {
	if len(args.Values) > 0 {
		value, rest := args.Values[0], args.Values[1:]
		args.Values = rest
		return value, nil
	} else {
		return nil, errors.New("not enough args")
	}
}

func (args Args) PopInt() (int, error) {
	value, popError := args.Pop()
	if popError != nil {
		return 0, popError
	}

	if n, ok := value.(int); ok {
		return n, nil
	} else {
		return 0, errors.New("value was not an int")
	}
}

func (args Args) PopByte() (byte, error) {
	value, popError := args.Pop()
	if popError != nil {
		return 0, popError
	}

	if b, ok := value.(byte); ok {
		return b, nil
	}

	if n, ok := value.(int); ok {
		return byte(n), nil
	}

	return 0, errors.New("value was not an int")
}

func (args Args) Push(value interface{}) {
	args.Values = append(args.Values, value)
}