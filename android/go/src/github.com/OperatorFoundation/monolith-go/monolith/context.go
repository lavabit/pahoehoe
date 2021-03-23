package monolith

type Context struct {
	values map[string]interface{}
}

func NewEmptyContext() *Context {
	values := make(map[string]interface{}, 0)
	return &Context{values:values}
}

func (context *Context) Set(name string, value interface{}) {
	context.values[name] = value
}

func (context *Context) Get(name string) (interface{}, bool) {
	if value, ok := context.values[name]; ok {
		return value, ok
	} else {
		return nil, false
	}
}

func (context *Context) GetInt(name string) (int, bool) {
	if value, ok := context.Get(name); ok {
		if n, ok := value.(int); ok {
			return n, true
		} else {
			return 0, false
		}
	} else {
		return 0, false
	}
}
