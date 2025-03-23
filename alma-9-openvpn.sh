#!/bin/bash -e

dnf -q -y install checkpolicy policycoreutils-python-utils openvpn gnutls-utils dnsmasq 1>/dev/null

cat <<-EOF > /etc/openvpn/vpn-cert.cfg
organization = "Lavabit LLC"
unit = "Lavabit Developer Proxy"
state = "Texas"
country = US
email = "support@alma.local"
cn = "vpn.alma.local"
dc = "alma.local"
serial = 003
activation_date = "2021-01-01 12:00:00"
expiration_date = "2031-12-31 12:00:00"
dns_name = "vpn.alma.local"
signing_key
encryption_key
tls_www_server
EOF

# VPN
certtool --stdout-info --sec-param=high --bits=4096 --generate-privkey --outfile /etc/openvpn/vpn-key.pem --template /etc/openvpn/vpn-cert.cfg 1>/dev/null
certtool --stdout-info --generate-request --load-privkey /etc/openvpn/vpn-key.pem --outfile /etc/openvpn/vpn-request.pem --template /etc/openvpn/vpn-cert.cfg 1>/dev/null
certtool --stdout-info --generate-certificate --load-request=/etc/openvpn/vpn-request.pem --load-privkey /etc/openvpn/vpn-key.pem --outfile /etc/openvpn/vpn-cert.pem \
  --load-ca-certificate /etc/vpnweb/ca-cert.pem --load-ca-privkey /etc/vpnweb/ca-key.pem --template /etc/openvpn/vpn-cert.cfg 1>/dev/null

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
[ ! -d /var/log/openvpn/ ] && mkdir /var/log/openvpn/

cat <<-EOF > /etc/openvpn/server/udp.246.conf
port 554
proto udp4
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
dev tun0
persist-tun
duplicate-cn
keepalive 10 30
local 192.168.221.246
mute-replay-warnings
# push "dhcp-option DNS 10.146.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 fd00:146:146:146::/112
server 10.146.0.0 255.255.248.0
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

cat <<-EOF > /etc/openvpn/server/tcp.246.conf
port 554
proto tcp4
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
dev tun1
persist-tun
duplicate-cn
keepalive 10 30
local 192.168.221.246
mute-replay-warnings
# push "dhcp-option DNS 10.246.0.1"
push "dhcp-option DNS 1.1.1.1"
push "dhcp-option DNS 4.2.2.1"
push "dhcp-option DNS 4.2.2.2"
push "redirect-gateway def1 bypass-dhcp"
push "route-ipv6 2000::/3"
script-security 1
server-ipv6 fd00:246:246:246::/112
server 10.246.0.0 255.255.248.0
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

# Enable packet forwarding support.
cat <<-EOF > /etc/sysctl.d/10-forwarding.conf

net.ipv4.ip_forward = 1
net.ipv6.conf.all.forwarding = 1
net.ipv6.conf.all.disable_ipv6 = 0

EOF

sed -i '/all.disable_ipv6/d' /etc/sysctl.conf && sysctl --quiet net.ipv6.conf.all.disable_ipv6=0
sysctl --quiet -p < /etc/sysctl.d/10-forwarding.conf

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

# Alternative selinux update method.
semanage port -a -t openvpn_port_t -p tcp 554
semanage port -a -t openvpn_port_t -p udp 554

systemctl --quiet daemon-reload
systemctl --quiet enable openvpn-server@udp.246.service && systemctl start openvpn-server@udp.246.service
systemctl --quiet enable openvpn-server@tcp.246.service && systemctl start openvpn-server@tcp.246.service


cat <<-EOF > /etc/dnsmasq.d/interfaces.conf
interface=tun0
interface=tun1

bind-interfaces
EOF


# Override the default dnsmasq service unit.
if [ -f /etc/systemd/system/multi-user.target.wants/dnsmasq-wait.service ]; then
  unlink /etc/systemd/system/multi-user.target.wants/dnsmasq-wait.service
fi

cat <<-EOF > /usr/lib/systemd/system/dnsmasq-wait.service
[Unit]
Description=Create a short pause after OpenVPN is started before (to allow the tun interfaces to get created) before starting dnsmasq
After=network.target
Before=dnsmasq.service

[Service]
Type=oneshot
ExecStart=bash -c 'for i in {1..30} ; do [ -d /sys/devices/virtual/net/tun0 ] && [ -d /sys/devices/virtual/net/tun1 ] && [ -f /run/NetworkManager/system-connections/tun0.nmconnection ] && [ -f /run/NetworkManager/system-connections/tun1.nmconnection ] && break ; sleep 2 ; done'
TimeoutStartSec=90
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target

EOF


[ ! -d /etc/systemd/system/dnsmasq.service.d/ ] && mkdir --parents /etc/systemd/system/dnsmasq.service.d/ 
cat <<-EOF > /etc/systemd/system/dnsmasq.service.d/override.conf
[Unit]
After=dnsmasq-wait.service

EOF

systemctl daemon-reload
systemctl --quiet enable dnsmasq-wait.service && systemctl --quiet start dnsmasq-wait.service
systemctl --quiet enable dnsmasq.service && systemctl --quiet start dnsmasq.service

# Fix the permissions on the interface config files to avoid errors below.
chown -R root:root /etc/sysconfig/network-scripts/
chmod 644 /etc/sysconfig/network-scripts/ifcfg-*
chcon "system_u:object_r:net_conf_t:s0" /etc/sysconfig/network-scripts/ifcfg-*

# Firewall rules.
DEVICE=$(ip route | awk '/^default via/ {print $5}')
firewall-cmd --add-port=53/udp 1>/dev/null && firewall-cmd --add-port=53/udp --permanent 1>/dev/null
firewall-cmd --add-port=53/tcp 1>/dev/null && firewall-cmd --add-port=53/tcp --permanent 1>/dev/null
firewall-cmd --add-port=443/tcp 1>/dev/null && firewall-cmd --add-port=443/tcp --permanent 1>/dev/null
firewall-cmd --add-port=554/udp 1>/dev/null && firewall-cmd --add-port=554/udp --permanent 1>/dev/null
firewall-cmd --add-port=554/tcp 1>/dev/null && firewall-cmd --add-port=554/tcp --permanent 1>/dev/null

firewall-cmd --zone=trusted --add-interface=tun0 1>/dev/null && firewall-cmd --permanent --zone=trusted --add-interface=tun0 1>/dev/null
firewall-cmd --zone=trusted --add-interface=tun1 1>/dev/null && firewall-cmd --permanent --zone=trusted --add-interface=tun1 1>/dev/null

firewall-cmd --add-masquerade 1>/dev/null && firewall-cmd --add-masquerade --permanent 1>/dev/null
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.146.0.0/21 -o $DEVICE -j MASQUERADE 1>/dev/null
firewall-cmd --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.246.0.0/21 -o $DEVICE -j MASQUERADE 1>/dev/null
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.146.0.0/21 -o $DEVICE -j MASQUERADE 1>/dev/null
firewall-cmd --permanent --direct --passthrough ipv4 -t nat -A POSTROUTING -s 10.246.0.0/21 -o $DEVICE -j MASQUERADE 1>/dev/null

dnf -q -y remove checkpolicy gnutls-utils 1>/dev/null
