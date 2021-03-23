package interconv

// ParseInt64 function to convert other type data to int64
func ParseInt64(val interface{}) (int64, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return 0, err
	}
	return int64(number), nil
}
