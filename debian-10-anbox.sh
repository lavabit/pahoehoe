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

# Download Termux
[ -d $BASE/apk/termux/ ] && rm --force --recursive $BASE/apk/termux/ ; mkdir --parents $BASE/apk/termux/
curl --silent --location --output $BASE/apk/termux/com.termux_106.apk https://f-droid.org/repo/com.termux_106.apk
printf "b28e4dac0707655c6d8d22abaf45338029ce434086e3d88b64bdee1e84c04ec3  $BASE/apk/termux/com.termux_106.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/apk/termux/com.termux.api_47.apk https://f-droid.org/repo/com.termux.api_47.apk
printf "086b8d7f098cee431bfac615213eae2e2cbb44f6f2543ee38a12e0f36b3098f8  $BASE/apk/termux/com.termux.api_47.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/apk/termux/com.termux.widget_11.apk https://f-droid.org/repo/com.termux.widget_11.apk
printf "934cfb004993348d207ad3e21928e94fba07cb8185ba292ab5209eab09c15dcc  $BASE/apk/termux/com.termux.widget_11.apk" | sha256sum -c || exit 1

# Download the OpenVPN Android GUI
[ -d $BASE/apk/openvpn/ ] && rm --force --recursive $BASE/apk/openvpn/ ; mkdir --parents $BASE/apk/openvpn/
curl --silent --location --output $BASE/apk/openvpn/de.blinkt.openvpn_175.apk https://f-droid.org/repo/de.blinkt.openvpn_175.apk
printf "359dd465e81c796c98f9cb4deb493956715c5834ebf31b643f366dcf6a713037  $BASE/apk/openvpn/de.blinkt.openvpn_175.apk" | sha256sum -c || exit 1

# First attempt.
export $(dbus-launch)
sudo systemctl stop anbox-container-manager.service && sleep 10
sudo /usr/share/anbox/anbox-bridge.sh start && sleep 10
IPADDR="`ip -br a show dev anbox0 | awk -F' ' '{print $3}' | awk -F'/' '{print $1}'`"
#sudo ANBOX_LOG_LEVEL=debug anbox container-manager --daemon --privileged --data-path=/var/lib/anbox --container-network-dns-servers=$IPADDR &
sudo /usr/bin/anbox container-manager --daemon --privileged --data-path=/var/lib/anbox --container-network-dns-servers="`ip -br a show dev eth0 | awk -F' ' '{print $3}' | awk -F'/' '{print $1}'`" &
env ANBOX_LOG_LEVEL=debug ANBOX_FORCE_SOFTWARE_RENDERING=1 EGL_LOG_LEVEL=debug anbox session-manager --single-window --window-size=1024,768 &
adb install $HOME/bitmask_android_leap/app/build/outputs/apk/normalInsecureFat/debug/Bitmask_debug.apk
adb install $HOME/com.termux_104.apk

# Alternative attempt.
export $(dbus-launch)
sudo /usr/local/bin/anbox container-manager --daemon --privileged --data-path=/var/lib/anbox &
env ANBOX_LOG_LEVEL=debug ANBOX_FORCE_SOFTWARE_RENDERING=1 EGL_LOG_LEVEL=debug /usr/local/bin/anbox session-manager --single-window --window-size=1024,768 &
adb install $HOME/bitmask_android_leap/app/build/outputs/apk/normalInsecureFat/debug/Bitmask_debug.apk
adb install $HOME/com.termux_104.apk
# sudo tail -f /var/lib/anbox/logs/console.log
