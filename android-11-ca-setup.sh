#!/bin/bash -ex

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

curl --silent --insecure --output $BASE/build/ca.pem https://api.proxy.lavabit.com/ca.crt
curl --silent --cacert $BASE/build/ca.pem --output /dev/null https://api.proxy.lavabit.com/provider.json

CERT_HASH=$(openssl x509 -inform PEM -subject_hash_old -in $BASE/build/ca.pem | head -1)
CERT_FILE="$BASE/build/$CERT_HASH.0"
cp $BASE/build/ca.pem $BASE/build/$CERT_FILE

openssl x509 -inform PEM -text -in $BASE/build/ca.pem -out /dev/null >> $CERT_FILE

adb root && sleep 10

adb wait-for-device-sideload
adb push $CERT_FILE /data/misc/user/0/cacerts-added/$CERT_FILE
adb shell "su 0 chmod 644 /data/misc/user/0/cacerts-added/$CERT_FILE" && sleep 10

## The commands to preload the certificate into the Termux CA repository.
# if [ -f $HOME/com.termux_108.apk ]; then
#   adb wait-for-device-sideload
#   adb install -r -g --no-streaming $HOME/com.termux_108.apk && sleep 5
#   adb shell "am start -a android.intent.action.MAIN -n com.termux/.HomeActivity" && sleep 10
#   adb root && sleep 10
#
#   adb push $CERT_FILE /storage/emulated/0/Download/$CERT_FILE && sleep 5
#   adb shell "su 0 cat /storage/emulated/0/Download/$CERT_FILE >> /data/data/com.termux/files/usr/etc/tls/cert.pem"
#   adb push $CERT_FILE /data/data/com.termux/files/usr/etc/tls/cert.pem && sleep 10
# fi

adb reboot
rm --force $BASE/build/ca.pem $CERT_FILE
