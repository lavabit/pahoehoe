#!/bin/bash -ex

curl --silent --insecure --output ca.pem https://api.proxy.lavabit.com/ca.crt
curl --silent --cacert ca.pem --output /dev/null https://api.proxy.lavabit.com/provider.json

CERT_HASH=$(openssl x509 -inform PEM -subject_hash_old -in ca.pem | head -1)
CERT_FILE="$CERT_HASH.0"
cp ca.pem $CERT_FILE

openssl x509 -inform PEM -text -in ca.pem -out /dev/null >> $CERT_FILE

/home/ladar/Android/Sdk/platform-tools/adb root && sleep 10

/home/ladar/Android/Sdk/platform-tools/adb wait-for-device-sideload
/home/ladar/Android/Sdk/platform-tools/adb push $CERT_FILE /data/misc/user/0/cacerts-added/$CERT_FILE
/home/ladar/Android/Sdk/platform-tools/adb shell "su 0 chmod 644 /data/misc/user/0/cacerts-added/$CERT_FILE" && sleep 10

if [ -f $HOME/proxy/apk/normalInsecureFat/debug/Bitmask_debug.apk ]; then
  /home/ladar/Android/Sdk/platform-tools/adb wait-for-device-sideload
  /home/ladar/Android/Sdk/platform-tools/adb install -r -g --no-streaming $HOME/proxy/apk/normalInsecureFat/debug/Bitmask_debug.apk && sleep 10
fi

if [ -f $HOME/com.termux_104.apk ]; then
  /home/ladar/Android/Sdk/platform-tools/adb wait-for-device-sideload
  /home/ladar/Android/Sdk/platform-tools/adb install -r -g --no-streaming $HOME/com.termux_104.apk && sleep 5
  /home/ladar/Android/Sdk/platform-tools/adb shell "am start -a android.intent.action.MAIN -n com.termux/.HomeActivity" && sleep 10
  /home/ladar/Android/Sdk/platform-tools/adb root && sleep 10

  # /home/ladar/Android/Sdk/platform-tools/adb push $CERT_FILE /storage/emulated/0/Download/$CERT_FILE && sleep 5
  # /home/ladar/Android/Sdk/platform-tools/adb shell "su 0 cat /storage/emulated/0/Download/$CERT_FILE >> /data/data/com.termux/files/usr/etc/tls/cert.pem"
  /home/ladar/Android/Sdk/platform-tools/adb push $CERT_FILE /data/data/com.termux/files/usr/etc/tls/cert.pem && sleep 10
fi

/home/ladar/Android/Sdk/platform-tools/adb reboot
rm --force ca.pem $CERT_FILE
