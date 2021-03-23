package monolith

import "errors"

// Produces an int into a Context
type SemanticIntProducerByteType struct {
	Name string
	Value ByteType
}

func (bt SemanticIntProducerByteType) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Incomplete
	}

	b, popError := buffer.Pop()
	if popError != nil {
		return Invalid
	}

	subbuffer := NewBuffer([]byte{b})

	if bt.Value.Validate(subbuffer, context) == Valid {
		intvalue := int(b)
		context.Set(bt.Name, intvalue)

		return Valid
	} else {
		return Invalid
	}
}

func (bt SemanticIntProducerByteType) Parse(buffer *Buffer, _ *Args, context *Context) {
	if buffer.Empty() {
		return
	}

	b, popError := buffer.Pop()
	if popError != nil {
		return
	}

	value := int(b)

	context.Set(bt.Name, value)
}

func (bt SemanticIntProducerByteType) Count() int {
	return bt.Value.Count()
}

func (bt SemanticIntProducerByteType) ByteFromArgs(args *Args, context *Context) (byte, error) {
	b, byteError := bt.Value.ByteFromArgs(args, context)
	if byteError != nil {
		return 0, byteError
	}

	n := int(b)
	context.Set(bt.Name, n)
	return b, nil
}

// Consumes an int from a Context
type SemanticIntConsumerByteType struct {
	Name string
}

func (bt SemanticIntConsumerByteType) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Incomplete
	}

	b, popError := buffer.Pop()
	if popError != nil {
		return Invalid
	}

	n := int(b)

	if value, ok := context.GetInt(bt.Name); ok {
		if n == value {
			return Valid
		} else {
			return Invalid
		}
	} else {
		return Invalid
	}
}

func (bt SemanticIntConsumerByteType) Parse(buffer *Buffer, args *Args, context *Context) {
	if buffer.Empty() {
		return
	}

	b, popError := buffer.Pop()
	if popError != nil {
		return
	}

	n := int(b)

	if value, ok := context.GetInt(bt.Name); ok {
		if n == value {
			args.Push(n)
		}
	}
}

func (bt SemanticIntConsumerByteType) Count() int {
	return 1
}

func (bt SemanticIntConsumerByteType) ByteFromArgs(_ *Args, context *Context) (byte, error) {
	if value, ok := context.GetInt(bt.Name); ok {
		b := byte(value)
		return b, nil
	} else {
		return 0, errors.New("undefined variable")
	}
}
