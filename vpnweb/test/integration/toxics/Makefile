# Some toxiproxy settings to simulate high latency or interrupted sockets.

PROXY?=sip
LATENCY?=1000

inspect:
	toxiproxy-cli list
	toxiproxy-cli inspect ${PROXY}

create:
	toxiproxy-cli create ${PROXY} --listen localhost:7001 --upstream localhost:6001
	toxiproxy-cli toxic add ${PROXY} --type latency --attributes latency=${LATENCY}

set-latency:
	toxiproxy-cli toxic update ${PROXY} --toxicName latency_downstream -a=jitter=0,latency=${LATENCY}

toggle:
	toxiproxy-cli toggle ${PROXY}
