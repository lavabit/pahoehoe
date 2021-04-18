#!/usr/bin/env bash

cd "$(realpath $(dirname "${BASH_SOURCE[0]}"))"

declare -g pid_file=""
declare -g should_exit=0
declare -g keep_fw=""
declare -g keep_fw2=""
declare -g keep_fw3=""

export PATH=$PATH:/usr/sbin:/sbin

# https://stackoverflow.com/a/26966800
kill_descendant_processes() {
	local pid="$1"
	local and_self="${2:-false}"
	if children="$(pgrep -P "$pid")"; then
		for child in $children; do
			kill_descendant_processes "$child" true
		done
	fi
	if [[ "$and_self" == true ]]; then
		kill -9 "$pid"
	fi
}

_command() {
	IFS=''
	for x in ${@}; do
		if ! command -v $x &> /dev/null; then
			echo "$x could not be found. Please install it." >&2
			declare -g should_exit=1
		fi
	done
}

on_exit() {
	set +u
	local _jobs="$(jobs -p) $(cat "$pid_file" 2>/dev/null)"
	echo "* Killed all of the script's background processes"
	if [ -n "$_jobs" ]; then
		IFS=' '
		for x in $_jobs; do
			kill_descendant_processes $x true >/dev/null 2>&1
		done
	fi
	echo "* Removed residue files"
	rm -f -- "$management_sock" "$pid_file" 2>/dev/null
	declare -g pid_file=""
	declare -g management_sock=""
	if [ "$1" != "nofwstop" ]; then
		fw_stop >/dev/null 2>&1
	fi
	echo ""
	set -u
}

_command curl jq sed umask mktemp tee openvpn sh grep nc id awk ip iptables ip6tables cut pgrep kill conntrack openssl tr

[ "$should_exit" = "1" ] && exit 1
unset should_exit

# Check if required user and group for openvpn are installed
if [ "$(getent passwd nobody)" = "" ];then
	echo "You need to have nobody as a user" >&2
	declare -g should_exit=1
fi
if [ "$(getent group nobody)" != "" ];then
	declare -g unprivgroup="nobody"
elif [ "$(getent group nogroup)" != "" ];then
	declare -g unprivgroup="nogroup"
else
	echo "You need to have either nobody or nogroup as a group" >&2
	declare -g should_exit=1
fi
[ "$should_exit" = "1" ] && exit 1
unset should_exit

trap 'on_exit' EXIT

set -um

declare -g _lavabitvpn="https://api.proxy.lavabit.com"
# declare -g _lavabitvpn_gw="https://api.proxy.lavabit.com"
declare -g _lavabitvpn_gw="none"
declare -g _lavabitvpn_ca="https://api.proxy.lavabit.com/provider.json"
declare -a -g _curl_std_opts_api=()
declare -a -g blacklist_locations
source lavabitvpn.config >/dev/null 2>&1
declare -g _curl_std_opts_api+=(--silent --fail "--capath" "/dev/null")
declare -a -g _curl_fw_ip=()
declare -g api_cert=""

get_api_ca() {
	local x=0; while :; do
		local _curl_fw_ip=("--connect-to" "$(echo "$_lavabitvpn_ca" | awk -F[/:] '{print $4}')::$keep_fw3:")
		IFS=$'\n'
		local ca_cert=( $(curl --insecure --silent "${_curl_fw_ip[@]}" "${_lavabitvpn_ca}" | jq -cr '.ca_cert_uri+"\n"+.ca_cert_fingerprint'))
		declare -g api_cert="$(curl --insecure --silent "${_curl_fw_ip[@]}" "${ca_cert[0]}")"
		local api_finger="$(echo "$api_cert" | openssl x509 -sha256 -fingerprint -noout | sed -e 's/://g' -e 's/ Fingerprint=/: /g' | tr '[:lower:]' '[:upper:]')"
		ca_cert[1]="$(echo "${ca_cert[1]}" | tr '[:lower:]' '[:upper:]')"
		if [ "${api_finger}" == "${ca_cert[1]}" ]; then
			echo "* Got API certificate and verfied";
			return 0
		else
			echo "* API certificate is invalid and verfication failed. Retrying"
			[ $x -ge 10 ] && exit 1
		fi
		local x=$((x+1))
	done
}

make_cert_and_cmdline() {
	[ -n "$keep_fw" ] && local _curl_fw_ip=("--connect-to" "$(echo "$_lavabitvpn" | awk -F[/:] '{print $4}')::$keep_fw:")
	if [ "$_lavabitvpn_gw" != "none" ];then
		[ -n "$keep_fw2" ] && local _curl_fw_ip+=("--connect-to" "$(echo "$_lavabitvpn_gw" | awk -F[/:] '{print $4}')::$keep_fw2:")
		echo "* Getting list of closest VPN gateways"
		local lavabitvpn_gw_list="$(curl "${_curl_std_opts_api[@]}" "${_curl_fw_ip[@]}" --cacert <(printf %s "$api_cert") "$_lavabitvpn_gw/geoip.json")"
		local -a lavabitvpn_gw_sel=( $(echo "$lavabitvpn_gw_list" | jq -cr '.gateways[0:] | .[]') )
		unset lavabitvpn_gw_list
	fi
	echo "* Getting new public and private certificate for the OpenVPN connection"
	local lavabitvpn_cert="$(curl "${_curl_std_opts_api[@]}" "${_curl_fw_ip[@]}" --cacert <(printf %s "$api_cert") "$_lavabitvpn/3/cert" || curl "${_curl_std_opts_api[@]}" "${_curl_fw_ip[@]}" --cacert <(printf %s "$api_cert") "$_lavabitvpn/1/cert")"
	declare -g lavabitvpn_private_key="$(echo "$lavabitvpn_cert" | sed -e '/-----BEGIN RSA PRIVATE KEY-----/,/-----END RSA PRIVATE KEY-----/!d')"
	declare -g lavabitvpn_public_key="$(echo "$lavabitvpn_cert" | sed -e '/-----BEGIN CERTIFICATE-----/,/-----END CERTIFICATE-----/!d')"
	unset lavabitvpn_cert

	declare -a -g make_opts=""
	declare -a -g firewall=""
	echo "* Getting list of all VPN gateways, OpenVPN configuration and IP addresses"
	declare lavabitvpn_gws="$(curl "${_curl_std_opts_api[@]}" "${_curl_fw_ip[@]}" --cacert <(printf %s "$api_cert") "$_lavabitvpn/3/config/eip-service.json" || curl "${_curl_std_opts_api[@]}" "${_curl_fw_ip[@]}" --cacert <(printf %s "$api_cert") "$_lavabitvpn/1/config/eip-service.json")"
	declare -a gw_len=( $(echo "$lavabitvpn_gws" | jq -r '.gateways[] | .ip_address') )
	IFS=''; for i in ${!gw_len[@]}; do
		set +u
		[ -n "$lavabitvpn_gw_sel[$i]" ] && local lavabitvpn_gw_sel[$i]="$(echo "$lavabitvpn_gws" | jq -cr  ".gateways[] | select(.host == \"${lavabitvpn_gw_sel[$i]}\") | .ip_address")"
		set -u
		if [ -z "${lavabitvpn_gw_sel[$i]}" ]; then
			if [ "$_lavabitvpn_gw" != "none" ];then
				echo "* List of closest servers failed. Picking server(s) at random."
			fi
			local lavabitvpn_gw_sel[$i]="$(echo "$lavabitvpn_gws" | jq -cr  ".gateways[$i] | .ip_address")"
		fi
		local location="$(echo "$lavabitvpn_gws" | jq -cr ".gateways[] | select(.ip_address == \"${lavabitvpn_gw_sel[$i]}\") | to_entries[] | select(.key== \"location\") | .value")"
		if [[ ! ${blacklist_locations[*]} =~ $location ]] || [[ -z $location ]]; then
			local port="$(echo "$lavabitvpn_gws" | jq -cr  ".gateways[] | select(.ip_address == \"${lavabitvpn_gw_sel[$i]}\") | to_entries[] | select(.key == \"capabilities\")| .value.transport | .[] | select(.type == \"openvpn\") | .ports[0]")"
			local proto="$(echo "$lavabitvpn_gws" | jq -cr  ".gateways[] | select(.ip_address == \"${lavabitvpn_gw_sel[$i]}\") | to_entries[] | select(.key == \"capabilities\")| .value.transport | .[] | select(.type == \"openvpn\") | .protocols[0]")"
			declare -g firewall[i]="${lavabitvpn_gw_sel[$i]} $port $proto"
			case $proto in
					tcp) local proto="tcp-client" ;;
			esac
			declare -a -g make_opts+=("remote ${lavabitvpn_gw_sel[$i]} $port $proto")
		fi
	done
	declare -g ovpn_config_file="$(echo "$lavabitvpn_gws" | jq -rc '.openvpn_configuration | to_entries[] | "--\(.key) \"\(.value)\""')"
	IFS=$'\n' declare -a -g ovpn_config_file=( $(echo "$ovpn_config_file" | sed -e '/tun-ipv6/d' -e '/ \"False\|false\"$/d' -e 's/ \"True\|true\"$//g' -e 's/ \"/ /g' -e 's/\"$//g' -e 's/^--//g') )
	declare -g ovpn_config_file="$(IFS=''; for x in ${!ovpn_config_file[@]}; do echo ${ovpn_config_file[x]}; done;)"
	declare -g ovpn_config_file+="$(IFS=''; for x in ${!make_opts[@]}; do echo ${make_opts[x]}; done;)"
	unset lavabitvpn_gws lavabitvpn_gw_sel gw_len make_opts
}

keep_fw_onreconnect() {
	local x=0; while [ -z "$keep_fw" ] || [ -z "$keep_fw2" ] || [ -z "$keep_fw3" ]; do
		[ "$x" == "0" ] && echo "* Getting API IP addresses for reconnect" && local x=1
		sleep 0.1
		declare -g keep_fw="$(getent ahostsv4 "$(echo "$_lavabitvpn" | awk -F[/:] '{print $4}')" | grep STREAM | head -n 1 | cut -d ' ' -f 1)"
		if [ "$_lavabitvpn_gw" != "none" ];then
			declare -g keep_fw2="$(getent ahostsv4 "$(echo "$_lavabitvpn_gw" | awk -F[/:] '{print $4}')" | grep STREAM | head -n 1 | cut -d ' ' -f 1)"
		else
			declare -g keep_fw2=none
		fi
		declare -g keep_fw3="$(getent ahostsv4 "$(echo "$_lavabitvpn_ca" | awk -F[/:] '{print $4}')" | grep STREAM | head -n 1 | cut -d ' ' -f 1)"
	done
	[ "$x" == "1" ] && echo "* Got API IP addresses for reconnect"
	fw_start >/dev/null 2>&1
}

openvpn_start() {
	IFS=''
	declare -g management_sock="$(mktemp -u)"
	declare -g pid_file="$(mktemp -u)"
	openvpn --daemon --config <(printf %s "$ovpn_config_file") --ca <(printf %s "$api_cert") \
		--cert <(printf %s "$lavabitvpn_public_key") --key <(printf %s "$lavabitvpn_private_key") \
		--remap-usr1 SIGTERM --client --nobind --management $management_sock unix --management-signal \
		--management-client-user "$(id -un)" --management-client-group "$(id -gn)" --dev tun0 \
		--tls-client --remote-cert-tls server --persist-key --persist-tun --persist-local-ip --auth-nocache \
		--user nobody --group "$unprivgroup" --writepid "$pid_file" --script-security 1 --verb 0 >/dev/null 2>&1
}

check_if_changes() {
	while IFS= read -r line || [[ -n "$line" ]]; do
		echo "$line" | grep -E -m 1 '^>STATE:.*,CONNECTED,SUCCESS,' >/dev/null 2>&1 && conntrack --flush >/dev/null 2>&1
		echo "$line" | grep -E -m 1 '^>STATE:.*,RECONNECTING,'      >/dev/null 2>&1 && break             >/dev/null 2>&1
	done < <(echo -e 'state on' | nc -U "$management_sock")
}

fw_start() {
	IFS=' '

	iptables  --new-chain lavabit
	ip6tables --new-chain lavabit

	iptables  -t nat --new-chain lavabit-nat
	ip6tables -t nat --new-chain lavabit-nat

	iptables  -t nat --new-chain lavabit-post
	ip6tables -t nat --new-chain lavabit-post

	# Add lavabit chain to OUTPUT
	iptables  -C OUTPUT --jump lavabit || iptables  -I OUTPUT --jump lavabit
	ip6tables -C OUTPUT --jump lavabit || ip6tables -I OUTPUT --jump lavabit

	# Add lavabit-nat to OUTPUT nat
	iptables   -t nat -C OUTPUT --jump lavabit-nat || iptables  -t nat -I OUTPUT --jump lavabit-nat
	ip6tables  -t nat -C OUTPUT --jump lavabit-nat || ip6tables -t nat -I OUTPUT --jump lavabit-nat

	# Add lavabit-nat to OUTPUT nat
	iptables   -t nat -C POSTROUTING --jump lavabit-post || iptables  -t nat -I POSTROUTING --jump lavabit-post
	ip6tables  -t nat -C POSTROUTING --jump lavabit-post || ip6tables -t nat -I POSTROUTING --jump lavabit-post

	# Reject all before doing rule below
	iptables  -I OUTPUT -j REJECT
	ip6tables -I OUTPUT -j REJECT

	# Flush all OUTPUT reject
	iptables  -F lavabit
	ip6tables -F lavabit

	# Flush all nat and postrouting
	iptables  -t nat -F lavabit-nat
	ip6tables -t nat -F lavabit-nat
	iptables  -t nat -F lavabit-post
	ip6tables -t nat -F lavabit-post

	# Reject all from all
	iptables  -I lavabit -j REJECT
	ip6tables -I lavabit -j REJECT

	# Remove OUTPUT Rejection
	iptables  -D OUTPUT 1
	ip6tables -D OUTPUT 1

	# Block all from lavabitvpn tun
	iptables  -I lavabit -o tun0 -j REJECT
	ip6tables -I lavabit -o tun0 -j REJECT

	# Block all IPv4 Internet from non-lavabitvpn
	local internet="0.0.0.0/5 8.0.0.0/7 11.0.0.0/8 12.0.0.0/6 16.0.0.0/4 32.0.0.0/3 64.0.0.0/2 128.0.0.0/3 160.0.0.0/5 168.0.0.0/6 172.0.0.0/12 172.32.0.0/11 172.64.0.0/10 172.128.0.0/9 173.0.0.0/8 174.0.0.0/7 176.0.0.0/4 192.0.0.0/9 192.128.0.0/11 192.160.0.0/13 192.169.0.0/16 192.170.0.0/15 192.172.0.0/14 192.176.0.0/12 192.192.0.0/10 193.0.0.0/8 194.0.0.0/7 196.0.0.0/6 200.0.0.0/5 208.0.0.0/4"
	for x in $internet;do
		iptables -I lavabit -d $x -j REJECT
	done

	# Block all IPv6 internet from non-lavabitvpn
	local internet6="2000::/3"
	for x in $internet6; do
		ip6tables -I lavabit -d $x -j REJECT
	done

	# Allow loopback interface
	iptables  -I lavabit -o lo+ -d 127.0.0.0/8 -j ACCEPT
	ip6tables -I lavabit -o lo+ -d     ::1/128 -j ACCEPT

	# Allow the Lavabit API addresses and DNS
	local z=0
	[ -n "$keep_fw"  ] && iptables -I lavabit -d ${keep_fw} -j ACCEPT && local z=1
	[ -n "$keep_fw2" ] && [ "$keep_fw2" != "none" ] && iptables -I lavabit -d ${keep_fw2} -j ACCEPT
	[ -n "$keep_fw3" ] && iptables -I lavabit -d ${keep_fw3} -j ACCEPT
	# if [ "$z" = "1" ];then
		# iptables  -t nat -I lavabit-nat -p udp --dport 53 -j DNAT --to 10.41.0.1:53
		# iptables  -t nat -I lavabit-nat -p tcp --dport 53 -j DNAT --to 10.41.0.1:53
		# iptables  -t nat -I lavabit-nat -p udp -o lo+ --dest 127.0.0.0/8 --dport 53 -j ACCEPT
		# iptables  -t nat -I lavabit-nat -p tcp -o lo+ --dest 127.0.0.0/8 --dport 53 -j ACCEPT
		# iptables  -t nat -I lavabit-post -p udp -o tun0 --dest 10.41.0.1 --dport 53 --jump MASQUERADE
		# iptables  -t nat -I lavabit-post -p tcp -o tun0 --dest 10.41.0.1 --dport 53 --jump MASQUERADE
		# iptables  -I lavabit -p tcp --dport 53 -j ACCEPT
		# iptables  -I lavabit -p udp --dport 53 -j ACCEPT
		# ip6tables -I lavabit -p tcp --dport 53 -j REJECT
		# ip6tables -I lavabit -p udp --dport 53 -j REJECT
	# else
		# for x in '' 6; do
			# ip${x}tables -I lavabit -p tcp --dport 53 -j ACCEPT
			# ip${x}tables -I lavabit -p udp --dport 53 -j ACCEPT
		# done
	# fi

	# Allow only IPv4 Internet to go through lavabitvpn
	for x in $internet;do
		iptables  -I lavabit -d $x -o tun0 -j ACCEPT
	done

	# Allow only IPv6 Internet to go through lavabitvpn
	for x in $internet6;do
		ip6tables -I lavabit -d $x -o tun0 -j ACCEPT
	done

	# Allow the current ranges (incase we have a public IP)
	local routes=( $(ip route list | grep '\bproto kernel\b' | cut -d' ' -f1) )
	local ifs=( $(ip route list | grep '\bproto kernel\b' | cut -d' ' -f3) )
	for i in ${!routes[@]}; do
		iptables  -I lavabit -d ${routes[$i]} -o ${ifs[$i]} -j ACCEPT
	done
	local routes6=( $(ip -6 route list | grep '\bproto kernel\b' | cut -d' ' -f1) )
	local ifs6=( $(ip -6 route list | grep '\bproto kernel\b' | cut -d' ' -f3) )
	for i in ${!routes6[@]}; do
		ip6tables -I lavabit -d ${routes6[$i]} -o ${ifs6[$i]} -j ACCEPT
	done

	# Excempt the lavabitvpn IPs
	for i in ${!firewall[@]}; do
		local fw_ip="$(echo ${firewall[$i]} | cut -d ' ' -f1)"
		local fw_port="$(echo ${firewall[$i]} | cut -d ' ' -f2)"
		local fw_proto="$(echo ${firewall[$i]} | cut -d ' ' -f3)"
		iptables -I lavabit -p $fw_proto -d $fw_ip --dport $fw_port -j ACCEPT
	done

	# Now we can delete REJECT all
	iptables  -D lavabit -j REJECT
	ip6tables -D lavabit -j REJECT

	# Reset connection
	conntrack --flush
}

fw_stop() {
	iptables  --flush lavabit
	ip6tables --flush lavabit

	iptables  --delete OUTPUT --jump lavabit
	ip6tables --delete OUTPUT --jump lavabit

	iptables  --delete-chain lavabit
	ip6tables --delete-chain lavabit

	iptables  -t nat --flush lavabit-nat
	ip6tables -t nat --flush lavabit-nat

	iptables  -t nat --delete OUTPUT --jump lavabit-nat
	ip6tables -t nat --delete OUTPUT --jump lavabit-nat

	iptables  -t nat --delete-chain lavabit-nat
	ip6tables -t nat --delete-chain lavabit-nat

	iptables  -t nat --flush lavabit-post
	ip6tables -t nat --flush lavabit-post

	iptables  -t nat --delete POSTROUTING --jump lavabit-post
	ip6tables -t nat --delete POSTROUTING --jump lavabit-post

	iptables  -t nat --delete-chain lavabit-post
	ip6tables -t nat --delete-chain lavabit-post

	conntrack --flush
}

main() {
	local y=1;while :; do
		echo "* Connection #$y"
		keep_fw_onreconnect >/dev/null 2>&1
		get_api_ca
		make_cert_and_cmdline
		keep_fw_onreconnect >/dev/null 2>&1
		openvpn_start >/dev/null 2>&1
		keep_fw_onreconnect >/dev/null 2>&1
		echo "* Started OpenVPN client"
		echo "* Monitoring OpenVPN status"
		check_if_changes
		echo "* Connection failed"
		echo "* Waiting 5 seconds after next reconnect..."
		sleep 5
		echo ""
		local y=$(($y+1))
		on_exit nofwstop
	done
}; main
