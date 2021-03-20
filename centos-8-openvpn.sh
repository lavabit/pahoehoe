#!/bin/bash -e

dnf -q -y install checkpolicy openvpn gnutls-utils dnsmasq

cat <<-EOF > /etc/openvpn/vpn-cert.cfg
organization = "Lavabit LLC"
unit = "Lavabit Developer Proxy"
state = "Texas"
country = US
email = "support@centos.local"
cn = "vpn.centos.local"
dc = "centos.local"
serial = 003
activation_date = "2021-01-01 12:00:00"
expiration_date = "2031-12-31 12:00:00"
dns_name = "vpn.centos.local"
dns_name = "242.vpn.centos.local"
dns_name = "243.vpn.centos.local"
dns_name = "244.vpn.centos.local"
dns_name = "245.vpn.centos.local"
signing_key
encryption_key
tls_www_server
EOF

# VPN
certtool --sec-param=high --bits=4096 --generate-privkey --outfile /etc/openvpn/vpn-key.pem --template /etc/openvpn/vpn-cert.cfg
certtool --generate-request --load-privkey /etc/openvpn/vpn-key.pem --outfile /etc/openvpn/vpn-request.pem --template /etc/openvpn/vpn-cert.cfg
certtool --generate-certificate --load-request=/etc/openvpn/vpn-request.pem --load-privkey /etc/openvpn/vpn-key.pem --outfile /etc/openvpn/vpn-cert.pem \
  --load-ca-certificate /etc/vpnweb/ca-cert.pem --load-ca-privkey /etc/vpnweb/ca-key.pem --template /etc/openvpn/vpn-cert.cfg

# DH Params
# Pre-generated DH values for use on test servers. Uncomment
# the openssl command below to generate a unique set of DH parameters
# if you;re setting up a production server.
cat <<-EOF > /etc/openvpn/dh.pem
-----BEGIN DH PARAMETERS-----
MIICCAKCAgEAmqnEVulBNsQLMyErDSnHxoofwcSu9shFBbfhtFEE7LUFw+PTdS0G
8FsqBXHbw61F7ZydR6Kf6ComVCUydF0ipeYmV7faOl6EnAug0+sVSHRCo+OLOUMq
wng1RPO9U25juBwbtR6rnxxiiuXJ4atILLifgMI0wrfdrYnuO5ibW/roAIaHUZci
cQjmXrlwrQyvlN0R/aY5SGOWUB7PhWV1hNNVDDQbYpdWti6AJPfOBWgWHm1gwP39
F/6c0KLnFij6Ut4sGU1gnuiSpY8+Kcf4xwk+VRc6AgCR1XRWoqS1MQJfbm9l9ZPx
oZAXsG1lxjlMWyFKa2cp8iAgQCL9P8UTeHIJWKVw9Wx4CjDmRdVfgW+DCz24hSUA
eGRNWZy5oP1poGr684UmfnEFlWe2zf8Y/PrJ3hXyr4D+b5h+gbPUE8s7PqE1G3Ev
6PISobokoqgQVMchds5d8d/mprq+avDEanyDfSS6I8HEh1IDI8ievBYsaC1Gf1Qs
H8WAWaLk0/2zidHv8hAhbpYkliULVHtMOCRpoewSE9+D8pTP3ZiPVEO4244ap5JV
qDL2M1mnW0cfRIyrZwyVB/S15hWzJCilUYz4jg2pDNjESs2JU9Vu13eQPQThaNc/
5Hxd4DGdtZWd30ay3JWLdnHuoBNWrOYex9Di+KnIMYqiUpK34XvbxLMCAQU=
-----END DH PARAMETERS-----
EOF

# Uncomment this for production deployments to generate new/unique Diffie-Hellman parameters.
# openssl dhparam -5 -outform PEM -out /etc/openvpn/dh.pem 4096

# Create log directory.
mkdir /var/log/openvpn/

cat <<-EOF > /etc/openvpn/server/tcp.242.conf
port 443
proto tcp
explicit-exit-notify 0
tls-server
mode server
user openvpn
group openvpn
ca /etc/vpnweb/ca-cert.pem
cert /etc/openvpn/vpn-cert.pem
key /etc/openvpn/vpn-key.pem
dh /etc/openvpn/dh.pem
auth SHA256
cipher AES-256-GCM
ncp-ciphers AES-256-GCM:AES-256-CBC
tls-cipher TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
tls-version-min 1.2
dev tun
duplicate-cn
keepalive 10 30
local 192.168.221.242
mute-replay-warnings
# push "dhcp-option DNS 10.242.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.242.0.0 255.255.248.0
tcp-nodelay
topology subnet
max-clients 2048
max-routes-per-client 2048
tcp-queue-limit 8192
txqueuelen 4096
bcast-buffers 8192
rcvbuf 2097152
sndbuf 2097152
verb 1
# log-append /var/log/openvpn/server.log
# push "block-outside-dns"
verify-x509-name "CN=BAZINGA"
EOF

cat <<-EOF > /etc/openvpn/server/udp.242.conf
port 53
proto udp
explicit-exit-notify 1
tls-server
mode server
user openvpn
group openvpn
ca /etc/vpnweb/ca-cert.pem
cert /etc/openvpn/vpn-cert.pem
key /etc/openvpn/vpn-key.pem
dh /etc/openvpn/dh.pem
auth SHA256
cipher AES-256-GCM
ncp-ciphers AES-256-GCM:AES-256-CBC
tls-cipher TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
tls-version-min 1.2
dev tun
duplicate-cn
keepalive 10 30
local 192.168.221.242
mute-replay-warnings
# push "dhcp-option DNS 10.142.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.142.0.0 255.255.248.0
topology subnet
max-clients 2048
max-routes-per-client 2048
txqueuelen 4096
bcast-buffers 8192
rcvbuf 2097152
sndbuf 2097152
verb 1
# log-append /var/log/openvpn/server.log
# push "block-outside-dns"
verify-x509-name "CN=BAZINGA"
EOF

cat <<-EOF > /etc/openvpn/server/tcp.243.conf
port 443
proto tcp
explicit-exit-notify 0
tls-server
mode server
user openvpn
group openvpn
ca /etc/vpnweb/ca-cert.pem
cert /etc/openvpn/vpn-cert.pem
key /etc/openvpn/vpn-key.pem
dh /etc/openvpn/dh.pem
auth SHA256
cipher AES-256-GCM
ncp-ciphers AES-256-GCM:AES-256-CBC
tls-cipher TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
tls-version-min 1.2
dev tun
duplicate-cn
keepalive 10 30
local 192.168.221.243
mute-replay-warnings
# push "dhcp-option DNS 10.243.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.243.0.0 255.255.248.0
tcp-nodelay
topology subnet
max-clients 2048
max-routes-per-client 2048
tcp-queue-limit 8192
txqueuelen 4096
bcast-buffers 8192
rcvbuf 2097152
sndbuf 2097152
verb 1
# log-append /var/log/openvpn/server.log
# push "block-outside-dns"
verify-x509-name "CN=BAZINGA"
EOF

cat <<-EOF > /etc/openvpn/server/udp.243.conf
port 53
proto udp
explicit-exit-notify 1
tls-server
mode server
user openvpn
group openvpn
ca /etc/vpnweb/ca-cert.pem
cert /etc/openvpn/vpn-cert.pem
key /etc/openvpn/vpn-key.pem
dh /etc/openvpn/dh.pem
auth SHA256
cipher AES-256-GCM
ncp-ciphers AES-256-GCM:AES-256-CBC
tls-cipher TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
tls-version-min 1.2
dev tun
duplicate-cn
keepalive 10 30
local 192.168.221.243
mute-replay-warnings
# push "dhcp-option DNS 10.143.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.143.0.0 255.255.248.0
topology subnet
max-clients 2048
max-routes-per-client 2048
txqueuelen 4096
bcast-buffers 8192
rcvbuf 2097152
sndbuf 2097152
verb 1
# log-append /var/log/openvpn/server.log
# push "block-outside-dns"
verify-x509-name "CN=BAZINGA"
EOF

cat <<-EOF > /etc/openvpn/server/tcp.244.conf
port 443
proto tcp
explicit-exit-notify 0
tls-server
mode server
user openvpn
group openvpn
ca /etc/vpnweb/ca-cert.pem
cert /etc/openvpn/vpn-cert.pem
key /etc/openvpn/vpn-key.pem
dh /etc/openvpn/dh.pem
auth SHA256
cipher AES-256-GCM
ncp-ciphers AES-256-GCM:AES-256-CBC
tls-cipher TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
tls-version-min 1.2
dev tun
duplicate-cn
keepalive 10 30
local 192.168.221.244
mute-replay-warnings
# push "dhcp-option DNS 10.244.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.244.0.0 255.255.248.0
tcp-nodelay
topology subnet
max-clients 2048
max-routes-per-client 2048
tcp-queue-limit 8192
txqueuelen 4096
bcast-buffers 8192
rcvbuf 2097152
sndbuf 2097152
verb 1
# log-append /var/log/openvpn/server.log
# push "block-outside-dns"
verify-x509-name "CN=BAZINGA"
EOF

cat <<-EOF > /etc/openvpn/server/udp.244.conf
port 53
proto udp
explicit-exit-notify 1
tls-server
mode server
user openvpn
group openvpn
ca /etc/vpnweb/ca-cert.pem
cert /etc/openvpn/vpn-cert.pem
key /etc/openvpn/vpn-key.pem
dh /etc/openvpn/dh.pem
auth SHA256
cipher AES-256-GCM
ncp-ciphers AES-256-GCM:AES-256-CBC
tls-cipher TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
tls-version-min 1.2
dev tun
duplicate-cn
keepalive 10 30
local 192.168.221.244
mute-replay-warnings
# push "dhcp-option DNS 10.144.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.144.0.0 255.255.248.0
topology subnet
max-clients 2048
max-routes-per-client 2048
txqueuelen 4096
bcast-buffers 8192
rcvbuf 2097152
sndbuf 2097152
verb 1
# log-append /var/log/openvpn/server.log
# push "block-outside-dns"
verify-x509-name "CN=BAZINGA"
EOF

cat <<-EOF > /etc/openvpn/server/tcp.245.conf
port 443
proto tcp
explicit-exit-notify 0
tls-server
mode server
user openvpn
group openvpn
ca /etc/vpnweb/ca-cert.pem
cert /etc/openvpn/vpn-cert.pem
key /etc/openvpn/vpn-key.pem
dh /etc/openvpn/dh.pem
auth SHA256
cipher AES-256-GCM
ncp-ciphers AES-256-GCM:AES-256-CBC
tls-cipher TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
tls-version-min 1.2
dev tun
duplicate-cn
keepalive 10 30
local 192.168.221.245
mute-replay-warnings
# push "dhcp-option DNS 10.245.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.245.0.0 255.255.248.0
tcp-nodelay
topology subnet
max-clients 2048
max-routes-per-client 2048
tcp-queue-limit 8192
txqueuelen 4096
bcast-buffers 8192
rcvbuf 2097152
sndbuf 2097152
verb 1
# log-append /var/log/openvpn/server.log
# push "block-outside-dns"
verify-x509-name "CN=BAZINGA"
EOF

cat <<-EOF > /etc/openvpn/server/udp.245.conf
port 53
proto udp
explicit-exit-notify 1
tls-server
mode server
user openvpn
group openvpn
ca /etc/vpnweb/ca-cert.pem
cert /etc/openvpn/vpn-cert.pem
key /etc/openvpn/vpn-key.pem
dh /etc/openvpn/dh.pem
auth SHA256
cipher AES-256-GCM
ncp-ciphers AES-256-GCM:AES-256-CBC
tls-cipher TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
tls-version-min 1.2
dev tun
duplicate-cn
keepalive 10 30
local 192.168.221.245
mute-replay-warnings
# push "dhcp-option DNS 10.145.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.145.0.0 255.255.248.0
topology subnet
max-clients 2048
max-routes-per-client 2048
txqueuelen 4096
bcast-buffers 8192
rcvbuf 2097152
sndbuf 2097152
verb 1
# log-append /var/log/openvpn/server.log
# push "block-outside-dns"
verify-x509-name "CN=BAZINGA"
EOF

# Enable packet forwarding support.
cat <<-EOF > /etc/sysctl.d/10-forwarding.conf

net.ipv4.ip_forward = 1
net.ipv6.conf.all.forwarding = 1
net.ipv6.conf.all.disable_ipv6 = 0

EOF

sed -i '/all.disable_ipv6/d' /etc/sysctl.conf && sysctl net.ipv6.conf.all.disable_ipv6=0
sysctl -p < /etc/sysctl.d/10-forwarding.conf

# Increase the system limits so TOR can make better use the hardware.
cat <<-EOF > /etc/security/limits.d/50-global.conf
*      soft    memlock    16467224
*      hard    memlock    16467224
*      soft    nproc      65536
*      hard    nproc      65536
*      soft    nofile     1048576
*      hard    nofile     1048576
*      soft    stack      unlimited
*      hard    stack      unlimited
EOF

chmod 644 /etc/security/limits.d/50-global.conf
chcon "unconfined_u:object_r:etc_t:s0" /etc/security/limits.d/50-global.conf

# Override the file descriptor limit for the systemd service description.
[ ! -d  /etc/systemd/system/openvpn-server@.service.d/ ] && mkdir --parents /etc/systemd/system/openvpn-server@.service.d/
cat <<-EOF > /etc/systemd/system/openvpn-server@.service.d/override.conf
[Service]
LimitNOFILE=65535
EOF

cat <<-EOF > $HOME/my-openvpn.te

module my-openvpn 1.0;

require {
        type openvpn_t;
        type dns_port_t;
        class udp_socket name_bind;
}

#============= openvpn_t ==============

allow openvpn_t dns_port_t:udp_socket name_bind;
EOF

checkmodule -M -m -o $HOME/my-openvpn.mod $HOME/my-openvpn.te
semodule_package -o $HOME/my-openvpn.pp -m $HOME/my-openvpn.mod
semodule -X 300 -i $HOME/my-openvpn.pp
rm --force $HOME/my-openvpn.mod $HOME/my-openvpn.pp $HOME/my-openvpn.te

systemctl daemon-reload
systemctl enable openvpn-server@tcp.242.service && systemctl start openvpn-server@tcp.242.service
systemctl enable openvpn-server@tcp.243.service && systemctl start openvpn-server@tcp.243.service
systemctl enable openvpn-server@tcp.244.service && systemctl start openvpn-server@tcp.244.service
systemctl enable openvpn-server@tcp.245.service && systemctl start openvpn-server@tcp.245.service
systemctl enable openvpn-server@udp.242.service && systemctl start openvpn-server@udp.242.service
systemctl enable openvpn-server@udp.243.service && systemctl start openvpn-server@udp.243.service
systemctl enable openvpn-server@udp.244.service && systemctl start openvpn-server@udp.244.service
systemctl enable openvpn-server@udp.245.service && systemctl start openvpn-server@udp.245.service

cat <<-EOF > /etc/dnsmasq.d/interfaces.conf
interface=tun0
interface=tun1
interface=tun2
interface=tun3
interface=tun4
interface=tun5
interface=tun6
interface=tun7

bind-interfaces
EOF

systemctl enable dnsmasq.service && systemctl start dnsmasq.service

# Firewall rules.
DEVICE=$(ip route | awk '/^default via/ {print $5}')
firewall-cmd --add-port=53/udp && firewall-cmd --add-port=53/udp --permanent
firewall-cmd --add-port=443/tcp && firewall-cmd --add-port=443/tcp --permanent

firewall-cmd --zone=trusted --add-interface=tun0 && firewall-cmd --permanent --zone=trusted --add-interface=tun0
firewall-cmd --zone=trusted --add-interface=tun1 && firewall-cmd --permanent --zone=trusted --add-interface=tun1
firewall-cmd --zone=trusted --add-interface=tun2 && firewall-cmd --permanent --zone=trusted --add-interface=tun2
firewall-cmd --zone=trusted --add-interface=tun3 && firewall-cmd --permanent --zone=trusted --add-interface=tun3
firewall-cmd --zone=trusted --add-interface=tun4 && firewall-cmd --permanent --zone=trusted --add-interface=tun4
firewall-cmd --zone=trusted --add-interface=tun5 && firewall-cmd --permanent --zone=trusted --add-interface=tun5
firewall-cmd --zone=trusted --add-interface=tun6 && firewall-cmd --permanent --zone=trusted --add-interface=tun6
firewall-cmd --zone=trusted --add-interface=tun7 && firewall-cmd --permanent --zone=trusted --add-interface=tun7

firewall-cmd --add-masquerade && firewall-cmd --add-masquerade --permanent
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.142.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.143.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.144.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.145.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.242.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.243.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.244.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.245.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.142.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.143.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.144.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.145.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.242.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.243.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.244.0.0/21 -o $DEVICE -j MASQUERADE
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.245.0.0/21 -o $DEVICE -j MASQUERADE

dnf -q -y remove checkpolicy gnutls-utils 
