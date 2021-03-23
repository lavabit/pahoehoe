CERTTOOL=certtool
CAKEY=ca-key.pem
CATEMPL=ca.cfg
SRVTEMPL=server-cert.cfg
DEPLOY=certificates
TLS=tls
VPN=vpn
SIP=sip2

usage:
	@echo "Use 'make ca' for creating a new disposable ca, or 'make certs' for creating new certs for services"

clean:
	rm -f $(TLS)-key.pem $(TLS)-cert.pem $(VPN)-key.pem $(VPN)-cert.pem $(SIP)-key.pem $(SIP)-cert.pem ca-key.pem ca.crt request.pem

ca:
	$(CERTTOOL) --generate-privkey --outfile $(CAKEY)
	$(CERTTOOL) --generate-self-signed --load-privkey ca-key.pem --outfile ca.crt --template $(CATEMPL)

certs:
	$(CERTTOOL) --generate-privkey --outfile $(TLS)-key.pem --template $(SRVTEMPL)
	$(CERTTOOL) --generate-request --load-privkey $(TLS)-key.pem --outfile request.pem --template $(SRVTEMPL)
	$(CERTTOOL) --generate-certificate --load-privkey $(TLS)-key.pem \
		    --template $(SRVTEMPL) --outfile $(TLS)-cert.pem \
		    --load-ca-certificate ca.crt --load-ca-privkey $(CAKEY)
	@rm request.pem
	$(CERTTOOL) --generate-privkey --outfile $(SIP)-key.pem --template $(SRVTEMPL)
	$(CERTTOOL) --generate-request --load-privkey $(SIP)-key.pem --outfile request.pem --template $(SRVTEMPL)
	$(CERTTOOL) --generate-certificate --load-privkey $(SIP)-key.pem \
		    --template $(SRVTEMPL) --outfile $(SIP)-cert.pem \
		    --load-ca-certificate ca.crt --load-ca-privkey $(CAKEY)
	@rm request.pem
	$(CERTTOOL) --generate-privkey --outfile $(VPN)-key.pem --template $(SRVTEMPL)
	$(CERTTOOL) --generate-request --load-privkey $(VPN)-key.pem --outfile request.pem --template $(SRVTEMPL)
	$(CERTTOOL) --generate-certificate --load-privkey $(VPN)-key.pem \
		    --template $(SRVTEMPL) --outfile $(VPN)-cert.pem \
		    --load-ca-certificate ca.crt --load-ca-privkey $(CAKEY)
	@rm request.pem
deploy:
	@rm -rf $(DEPLOY)
	@mkdir $(DEPLOY)
	cp ca.crt $(TLS)-key.pem $(TLS)-cert.pem $(VPN)-cert.pem $(VPN)-key.pem $(SIP)-cert.pem $(SIP)-key.pem $(DEPLOY)
	@echo "done"
