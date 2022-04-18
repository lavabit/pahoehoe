#!/bin/bash -e

export DEBCONF_NONINTERACTIVE_SEEN=true 
export DEBIAN_FRONTEND=noninteractive

apt-get -qq -y install golang gnutls-bin git gnupg nload net-tools toxiproxy-cli expect build-essential pkg-config golang-go ca-certificates openvpn python3-pysodium python3-jinja2 python3-netaddr python3-openssl python3-yaml python3-six python3-crypto jq daemon < /dev/null > /dev/null

sed -i "s/1024/3072 -b 1024 -d 1024 -i 1024/g" /etc/default/haveged
sed -i "s/ENABLED=.*/ENABLED=\"true\"/g" /etc/default/sysstat
sudo systemctl --quiet restart haveged && sudo systemctl --quiet restart sysstat

# Point us at the development environment.
sudo tee --append /etc/hosts <<-EOF > /dev/null
192.168.221.146 api.debian.local
192.168.221.142 vpn.debian.local
192.168.221.142 142.vpn.debian.local
192.168.221.143 143.vpn.debian.local
192.168.221.144 144.vpn.debian.local
192.168.221.145 145.vpn.debian.local
EOF

sed -i "/HISTCONTROL/d" $HOME/.bashrc
sed -i "/HISTFILESIZE/d" $HOME/.bashrc
sed -i "s/HISTSIZE=.*/export HISTSIZE=100000/g" $HOME/.bashrc

[ -d $HOME/vpnweb ] && rm --force --recursive $HOME/vpnweb
cd $HOME && git clone --quiet https://github.com/lavabit/pahoehoe.git $HOME/vpnweb && cd $HOME/vpnweb && FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch --subdirectory-filter vpnweb

[ -d /etc/vpnweb/ ] && rm --force --recursive /etc/vpnweb/
mkdir --parents /etc/vpnweb/public/
mkdir --parents /etc/vpnweb/public/1/
mkdir --parents /etc/vpnweb/public/3/

cat <<-EOF > /etc/default/vpnweb
VPNWEB_AUTH=anon
VPNWEB_PORT=443
VPNWEB_ADDRESS=192.168.221.146
VPNWEB_REDIRECT=https://lavabit.com/
VPNWEB_AUTH_SECRET="`expect_mkpasswd -l 32 -d 6 -C 6 -s 0 -d 4`"
VPNWEB_API_PATH=/etc/vpnweb/public/
VPNWEB_PROVIDER_CA=/etc/vpnweb/ca-cert.pem
VPNWEB_TLSCRT=/etc/vpnweb/tls-cert.pem
VPNWEB_TLSKEY=/etc/vpnweb/tls-key.pem
VPNWEB_CACRT=/etc/vpnweb/ca-cert.pem
VPNWEB_CAKEY=/etc/vpnweb/ca-key.pem
EOF

cat <<-EOF > /etc/vpnweb/ca.cfg
organization = "Lavabit LLC"
unit = "Lavabit Proxy Service"
state = "Texas"
country = US
email = "support@debian.local"
cn = "debian.local"
dc = "debian.local"
serial = 001
activation_date = "2021-01-01 10:00:00"
expiration_date = "2031-12-31 23:59:59"
dns_name = "debian.local"
ca
signing_key
encryption_key
cert_signing_key
EOF

cat <<-EOF > /etc/vpnweb/tls-cert.cfg
organization = "Lavabit LLC"
unit = "Lavabit Proxy Service"
state = "Texas"
country = US
email = "support@debian.local"
cn = "api.debian.local"
dc = "debian.local"
serial = 002
activation_date = "2021-01-01 12:00:00"
expiration_date = "2031-12-31 12:00:00"
dns_name = "api.debian.local"
signing_key
encryption_key
tls_www_server
EOF

# CA
certtool --stdout-info --sec-param=ultra --generate-privkey --outfile /etc/vpnweb/ca-key.pem 1>/dev/null
certtool --stdout-info --sec-param=ultra --generate-self-signed --load-privkey /etc/vpnweb/ca-key.pem --outfile /etc/vpnweb/ca-cert.pem --template /etc/vpnweb/ca.cfg 1>/dev/null

# TLS
certtool --stdout-info --sec-param=ultra --generate-privkey --outfile /etc/vpnweb/tls-key.pem --template /etc/vpnweb/tls-cert.cfg 1>/dev/null
certtool --stdout-info --sec-param=ultra --generate-request --load-privkey /etc/vpnweb/tls-key.pem --outfile /etc/vpnweb/tls-request.pem --template /etc/vpnweb/tls-cert.cfg 1>/dev/null
certtool --stdout-info --sec-param=ultra --generate-certificate --load-request=/etc/vpnweb/tls-request.pem --load-privkey /etc/vpnweb/tls-key.pem --outfile /etc/vpnweb/tls-cert.pem \
  --load-ca-certificate /etc/vpnweb/ca-cert.pem --load-ca-privkey /etc/vpnweb/ca-key.pem --template /etc/vpnweb/tls-cert.cfg 1>/dev/null

rm --force /etc/vpnweb/ca.cfg
rm --force /etc/vpnweb/tls-cert.cfg
rm --force /etc/vpnweb/tls-request.pem

export FINGERPRINT="`certtool --stdout-info --certificate-info --infile /etc/vpnweb/ca-cert.pem | grep 'sha256:' | head -1 | tr -d '[:space:]' | awk -F':' '{print $2}'`"

cat <<-EOF > /etc/vpnweb/provider.yaml
auth: anon

openvpn:
    - auth: SHA256
    - cipher: AES-256-GCM
    - keepalive: "10 30"
    - tls-cipher: TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
    - tun-ipv6: true
    - dev: tun
    - sndbuf: 0
    - rcvbuf: 0
    - nobind: true
    - persist-key: true
    - key-direction: 1

locations:
    - Dallas:
        - name: Dallas
        - country_code: US
        - hemisphere: N
        - timezone: -6

gateways:
    - 142.vpn.debian.local:
        - host: 142.vpn.debian.local
        - ip_address: 192.168.221.142
        - location: Dallas
        - transports:
            - [ "openvpn", "tcp", "443"]

provider:
    - name: "Debian Test Proxy"
    - description: "Debian Test Proxy"
    - domain: debian.local
    - api_uri: api.debian.local
    - fingerprint: "SHA256: $FINGERPRINT"
EOF

cat <<-EOF > /etc/vpnweb/public/geoip.json
{
  "ip": "192.168.221.146",
  "cc": "US",
  "city": "Dallas",
  "lat": 32.776,
  "lon": -97.822,
  "gateways": [
    "242.vpn.debian.local",
    "243.vpn.debian.local",
    "244.vpn.debian.local",
    "245.vpn.debian.local"
  ]
}
EOF

cat <<-EOF > /etc/vpnweb/public/1/service.json
{
  "name": "anonymous",
  "description": "anonymous access to the VPN",
  "eip_rate_limit": false
}
EOF

cat <<-EOF > /etc/vpnweb/public/1/configs.json
{
  "services": {
    "eip": "/1/configs/eip-service.json"
  }
}
EOF

cat <<-EOF > /etc/vpnweb/public/3/service.json
{
  "name": "anonymous",
  "description": "anonymous access to the VPN",
  "eip_rate_limit": false
}
EOF

cat <<-EOF > /etc/vpnweb/public/3/configs.json
{
  "services": {
    "eip": "/3/configs/eip-service.json"
  }
}
EOF

GOPATH=$HOME/vpnweb/.go/ go build -a

# $HOME/vpnweb/scripts/gen-shapeshifter-state.py /etc/vpnweb/shapeshifter-state/
# python3 $HOME/vpnweb/scripts/simplevpn.py --file=eip --config=/etc/vpnweb/provider.yaml --template=$HOME/vpnweb/scripts/templates/eip-service.1.json.jinja --obfs4_state=/etc/vpnweb/shapeshifter-state/ | jq -M '.' > /etc/vpnweb/public/1/eip-service.json || echo "ERROR: see /etc/vpnweb/public/1/eip-service.json for output"
# python3 $HOME/vpnweb/scripts/simplevpn.py --file=eip --config=/etc/vpnweb/provider.yaml --template=$HOME/vpnweb/scripts/templates/eip-service.3.json.jinja --obfs4_state=/etc/vpnweb/shapeshifter-state/ | jq -M '.' > /etc/vpnweb/public/3/eip-service.json || echo "ERROR: see /etc/vpnweb/public/3/eip-service.json for output"
python3 $HOME/vpnweb/scripts/simplevpn.py --file=eip --config=/etc/vpnweb/provider.yaml --template=$HOME/vpnweb/scripts/templates/eip-service.1.json.jinja | jq -M '.' > /etc/vpnweb/public/1/eip-service.json || echo "ERROR: see /etc/vpnweb/public/1/eip-service.json for output"
python3 $HOME/vpnweb/scripts/simplevpn.py --file=eip --config=/etc/vpnweb/provider.yaml --template=$HOME/vpnweb/scripts/templates/eip-service.3.json.jinja | jq -M '.' > /etc/vpnweb/public/3/eip-service.json || echo "ERROR: see /etc/vpnweb/public/3/eip-service.json for output"
python3 $HOME/vpnweb/scripts/simplevpn.py --file=provider --config=/etc/vpnweb/provider.yaml --template=$HOME/vpnweb/scripts/templates/provider.json.jinja | jq -M '.' > /etc/vpnweb/public/provider.json || echo "ERROR: see /etc/vpnweb/provider.json for output"

[ -f /run/vpnweb.pid ] && /usr/bin/pkill --pidfile /run/vpnweb.pid
install -m 755 $HOME/vpnweb/vpnweb /usr/local/bin/

cat <<-EOF > /usr/lib/systemd/system/vpnweb.service
[Unit]
Description=vpnweb
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
EnvironmentFile=/etc/default/vpnweb
ExecStart=/usr/bin/daemon --inherit --pidfile=/run/vpnweb.pid -- /usr/local/bin/vpnweb -tls=true
ExecStop=bash -c '/usr/bin/test -f /run/vpnweb.pid && /usr/bin/pkill --pidfile /run/vpnweb.pid'
ExecReload=bash -c '/usr/bin/test -f /run/vpnweb.pid && /usr/bin/pkill --signal HUP --pidfile /run/vpnweb.pid'
LimitNOFILE=65535

[Install]
WantedBy=multi-user.target
EOF

systemctl --quiet daemon-reload
systemctl --quiet enable vpnweb.service && systemctl --quiet start vpnweb.service

echo "VPN web service installed."
echo "Starting unit tests."
sleep 10

curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/ca.crt || { echo "https://api.debian.local/ca.crt failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/provider.json || { echo "https://api.debian.local/provider.json failed ..." ; exit 1 ; }

curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/1/cert || { echo "https://api.debian.local/1/cert failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/1/ca.crt || { echo "https://api.debian.local/1/ca.crt failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/1/configs.json || { echo "https://api.debian.local/1/configs.json failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/1/service.json || { echo "https://api.debian.local/1/service.json failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/1/config/eip-service.json || { echo "https://api.debian.local/1/config/eip-service.json failed ..." ; exit 1 ; }

curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/3/cert || { echo "https://api.debian.local/3/cert failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/3/ca.crt || { echo "https://api.debian.local/3/ca.crt failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/3/configs.json || { echo "https://api.debian.local/3/configs.json failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/3/service.json || { echo "https://api.debian.local/3/service.json failed ..." ; exit 1 ; }
curl --fail --silent --show-error --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/3/config/eip-service.json || { echo "https://api.debian.local/3/config/eip-service.json failed ..." ; exit 1 ; }

curl --fail --silent --show-error --insecure --location --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/ || { echo "https://api.debian.local/ redirection failed ..." ; exit 1 ; }
curl --fail --silent --show-error --insecure --location --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem https://api.debian.local/$RANDOM || { echo "https://api.debian.local/RANDOM redirection failed ..." ; exit 1 ; }

for i in {1..100}; do
  URL="`cat /dev/urandom | tr -dc '[:alnum:]\#\\\/\.\-\_' | fold -w 32 | head -n 1`"
  curl --fail --silent --show-error --globoff --insecure --location --retry 10 --retry-delay 10 --max-time 300 --connect-timeout 300 --output /dev/null --cacert /etc/vpnweb/ca-cert.pem "https://api.debian.local/${URL}" || { echo "https://api.debian.local/${URL} redirection failed" ; exit 1 ; }
done

echo "Unit tests completed."

# Cleanup.
rm --force --recursive $HOME/go/ $HOME/.cache/ $HOME/vpnweb/
