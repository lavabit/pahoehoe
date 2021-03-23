package monolith

type TimedPart struct {
	Milliseconds uint
	Items        []ByteType
}

type TimedMessage struct {
	Milliseconds uint
	bytes        []byte
}

func (message TimedMessage) Bytes() []byte {
	return message.bytes
}

func (part TimedPart) MessageFromArgs(args *Args, context *Context) Message {
	var result []byte

	for index := 0; index < len(part.Items); index++ {
		b, byteError := part.Items[index].ByteFromArgs(args, nil)
		if byteError != nil {
			continue
		} else {
			result = append(result, b)
		}
	}

	m := TimedMessage{
		Milliseconds: part.Milliseconds,
		bytes:        result,
	}

	return m
}

func (part TimedPart) Parse(buffer *Buffer, args *Args, context *Context) {
	resultBytes := buffer
	resultArgs := args

	for index := 0; index < len(part.Items); index++ {
		part.Items[index].Parse(resultBytes, resultArgs, nil)
	}

	return
}

func (part TimedPart) Validate(buffer *Buffer, context *Context) Validity {
	bs2 := buffer
	valid := Valid

	for index := 0; index < len(part.Items); index++ {
		valid = part.Items[index].Validate(bs2, nil)
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
