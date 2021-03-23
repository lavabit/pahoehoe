package interconv

// ParseInt32 function to convert other type data to int
func ParseInt32(val interface{}) (int32, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return 0, err
	}
	return int32(number), nil
}
