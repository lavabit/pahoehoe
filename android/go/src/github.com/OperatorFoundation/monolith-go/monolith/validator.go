package monolith

import "github.com/deckarep/golang-set"

type Validity int

const (
	Valid Validity = iota
	Invalid
	Incomplete
)

func (v Validity) String() string {
	return [...]string{"Valid", "Invalid", "Incomplete"}[v]
}

type Validateable interface {
	Validate(buffer *Buffer, context *Context) Validity
}

func (description Description) Validate(buffer *Buffer, context *Context) Validity {
	bs2 := buffer
	valid := Valid

	for _, part := range description.Parts {
		valid = part.Validate(bs2, context)
		switch valid {
		case Valid:
			continue
		case Invalid:
			return Invalid
		case Incomplete:
			return Incomplete
		default:
			return Invalid
		}
	}

	return Valid
}

func (part BytesPart) Validate(buffer *Buffer, context *Context) Validity {
	bs2 := buffer
	valid := Valid

	for index := 0; index < len(part.Items); index++ {
		valid = part.Items[index].Validate(bs2, context)
		switch valid {
		case Valid:
			continue
		case Invalid:
			return Invalid
		case Incomplete:
			return Incomplete
		default:
		}
	}

	return Valid
}

func (bt FixedByteType) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Incomplete
	}

	b, popError := buffer.Pop()
	if popError != nil {
		return Invalid
	}

	if b == bt.Byte {
		return Valid
	} else {
		return Invalid
	}
}

func (bt EnumeratedByteType) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Incomplete
	}

	b, popError := buffer.Pop()
	if popError != nil {
		return Invalid
	}

	options := make([]interface{}, len(bt.Options))
	for index, option := range bt.Options {
		options[index] = option
	}

	set := mapset.NewSetFromSlice(options)
	if set.Contains(b) {
		return Valid
	} else {
		return Invalid
	}
}

func (bt RandomByteType) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Incomplete
	}

	_, popError := buffer.Pop()
	if popError != nil {
		return Invalid
	}

	return Valid
}

func (bt RandomEnumeratedByteType) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Incomplete
	}

	b, popError := buffer.Pop()
	if popError != nil {
		return Invalid
	}

	options := make([]interface{}, len(bt.RandomOptions))
	for index, option := range bt.RandomOptions {
		options[index] = option
	}

	set := mapset.NewSetFromSlice(options)

	if set.Contains(b) {
		return Valid
	} else {
		return Invalid
	}
}

