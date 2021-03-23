package monolith

type Monolith interface {
	Messageable
	Parseable
	Validateable
	Countable
}

type Description struct {
	Parts []Monolith
}

type Byteable interface {
	Bytes() []byte
}

type Messageable interface {
	MessageFromArgs(args *Args, context *Context) Message
}

type BytesPart struct {
	Items []ByteType
}

type ByteType interface {
	Validateable
	Parseable
	Countable
	ByteFromArgs(args *Args, context *Context) (byte, error)
}

type FixedByteType struct {
	Byte byte
}

type EnumeratedByteType struct {
	Options []byte
}

type RandomByteType struct {

}

type RandomEnumeratedByteType struct {
	RandomOptions []byte
}

type SemanticByteType struct {

}

type Message interface {
	Byteable
}

type BytesMessage struct {
	bytes []byte
}

func (message BytesMessage) Bytes() []byte {
	return message.bytes
}
