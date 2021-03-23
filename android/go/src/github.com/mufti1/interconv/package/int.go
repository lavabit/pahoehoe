package interconv

// ParseInt function to convert other type data to int
func ParseInt(val interface{}) (int, error) {
	number, err := ParseFloat64(val)
	if err != nil {
		return 0, err
	}
	return int(number), nil
}
