package interconv

// ParseInt8 function converts data of any type to int8
func ParseInt8(val interface{}) (int8, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return 0, err
	}
	return int8(number), err
}
