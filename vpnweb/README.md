vpnweb
======
A minimalistic webapp in Go to service API/JSON for the LEAP VPN


Configuration
-------------

You can pass generic configuration options either as a flag or an environment
variable. Some specific options can be passed only as env vars (like
authentication credentials).

| Flag               |  Env var                 |  default             | Description                                            |
|--------------------|:------------------------:|----------------------|-------------------------------------------------------:|
| **apiPath**        | `VPNWEB_API_PATH`        | /etc/leap/config/vpn | _Path for the public API static files_                 |
| **providerCaCrt**  | `VPNWEB_PROVIDER_CA`     | /etc/leap/ca/ca.crt  | _Path for the provider CA certificate_                 |
| **port**           | `VPNWEB_PORT`            | 8000                 | _Port where the api server will listen_                |
| **metricsPort**    | `VPNWEB_METRICS_PORT`    | 8001                 | _Port where the metrics server will listen_            |
| **tls**            |                          | false                | _Enable TLS on the service_                            |
| **tlsCrt**         | `VPNWEB_TLSCRT`          |                      | _Path to the cert file for TLS_                        |
| **tlsKey**         | `VPNWEB_TLSKEY`          |                      | _Path to the key file for TLS_                         |
| **vpnCaCrt**       | `VPNWEB_CACRT`           |                      | _Path to the CA public key used for VPN certificates_  |
| **vpnCaKey**       | `VPNWEB_CAKEY`           |                      | _Path to the CA private key used for VPN certificates_ |
| **auth**           | `VPNWEB_AUTH`            |                      | _Authentication module to use (one of: sip2, anon)_    |
| **authSecret**     | `VPNWEB_AUTH_SECRET`     |                      | _Authentication secret to sign auth tokens_            |
| **passwordPolicy** | `VPNWEB_PASSWORD_POLICY` | require              | _Password policy, if used (one of: require, ignore)_   |


SIP2 authentication:

|  Env var                      | Description                                            |
|:-----------------------------:|-------------------------------------------------------:|
| `VPNWEB_SIP_USER`             | Administrative user for the SIP2 telnet interface      |
| `VPNWEB_SIP_PASS`             | Administrative pass for the SIP2 telnet interface      |
| `VPNWEB_SIP_HOST`             | Host to connect to the SIP2 telnet interface           |
| `VPNWEB_SIP_PORT`             | Port to connect to the SIP2 telnet interface           |
| `VPNWEB_SIP_LIBR_LOCATION`    | Library location identifier                            |
| `VPNWEB_SIP_TERMINATOR`       | Telnet terminator used by the endpoint (default: \r\n) |


