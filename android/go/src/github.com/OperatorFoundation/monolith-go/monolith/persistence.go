package monolith

import (
	"bytes"
	"encoding/base64"
	"encoding/gob"
)

func InitializeGobRegistry() {
	gob.Register(BytesPart{})
	gob.Register(FixedByteType{})
	gob.Register(RandomByteType{})
	gob.Register(EnumeratedByteType{})
	gob.Register(RandomEnumeratedByteType{})
}

func (instance Instance) Encode() (string, error) {
	InitializeGobRegistry()

	var buffer bytes.Buffer
	encoder := gob.NewEncoder(&buffer)

	marshalError := encoder.Encode(instance)
	if marshalError != nil {
		return "", marshalError
	}

	encoded := base64.StdEncoding.EncodeToString(buffer.Bytes())
	return encoded, nil
}

func DecodeInstance(encoded string) (*Instance, error) {
	decoded, base64Error := base64.StdEncoding.DecodeString(encoded)
	if base64Error != nil {
		return nil, base64Error
	}

	var buffer bytes.Buffer
	buffer.Write(decoded)

	decoder := gob.NewDecoder(&buffer)

	var instance Instance
	unmarshalError := decoder.Decode(&instance)
	if unmarshalError != nil {
		return nil, unmarshalError
	}

	return &instance, nil
}

func (description Description) Encode() (string, error) {
	InitializeGobRegistry()

	var buffer bytes.Buffer
	encoder := gob.NewEncoder(&buffer)

	marshalError := encoder.Encode(description)
	if marshalError != nil {
		return "", marshalError
	}

	encoded := base64.StdEncoding.EncodeToString(buffer.Bytes())
	return encoded, nil
}

func DecodeDescription(encoded string) (*Description, error) {
	decoded, base64Error := base64.StdEncoding.DecodeString(encoded)
	if base64Error != nil {
		return nil, base64Error
	}

	var buffer bytes.Buffer
	buffer.Write(decoded)

	decoder := gob.NewDecoder(&buffer)

	var description Description
	unmarshalError := decoder.Decode(&description)
	if unmarshalError != nil {
		return nil, unmarshalError
	}

	return &description, nil
}
