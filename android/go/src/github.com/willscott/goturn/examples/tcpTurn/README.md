TURN-indirected web request
============================

This example uses the STUN Client to establish a TCP connection through a
TURN server.

This example uses an AppRTC-like system for credential distribution, where it
makes an HTTP request to a URL containing a JSON-encoded set of credentials.

Usage
-----

```
go run main.go [--credentials https://example.com/credentials.json] [--url http://myip.info]
```
