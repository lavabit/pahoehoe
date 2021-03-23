package monolith

type SemanticIntConsumerOptionalPart struct {
	Name string
	Condition Condition
	Item Monolith
	Cached Monolith
}

func (part *SemanticIntConsumerOptionalPart) Fix(n int) Monolith {
	if part.Condition.Evaluate(n) {
		return part.Item
	} else {
		return Empty{}
	}
}

func (part *SemanticIntConsumerOptionalPart) Parse(buffer *Buffer, args *Args, context *Context) {
	if buffer.Empty() {
		return
	}

	if n, ok := context.GetInt(part.Name); ok {
		part.Cached = part.Fix(n)
		part.Cached.Parse(buffer, args, context)
	}
}

func (part *SemanticIntConsumerOptionalPart) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Invalid
	}

	if n, ok := context.GetInt(part.Name); ok {
		part.Cached = part.Fix(n)
		return part.Cached.Validate(buffer, context)
	} else {
		return Invalid
	}
}

func (part *SemanticIntConsumerOptionalPart) MessageFromArgs(args *Args, context *Context) Message {
	if n, ok := context.GetInt(part.Name); ok {
		part.Cached = part.Fix(n)
		return part.Cached.MessageFromArgs(args, context)
	} else {
		return nil
	}
}

func (part *SemanticIntConsumerOptionalPart) Count() int {
	if part.Cached != nil {
		return part.Cached.Count()
	} else {
		return 0
	}
}
