# Interconv
[![Build Status](https://travis-ci.org/mufti1/interconv.svg?branch=master)](https://travis-ci.org/mufti1/interconv) [![Coverage Status](https://coveralls.io/repos/github/mufti1/interconv/badge.svg?branch=master)](https://coveralls.io/github/mufti1/interconv?branch=master) [![Go Report Card](https://goreportcard.com/badge/github.com/mufti1/interconv)](https://goreportcard.com/report/github.com/mufti1/interconv) [![GoDoc](https://godoc.org/github.com/mufti1/interconv/package?status.svg)](https://godoc.org/github.com/mufti1/interconv/package)<br />

## Description
This is package golang for convert interface into any data type

## How To Use
-   get package
```go
go get github.com/mufti1/interconv/package
```
-   import 
```go
import "github.com/mufti1/interconv/package"
```
-   use
```go
var i interface{}
// example for float64
// you can use error return if you need it
numFloat64 , _ := interconv.ParseFloat64(i)
```

## Status
Completed and open to update

## Data type that can be converted
-   Int
-   Int8
-   Int32
-   Int16
-   Int64
-   Float32
-   Float64
-   Boolean
-   String
-   Uint
-   Uint8
-   Uint16
-   Uint32
-   Uint64
-   Uintptr

## License

it is released under the MIT license. See 
[LICENSE](https://github.com/mufti1/interconv/blob/master/LICENSE).
