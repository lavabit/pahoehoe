SIP2 authentication howto
=========================

stunnel configuration
---------------------

SIP2 has no encryption built-in. To encrypt traffic, we install stunnel both on
the server (the one with the koha instance) and the client (the vpnweb node):

```
sudo apt-get install stunnel4
```

Set value to ENABLED in the config file:

```
/etc/default/stunnel4
```

Configure the server, see `docs/examples/stunnel.conf-server-sample`:

```
[sip2]
accept  = 6443
connect = 6001
cert    = /etc/stunnel/ssl/sip2-cert.pem
key     = /etc/stunnel/ssl/sip2-key.pem
```

You will need to generate a certificate pair for this service. For a throwaway test deployment,
you can look in the `test/simple-ca` folder.

Now configure the client side (the box where the vpnweb instance is running). Create /etc/stunnel/sip.conf:

```
[sip2]
accept  = localhost:6001
connect = koha.example.org:6443
client  = yes
```

Now you can start the service:

```
sudo /etc/init.d/stunnel4 start
```

And configure vpnweb to connect to the local port:

```
export VPNWEB_SIP_HOST=localhost
export VPNWEB_SIP_PORT=6001  
```
