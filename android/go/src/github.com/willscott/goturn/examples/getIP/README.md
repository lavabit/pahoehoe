Stun IP address example
=======================

This example uses the STUN protocol to determine the externally visible address
of a locally bound port. In particular, it demonstrates the construction and
sending of a STUN request, and the parsing of a received reply.

Usage
-----

```
go run main.go [--server stun.l.google.com:19302]
```

The response IP address will be logged to stdout.
