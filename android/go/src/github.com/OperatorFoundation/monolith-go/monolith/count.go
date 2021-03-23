package monolith

type Countable interface {
	Count() int
}

func (part BytesPart) Count() int {
	result := 0

	for index := 0; index < len(part.Items); index++ {
		c := part.Items[index].Count()
		result = result + c
	}

	return result
}

func (bt FixedByteType) Count() int {
	return 1
}

func (bt EnumeratedByteType) Count() int {
	return 1
}

func (bt RandomByteType) Count() int {
	return 1
}

func (bt RandomEnumeratedByteType) Count() int {
	return 1
}

func (part TimedPart) Count() int {
	result := 0

	for index := 0; index < len(part.Items); index++ {
		c := part.Items[index].Count()
		result = result + c
	}

	return result
}