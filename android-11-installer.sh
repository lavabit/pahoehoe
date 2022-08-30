#!/bin/bash

# If the version string isn't already set, we declare it here.
[ ! -n "$VERSTR" ] && export VERSTR="1.0.3-RC"

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

export ANDROID_SDK_ROOT=$HOME/Android/Sdk/
export ADB="${ANDROID_SDK_ROOT}/platform-tools/adb"


${ADB} wait-for-device
while [ "`${ADB} shell getprop sys.boot_completed | tr -d '\r' `" != "1" ] ; do sleep 1 ; done
while [ "`${ADB} shell getprop init.svc.bootanim | tr -d '\r' `" != "stopped" ] ; do sleep 1 ; done

# Termux install
[ -f $BASE/build/termux/com.termux_118.apk ] && ${ADB} install --no-streaming $BASE/build/termux/com.termux_118.apk
[ -f $BASE/build/termux/com.termux_118.apk ] && [ -f $BASE/build/termux/com.termux.api_51.apk ] && ${ADB} install --no-streaming $BASE/build/termux/com.termux.api_51.apk
[ -f $BASE/build/termux/com.termux_118.apk ] && [ -f $BASE/build/termux/com.termux.boot_7.apk ] && ${ADB} install --no-streaming $BASE/build/termux/com.termux.boot_7.apk
[ -f $BASE/build/termux/com.termux_118.apk ] && [ -f $BASE/build/termux/com.termux.widget_13.apk ] && ${ADB} install --no-streaming $BASE/build/termux/com.termux.widget_13.apk
[ -f $BASE/build/termux/com.termux_118.apk ] && [ -f $BASE/build/termux/com.termux.styling_29.apk ] && ${ADB} install --no-streaming $BASE/build/termux/com.termux.styling_29.apk

# Install ConnectBot
[ -f $BASE/build/connectbot/org.connectbot_10908000.apk ] && ${ADB} install --no-streaming $BASE/build/connectbot/org.connectbot_10908000.apk

# OpenVPN client install
[ -f $BASE/build/openvpn/de.blinkt.openvpn_194.apk ] && ${ADB} install --no-streaming $BASE/build/openvpn/de.blinkt.openvpn_194.apk

# CPU Info Install
[ -f $BASE/build/cpuinfo/com.kgurgul.cpuinfo_40500.apk ] && ${ADB} install --no-streaming $BASE/build/cpuinfo/com.kgurgul.cpuinfo_40500.apk

# Lavabit Proxy Install
# [ -f $BASE/build/outputs/apk/lavabitInsecureFat/debug/Lavabit_Proxy_debug_${VERSTR}.apk ] && ${ADB} install --no-streaming $BASE/build/outputs/apk/lavabitInsecureFat/debug/Lavabit_Proxy_debug_${VERSTR}.apk
[ -f $BASE/build/outputs/apk/lavabitProductionFat/release/Lavabit_Proxy_release_${VERSTR}.apk ] && ${ADB} install --no-streaming $BASE/build/outputs/apk/lavabitProductionFat/release/Lavabit_Proxy_release_${VERSTR}.apk

# Start the app.
${ADB} shell "am start -a android.intent.action.MAIN -n com.lavabit.pahoehoe/se.leap.bitmaskclient.base.StartActivity"

exit 0

# The commands in condensed form, for the purpose of cutting and pasting.
cat <<-EOF
adb install build/termux/com.termux_118.apk
adb install build/termux/com.termux.api_51.apk
adb install build/termux/com.termux.boot_7.apk
adb install build/termux/com.termux.widget_13.apk
adb install build/termux/com.termux.styling_29.apk
adb install build/openvpn/de.blinkt.openvpn_194.apk
adb install build/connectbot/org.connectbot_10908000.apk
adb install build/cpuinfo/com.kgurgul.cpuinfo_40500.apk

adb install build/outputs/apk/lavabitProductionFat/release/Lavabit_Proxy_release_${VERSTR}.apk
adb shell "am start -a android.intent.action.MAIN -n com.lavabit.pahoehoe/se.leap.bitmaskclient.base.StartActivity"
EOF
