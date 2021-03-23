package interconv

import (
	"fmt"
)

// ParseBoolean function to convert other type data to boolean
func ParseBoolean(val interface{}) (bool, error) {
	switch val {
	case nil:
		return false, nil
	case "1":
		return true, nil
	case 1:
		return true, nil
	case "0":
		return false, nil
	case 0:
		return false, nil
	default:
		return false, fmt.Errorf("unable to casting %v (type %T)", val, val)
	}
}
