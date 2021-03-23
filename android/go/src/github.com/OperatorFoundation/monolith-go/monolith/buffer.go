package monolith

import "errors"

type Buffer struct {
	value []byte
}

func NewEmptyBuffer() *Buffer {
	value := make([]byte, 0)
	return &Buffer{value:value}
}

func NewBuffer(value []byte) *Buffer {
	return &Buffer{value:value}
}

func (buffer *Buffer) Empty() bool {
	return len(buffer.value) == 0
}

func (buffer *Buffer) Pop() (byte, error) {
	if buffer.Empty() {
		return 0, errors.New("buffer is empty")
	}

	b := buffer.value[0]
	buffer.value = buffer.value[1:]

	return b, nil
}

func (buffer *Buffer) PopBytes(n int) ([]byte, error) {
	if len(buffer.value) < n {
		return []byte{}, errors.New("buffer is short")
	}

	bs := buffer.value[n:]
	buffer.value = buffer.value[n+1:]

	return bs, nil
}

func (buffer *Buffer) Push(bs []byte) {
	if bs == nil {
		return
	}

	if len(bs) == 0 {
		return
	}

	buffer.value = append(buffer.value, bs...)
}