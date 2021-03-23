package monolith

type Empty struct {
}

func (part Empty) Parse(buffer *Buffer, args *Args, context *Context) {
}

func (part Empty) Validate(buffer *Buffer, context *Context) Validity {
	return Valid
}

func (part Empty) MessageFromArgs(args *Args, context *Context) Message {
	return nil
}

func (part Empty) Count() int {
	return 0
}
