package interconv

// ParseInt16 function converts data of any type to int16
func ParseInt16(val interface{}) (int16, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return 0, err
	}
	return int16(number), err
}
