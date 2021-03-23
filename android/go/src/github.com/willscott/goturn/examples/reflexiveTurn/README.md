TURN loop-back example
=======================

This is a low-level example using TURN to send a packet back to itself.
It demonstrates creating a reservation, and transmitting data through the
TURN proxy. This example uses an in-line send indication, meaning that data is
sent back to a single UDP socket that has been bound on the client. It is
generally more functional to create a second channel for transmitting and
receiving data.

This example uses an AppRTC-like system for credential distribution, where it
makes an HTTP request to a URL containing a JSON-encoded set of credentials.

Usage
-----

```
go run main.go [--credentials https://example.com/credentials.json]
```

Status on reservation, establishment, and transmission of data will be
logged to standard out.
