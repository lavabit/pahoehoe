package interconv

// ParseUint function to convert interface to uint
func ParseUint(val interface{}) (uint, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return uint(0), err
	}
	return uint(number), nil
}

// ParseUint8 function to convert interface data to uint8
func ParseUint8(val interface{}) (uint8, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return uint8(0), err
	}
	return uint8(number), nil
}

// ParseUint16 function to convert interface data to uint16
func ParseUint16(val interface{}) (uint16, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return uint16(0), err
	}
	return uint16(number), nil
}

// ParseUint32 function to convert interface data to uint32
func ParseUint32(val interface{}) (uint32, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return uint32(0), err
	}
	return uint32(number), nil
}

// ParseUint64 function to convert interface data to uint64
func ParseUint64(val interface{}) (uint64, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return uint64(0), err
	}
	return uint64(number), nil
}

// ParseUintptr function to convert interface data to uintptr
func ParseUintptr(val interface{}) (uintptr, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return uintptr(0), err
	}
	return uintptr(number), nil
}
