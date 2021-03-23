package main

import (
  "log"
  "net/http"
  "path/filepath"

  "0xacab.org/leap/vpnweb/pkg/auth"
  "0xacab.org/leap/vpnweb/pkg/config"
  "0xacab.org/leap/vpnweb/pkg/web"
)

func main() {
  opts := config.NewOpts()
  ch := web.NewCertHandler(opts.CaCrt, opts.CaKey)
  authenticator := auth.GetAuthenticator(opts, false)

  srv := http.NewServeMux()

  /* protected routes */
  srv.Handle("/cert", web.RestrictedMiddleware(authenticator.NeedsCredentials, ch.CertResponder, opts))
  srv.Handle("/1/cert", web.RestrictedMiddleware(authenticator.NeedsCredentials, ch.CertResponder, opts))
  srv.Handle("/3/cert", web.RestrictedMiddleware(authenticator.NeedsCredentials, ch.CertResponder, opts))

  /* files */
  web.HttpFileHandler(srv, "/ca.crt", opts.ProviderCaPath)
  web.HttpFileHandler(srv, "/geoip.json", filepath.Join(opts.ApiPath, "geoip.json"))
  web.HttpFileHandler(srv, "/provider.json", filepath.Join(opts.ApiPath, "provider.json"))

  web.HttpFileHandler(srv, "/1/ca.crt", opts.ProviderCaPath)
  web.HttpFileHandler(srv, "/1/configs.json", filepath.Join(opts.ApiPath, "1", "configs.json"))
  web.HttpFileHandler(srv, "/1/service.json", filepath.Join(opts.ApiPath, "1", "service.json"))
  web.HttpFileHandler(srv, "/1/config/eip-service.json", filepath.Join(opts.ApiPath, "1", "eip-service.json"))

  web.HttpFileHandler(srv, "/3/ca.crt", opts.ProviderCaPath)
  web.HttpFileHandler(srv, "/3/configs.json", filepath.Join(opts.ApiPath, "3", "configs.json"))
  web.HttpFileHandler(srv, "/3/service.json", filepath.Join(opts.ApiPath, "3", "service.json"))
  web.HttpFileHandler(srv, "/3/config/eip-service.json", filepath.Join(opts.ApiPath, "3", "eip-service.json"))

  /* catchall redirect */
  if opts.Redirect != "" {
    srv.Handle("/", http.RedirectHandler(opts.Redirect, http.StatusMovedPermanently))
  }

  /* address setup */
  pstr := ":" + opts.Port
  if opts.Address != "" {
    pstr = opts.Address + ":" + opts.Port
  }

  /* api server */
  if opts.Tls == true {
    log.Fatal(http.ListenAndServeTLS(pstr, opts.TlsCrt, opts.TlsKey, srv))
  } else {
    log.Fatal(http.ListenAndServe(pstr, srv))
  }
}
