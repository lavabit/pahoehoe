package monolith

import (
	"errors"
	mapset "github.com/deckarep/golang-set"
)
import "math/rand"

type Instance struct {
	Desc Description
	Args *Args
}

func (instance Instance) Messages() []Message {
	context := NewEmptyContext()
	ms := instance.Desc.MessagesFromArgs(instance.Args, context)
	result := make([]Message, 0)
	for _, m := range ms {
		if m != nil {
			result = append(result, m)
		}
	}

	return result
}

func (description Description) MessagesFromArgs(args *Args, context *Context) []Message {
	var m Message
	result := make([]Message, 0)

	for _, part := range description.Parts {
		m = part.MessageFromArgs(args, context)
		result = append(result, m)
	}

	return result
}

func (part BytesPart) MessageFromArgs(args *Args, context *Context) Message {
	result := make([]byte, 0)

	for index := 0; index < len(part.Items); index++ {
		b, byteError := part.Items[index].ByteFromArgs(args, context)
		if byteError != nil {
			continue
		} else {
			result = append(result, b)
		}
	}

	m := BytesMessage{bytes: result}
	return m
}

func (bt FixedByteType) ByteFromArgs(_ *Args, _ *Context) (byte, error) {
	return bt.Byte, nil
}

func (bt EnumeratedByteType) ByteFromArgs(args *Args, _ *Context) (byte, error) {
	b, popError := args.PopByte()
	if popError != nil {
		return 0, popError
	}

	options := make([]interface{}, len(bt.Options))
	for index, option := range bt.Options {
		options[index] = option
	}
	set := mapset.NewSetFromSlice(options)
	if set.Contains(b) {
		return b, nil
	} else {
		return 0, errors.New("invalid arg")
	}
}

func (bt RandomByteType) ByteFromArgs(args *Args, _ *Context) (byte, error) {
	bs := make([]byte, 1)
	rand.Read(bs)

	return bs[0], nil
}

func (bt RandomEnumeratedByteType) ByteFromArgs(args *Args, _ *Context) (byte, error) {
	index := rand.Intn(len(bt.RandomOptions))
	b := bt.RandomOptions[index]

	return b, nil
}
