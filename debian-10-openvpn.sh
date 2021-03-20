#!/bin/bash

env DEBCONF_NONINTERACTIVE_SEEN=true DEBIAN_FRONTEND=noninteractive apt-get -qq -y install openvpn gnutls-bin iptables-persistent < /dev/null > /dev/null

cat <<-EOF > /etc/openvpn/vpn-cert.cfg
organization = "Lavabit LLC"
unit = "Lavabit Developer Proxy"
state = "Texas"
country = US
email = "support@debian.local"
cn = "vpn.debian.local"
dc = "debian.local"
serial = 003
activation_date = "2021-01-01 12:00:00"
expiration_date = "2031-12-31 12:00:00"
dns_name = "vpn.debian.local"
dns_name = "142.vpn.debian.local"
dns_name = "143.vpn.debian.local"
dns_name = "144.vpn.debian.local"
dns_name = "145.vpn.debian.local"
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

cat <<-EOF > /etc/openvpn/server/tcp.142.conf
port 443
proto tcp
explicit-exit-notify 0
tls-server
mode server
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
local 192.168.221.142
mute-replay-warnings
# push "dhcp-option DNS 10.82.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.82.0.0 255.255.248.0
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

cat <<-EOF > /etc/openvpn/server/udp.142.conf
port 53
proto udp
explicit-exit-notify 1
tls-server
mode server
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
local 192.168.221.142
mute-replay-warnings
# push "dhcp-option DNS 10.42.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.42.0.0 255.255.248.0
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

cat <<-EOF > /etc/openvpn/server/tcp.143.conf
port 443
proto tcp
explicit-exit-notify 0
tls-server
mode server
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
local 192.168.221.143
mute-replay-warnings
# push "dhcp-option DNS 10.83.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.83.0.0 255.255.248.0
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

cat <<-EOF > /etc/openvpn/server/udp.143.conf
port 53
proto udp
explicit-exit-notify 1
tls-server
mode server
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
local 192.168.221.143
mute-replay-warnings
# push "dhcp-option DNS 10.43.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.43.0.0 255.255.248.0
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

cat <<-EOF > /etc/openvpn/server/tcp.144.conf
port 443
proto tcp
explicit-exit-notify 0
tls-server
mode server
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
local 192.168.221.144
mute-replay-warnings
# push "dhcp-option DNS 10.84.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.84.0.0 255.255.248.0
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

cat <<-EOF > /etc/openvpn/server/udp.144.conf
port 53
proto udp
explicit-exit-notify 1
tls-server
mode server
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
local 192.168.221.144
mute-replay-warnings
# push "dhcp-option DNS 10.44.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.44.0.0 255.255.248.0
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

cat <<-EOF > /etc/openvpn/server/tcp.145.conf
port 443
proto tcp
explicit-exit-notify 0
tls-server
mode server
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
local 192.168.221.145
mute-replay-warnings
# push "dhcp-option DNS 10.85.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.85.0.0 255.255.248.0
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

cat <<-EOF > /etc/openvpn/server/udp.145.conf
port 53
proto udp
explicit-exit-notify 1
tls-server
mode server
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
local 192.168.221.145
mute-replay-warnings
# push "dhcp-option DNS 10.45.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 2001:db8:123::/64
server 10.45.0.0 255.255.248.0
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

# reneg-sec 86400
# tran-window 86400

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

# Override the file descriptor limit for the systemd service description.
[ ! -d  /etc/systemd/system/openvpn-server@.service.d/ ] && mkdir --parents /etc/systemd/system/openvpn-server@.service.d/
cat <<-EOF > /etc/systemd/system/openvpn-server@.service.d/override.conf
[Service]
LimitNOFILE=65535
EOF

systemctl daemon-reload
systemctl enable openvpn-server@tcp.142.service && systemctl start openvpn-server@tcp.142.service
systemctl enable openvpn-server@tcp.143.service && systemctl start openvpn-server@tcp.143.service
systemctl enable openvpn-server@tcp.144.service && systemctl start openvpn-server@tcp.144.service
systemctl enable openvpn-server@tcp.145.service && systemctl start openvpn-server@tcp.145.service
systemctl enable openvpn-server@udp.142.service && systemctl start openvpn-server@udp.142.service
systemctl enable openvpn-server@udp.143.service && systemctl start openvpn-server@udp.143.service
systemctl enable openvpn-server@udp.144.service && systemctl start openvpn-server@udp.144.service
systemctl enable openvpn-server@udp.145.service && systemctl start openvpn-server@udp.145.service


# env DEBCONF_NONINTERACTIVE_SEEN=true DEBIAN_FRONTEND=noninteractive apt-get -qq -y install dnsmasq < /dev/null > /dev/null

# cat <<-EOF > /etc/dnsmasq.d/interfaces.conf
# interface=tun0
# interface=tun1
# interface=tun2
# interface=tun3
# interface=tun4
# interface=tun5
# interface=tun6
# interface=tun7
# 
# bind-interfaces
# EOF

# systemctl enable dnsmasq.service && systemctl start dnsmasq.service

iptables --append FORWARD -i tun0 -j ACCEPT
iptables --append FORWARD -i tun1 -j ACCEPT
iptables --append FORWARD -i tun2 -j ACCEPT
iptables --append FORWARD -i tun3 -j ACCEPT
iptables --append FORWARD -i tun4 -j ACCEPT
iptables --append FORWARD -i tun5 -j ACCEPT
iptables --append FORWARD -i tun6 -j ACCEPT
iptables --append FORWARD -i tun7 -j ACCEPT

# Open up port 443 for TCP and UDP connections.
iptables --append INPUT --proto icmp --jump ACCEPT
iptables --append INPUT --in-interface lo --jump ACCEPT
iptables --append INPUT --match state --state RELATED,ESTABLISHED --jump ACCEPT
iptables --append INPUT --proto tcp --match state --state NEW -m tcp --dport 22 --jump ACCEPT
iptables --append INPUT --proto udp --match state --state NEW -m udp --dport 53 --jump ACCEPT
iptables --append INPUT --proto tcp --match state --state NEW -m tcp --dport 443 --jump ACCEPT
iptables --append INPUT -j REJECT --reject-with icmp-host-prohibited

ip6tables --append INPUT --in-interface lo --jump ACCEPT
ip6tables --append INPUT --proto ipv6-icmp --jump ACCEPT
ip6tables --append INPUT --match state --state RELATED,ESTABLISHED --jump ACCEPT
ip6tables --append INPUT --proto tcp --match state --state NEW -m tcp --dport 22 --jump ACCEPT
ip6tables --append INPUT -j REJECT --reject-with icmp6-adm-prohibited

# Add iptables-rule to forward packets from our OpenVPN subnets.
iptables --table nat --append POSTROUTING -s 10.42.0.0/24 -o eth0 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.42.0.0/24 -o eth1 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.43.0.0/24 -o eth0 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.43.0.0/24 -o eth1 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.44.0.0/24 -o eth0 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.44.0.0/24 -o eth1 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.45.0.0/24 -o eth0 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.45.0.0/24 -o eth1 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.82.0.0/24 -o eth0 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.82.0.0/24 -o eth1 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.83.0.0/24 -o eth0 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.83.0.0/24 -o eth1 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.84.0.0/24 -o eth0 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.84.0.0/24 -o eth1 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.85.0.0/24 -o eth0 -j MASQUERADE
iptables --table nat --append POSTROUTING -s 10.85.0.0/24 -o eth1 -j MASQUERADE

iptables-save > /etc/iptables/rules.v4
ip6tables-save > /etc/iptables/rules.v6

# firewall-cmd --add-port=443/tcp && firewall-cmd --add-port=443/tcp --permanent
# firewall-cmd --add-port=443/udp && firewall-cmd --add-port=443/udp --permanent

# Discard swap pages to free space.
sudo swapoff --all
sudo sed -i "s/swap    sw/swap    pri=1,discard,sw/g" /etc/fstab
sudo swapon --all

# Trim the drive to free space.
sudo sed -i "s/OnCalendar.*/OnCalendar=hourly/g" /lib/systemd/system/fstrim.timer
sudo sed -i "s/AccuracySec.*/AccuracySec=5m/g" /lib/systemd/system/fstrim.timer
sudo systemctl daemon-reload && sudo systemctl enable fstrim.timer
sudo fstrim --all


