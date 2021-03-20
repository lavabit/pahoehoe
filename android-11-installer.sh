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

export ANDROID_SDK_ROOT=$HOME/Android/Sdk/
export ADB="${ANDROID_SDK_ROOT}/platform-tools/adb"


${ADB} wait-for-device
while [ "`${ADB} shell getprop sys.boot_completed | tr -d '\r' `" != "1" ] ; do sleep 1 ; done
while [ "`${ADB} shell getprop init.svc.bootanim | tr -d '\r' `" != "stopped" ] ; do sleep 1 ; done

# Termux install
[ -f $BASE/apk/termux/com.termux_106.apk ] && ${ADB} install --no-streaming $BASE/apk/termux/com.termux_106.apk
[ -f $BASE/apk/termux/com.termux_106.apk ] && [ -f $BASE/apk/termux/com.termux.api_47.apk ] && ${ADB} install --no-streaming $BASE/apk/termux/com.termux.api_47.apk
[ -f $BASE/apk/termux/com.termux_106.apk ] && [ -f $BASE/apk/termux/com.termux.boot_7.apk ] && ${ADB} install --no-streaming $BASE/apk/termux/com.termux.boot_7.apk
[ -f $BASE/apk/termux/com.termux_106.apk ] && [ -f $BASE/apk/termux/com.termux.widget_11.apk ] && ${ADB} install --no-streaming $BASE/apk/termux/com.termux.widget_11.apk
[ -f $BASE/apk/termux/com.termux_106.apk ] && [ -f $BASE/apk/termux/com.termux.styling_29.apk ] && ${ADB} install --no-streaming $BASE/apk/termux/com.termux.styling_29.apk

# Install ConnectBot
[ -f $BASE/apk/connectbot/org.connectbot_10906000.apk ] && ${ADB} install --no-streaming $BASE/apk/connectbot/org.connectbot_10906000.apk

# OpenVPN client install
[ -f $BASE/apk/openvpn/de.blinkt.openvpn_175.apk ] && ${ADB} install --no-streaming $BASE/apk/openvpn/de.blinkt.openvpn_175.apk

# CPU Info Install
[ -f $BASE/apk/cpuinfo/com.kgurgul.cpuinfo_40403.apk ] && ${ADB} install --no-streaming $BASE/apk/cpuinfo/com.kgurgul.cpuinfo_40403.apk

# Lavabit Proxy Install
# [ -f $BASE/apk/customInsecureFat/debug/LavabitProxy_debug_1.0.0-RC1.apk ] && ${ADB} install --no-streaming $BASE/apk/customInsecureFat/debug/LavabitProxy_debug_1.0.0-RC1.apk
[ -f $BASE/apk/customProductionFat/release/LavabitProxy_release_1.0.0-RC1.apk ] && ${ADB} install --no-streaming $BASE/apk/customProductionFat/release/LavabitProxy_release_1.0.0-RC1.apk

# Bitmask Install
# [ -f $BASE/apk/normalInsecureFat/debug/Bitmask_debug_1.0.0-RC1.apk ] && ${ADB} install --no-streaming $BASE/apk/normalInsecureFat/debug/Bitmask_debug_1.0.0-RC1.apk
[ -f $BASE/apk/normalProductionFat/release/Bitmask_release_1.0.0-RC1.apk ] && ${ADB} install --no-streaming $BASE/apk/normalProductionFat/release/Bitmask_release_1.0.0-RC1.apk

# Start the app.
${ADB} shell "am start -a android.intent.action.MAIN -n com.lavabit.pahoehoe/se.leap.bitmaskclient.base.StartActivity"
