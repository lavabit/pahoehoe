#!/bin/bash

# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=$BASH_SOURCE
else
  export CMD=$0
fi

# Ensure a consistent working directory so relative paths work.
pushd `dirname $CMD` > /dev/null
BASE=`pwd -P`
popd > /dev/null
cd $BASE

# First attempt.
export $(dbus-launch)
sudo systemctl stop anbox-container-manager.service && sleep 10
sudo /usr/share/anbox/anbox-bridge.sh start && sleep 10
IPADDR="`ip -br a show dev anbox0 | awk -F' ' '{print $3}' | awk -F'/' '{print $1}'`"
#sudo ANBOX_LOG_LEVEL=debug anbox container-manager --daemon --privileged --data-path=/var/lib/anbox --container-network-dns-servers=$IPADDR &
sudo /usr/bin/anbox container-manager --daemon --privileged --data-path=/var/lib/anbox --container-network-dns-servers="`ip -br a show dev eth0 | awk -F' ' '{print $3}' | awk -F'/' '{print $1}'`" &
env ANBOX_LOG_LEVEL=debug ANBOX_FORCE_SOFTWARE_RENDERING=1 EGL_LOG_LEVEL=debug anbox session-manager --single-window --window-size=1024,768 &

# Alternative attempt.
export $(dbus-launch)
sudo /usr/local/bin/anbox container-manager --daemon --privileged --data-path=/var/lib/anbox &
env ANBOX_LOG_LEVEL=debug ANBOX_FORCE_SOFTWARE_RENDERING=1 EGL_LOG_LEVEL=debug /usr/local/bin/anbox session-manager --single-window --window-size=1024,768 &
# sudo tail -f /var/lib/anbox/logs/console.log
