package monolith

import "github.com/deckarep/golang-set"

type Parseable interface {
	Parse(buffer *Buffer, args *Args, context *Context)
}

func (description Description) Parse(buffer *Buffer, args *Args, context *Context) {
	for _, part := range description.Parts {
		part.Parse(buffer, args, context)
	}
}

func (part BytesPart) Parse(buffer *Buffer, args *Args, context *Context) {
	for index := 0; index < len(part.Items); index++ {
		part.Items[index].Parse(buffer, args, context)
	}
}

func (bt FixedByteType) Parse(buffer *Buffer, _ *Args, _ *Context) {
	if buffer.Empty() {
		return
	}

	_, popError := buffer.Pop()
	if popError != nil {
		return
	}
}

func (bt EnumeratedByteType) Parse(buffer *Buffer, _ *Args, _ *Context) {
	if buffer.Empty() {
		return
	}

	arg, popError := buffer.Pop()
	if popError != nil {
		return
	}

	options := make([]interface{}, len(bt.Options))
	for index, option := range bt.Options {
		options[index] = option
	}

	set := mapset.NewSetFromSlice(options)
	if set.Contains(arg) {
		return
	} else {
		return
	}
}

func (bt RandomByteType) Parse(buffer *Buffer, _ *Args, _ *Context) {
	if buffer.Empty() {
		return
	}

	_, popError := buffer.Pop()
	if popError != nil {
		return
	}

	return
}

func (bt RandomEnumeratedByteType) Parse(buffer *Buffer, _ *Args, _ *Context) {
	if buffer.Empty() {
		return
	}

	arg, popError := buffer.Pop()
	if popError != nil {
		return
	}

	options := make([]interface{}, len(bt.RandomOptions))
	for index, option := range options {
		options[index] = option
	}

	set := mapset.NewSetFromSlice(options)
	if set.Contains(arg) {
		return
	} else {
		return
	}
}

