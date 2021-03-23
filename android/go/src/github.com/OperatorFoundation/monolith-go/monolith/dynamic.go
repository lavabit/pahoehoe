package monolith

import "math/rand"

type DynamicPart interface {
	Fix(n int) BytesPart
}

type ArgsDynamicPart struct {
	Item ByteType
}

type SemanticLengthConsumerDynamicPart struct {
	Name string
	Item ByteType
	Cached *BytesPart
}

type SemanticSeedConsumerDynamicPart struct {
	Name string
	Item ByteType
	Cached *BytesPart
}

func (part ArgsDynamicPart) Fix(n int) BytesPart {
	var items = make([]ByteType, n)

	for index := 0; index < n; n++ {
		items[index] = part.Item
	}

	return BytesPart{Items:items}
}

func (part ArgsDynamicPart) MessageFromArgs(args *Args, context *Context) Message {
	if args.Empty() {
		return nil
	}

	n, popError := args.PopInt()
	if popError != nil {
		return nil
	}

	bp := part.Fix(n)
	return bp.MessageFromArgs(args, context)
}

func (part *SemanticLengthConsumerDynamicPart) Fix(n int) BytesPart {
	var items = make([]ByteType, n)

	for index := 0; index < n; index++ {
		items[index] = part.Item
	}

	return BytesPart{Items:items}
}

func (part *SemanticLengthConsumerDynamicPart) Parse(buffer *Buffer, args *Args, context *Context) {
	if buffer.Empty() {
		return
	}

	if n, ok := context.GetInt(part.Name); ok {
		bp := part.Fix(n)
		part.Cached = &bp
		part.Cached.Parse(buffer, args, context)
	}
}

func (part *SemanticLengthConsumerDynamicPart) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Invalid
	}

	if n, ok := context.GetInt(part.Name); ok {
		bp := part.Fix(n)
		part.Cached = &bp
		return part.Cached.Validate(buffer, context)
	} else {
		return Invalid
	}
}

func (part *SemanticLengthConsumerDynamicPart) MessageFromArgs(args *Args, context *Context) Message {
	if n, ok := context.GetInt(part.Name); ok {
		bp := part.Fix(n)
		part.Cached = &bp
		return bp.MessageFromArgs(args, context)
	} else {
		return nil
	}
}

func (part *SemanticLengthConsumerDynamicPart) Count() int {
	if part.Cached != nil {
		return part.Cached.Count()
	} else {
		return 0
	}
}

func (part *SemanticSeedConsumerDynamicPart) Fix(seed int) BytesPart {
	r := rand.New(rand.NewSource(int64(seed)))
	n := r.Intn(256)
	var items = make([]ByteType, n)

	for index := 0; index < n; index++ {
		items[index] = part.Item
	}

	return BytesPart{Items:items}
}

func (part *SemanticSeedConsumerDynamicPart) Parse(buffer *Buffer, args *Args, context *Context) {
	if buffer.Empty() {
		return
	}

	if seed, ok := context.GetInt(part.Name); ok {
		bp := part.Fix(seed)
		part.Cached = &bp
		part.Cached.Parse(buffer, args, context)
	}
}

func (part *SemanticSeedConsumerDynamicPart) Validate(buffer *Buffer, context *Context) Validity {
	if buffer.Empty() {
		return Invalid
	}

	if seed, ok := context.GetInt(part.Name); ok {
		bp := part.Fix(seed)
		part.Cached = &bp
		return part.Cached.Validate(buffer, context)
	} else {
		return Invalid
	}
}

func (part *SemanticSeedConsumerDynamicPart) MessageFromArgs(args *Args, context *Context) Message {
	if seed, ok := context.GetInt(part.Name); ok {
		bp := part.Fix(seed)
		part.Cached = &bp
		return bp.MessageFromArgs(args, context)
	} else {
		return nil
	}
}

func (part *SemanticSeedConsumerDynamicPart) Count() int {
	if part.Cached != nil {
		return part.Cached.Count()
	} else {
		return 0
	}
}
