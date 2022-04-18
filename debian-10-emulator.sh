#!/bin/bash

# If the version string isn't already set, we declare it here.
[ ! -n "$VERSTR" ] && export VERSTR="1.0.2-RC"

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
export ANDROID_AVD_HOME=$HOME/.android/avd/

export ADB="${ANDROID_SDK_ROOT}/platform-tools/adb"
export EMULATOR="${ANDROID_SDK_ROOT}/emulator/emulator"
export AVDMANAGER="${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin/avdmanager"
export SDKMANAGER="${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin/sdkmanager"

${AVDMANAGER} create avd --force --device "pixel_3" --tag "google_apis" --package "system-images;android-30;google_apis;x86_64" --abi "x86_64" --name "generic"

sed -i "s/\(hw.gps[ ]\?=[ ]\?\).*/\1no/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(hw.dPad[ ]\?=[ ]\?\).*/\1yes/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(hw.cpu.ncore[ ]\?=[ ]\?\).*/\14/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(hw.keyboard[ ]\?=[ ]\?\).*/\1yes/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(hw.gpu.mode[ ]\?=[ ]\?\).*/\1host/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(hw.audioInput[ ]\?=[ ]\?\).*/\1no/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(hw.audioOutput[ ]\?=[ ]\?\).*/\1no/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(hw.gpu.enabled[ ]\?=[ ]\?\).*/\1yes/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(fastboot.forceFastBoot[ ]\?=[ ]\?\).*/\1no/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini
sed -i "s/\(fastboot.forceColdBoot[ ]\?=[ ]\?\).*/\1yes/g" ${ANDROID_AVD_HOME}/generic.avd/config.ini

# cd /opt/android-sdk-linux/emulator/
# cd $HOME/Android/Sdk/emulator/
${EMULATOR} -no-boot-anim -no-audio -netfast -no-snapshot -verbose @generic &>> /tmp/generic.emu.log &

${ADB} wait-for-local-device
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
[ -f $BASE/build/openvpn/de.blinkt.openvpn_189.apk ] && ${ADB} install --no-streaming $BASE/build/openvpn/de.blinkt.openvpn_189.apk

# CPU Info Install
[ -f $BASE/build/cpuinfo/com.kgurgul.cpuinfo_40500.apk ] && ${ADB} install --no-streaming $BASE/build/cpuinfo/com.kgurgul.cpuinfo_40500.apk

# Lavabit Proxy Install
# [ -f $BASE/build/outputs/apk/lavabitInsecureFat/debug/Lavabit_Proxy_debug_${VERSTR}.apk ] && ${ADB} install --no-streaming $BASE/build/outputs/apk/lavabitInsecureFat/debug/Lavabit_Proxy_debug_${VERSTR}.apk
[ -f $BASE/build/outputs/apk/lavabitProductionFat/release/Lavabit_Proxy_release_${VERSTR}.apk ] && ${ADB} install --no-streaming $BASE/build/outputs/apk/lavabitProductionFat/release/Lavabit_Proxy_release_${VERSTR}.apk

# or
# QT_QPA_PLATFORM_PLUGIN_PATH=/opt/android-sdk-linux/emulator/lib64/qt/plugins
# QT_OPENGL=software
# QMLSCENE_DEVICE=softwarecontext
#
# ./emulator -use-system-libs -no-boot-anim -crostini -no-audio -netfast -no-snapshot -gpu host -accel on -verbose @generic
${ADB} logcat *:* >> /tmp/generic.sys.log &

# Start the app.
${ADB} shell "am start -a android.intent.action.MAIN -n com.lavabit.pahoehoe/se.leap.bitmaskclient.base.StartActivity"

exit

${AVDMANAGER} delete avd --name generic


# skin.name=1080x1920        # proper screen size for emulator
# hw.lcd.density=480
# -use-system-libs
# -use-host-vulkan
# 1380,244,480,960
# -no-hidpi-scaling
# hw.gltransport=virtio-gpu-pipe
