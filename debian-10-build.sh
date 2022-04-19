#!/bin/bash

export ANDROID_AVD_HOME=$HOME/.avd
export ANDROID_SDK_HOME=$HOME/.android
export ANDROID_PREFS_ROOT=$HOME/.android
export ANDROID_HOME=/opt/android-sdk-linux
export ANDROID_SDK_ROOT=/opt/android-sdk-linux

export GRADLE_USER_HOME=$HOME/.gradle
# export GRADLE_OPTS=-Dorg.gradle.daemon=false

# alias javac='javac -Xlint:-deprecation -Xlint:-unchecked'
# export JDK_JAVAC_OPTIONS="-Xlint:-deprecation -Xlint:-unchecked"

if [ -f $HOME/key.sh ] && [ ! -n "$VERSTR" ]; then
  printf "A production signing key was provided, but the required VERSTR (version string) variable is missing.\n"
  exit 1
elif [ ! -n "$VERSTR" ]; then
  printf "The VERSTR variable is empty. The build.gradle value will be used instead.\n"
fi

# Remove everything that might be left over from a previous build.
cd $HOME && rm --recursive --force $HOME/.cache/Google/ $HOME/.config/Google/ $HOME/.local/share/Google/ $HOME/.cache/Android\ Open\ Source\ Project/ $HOME/.config/Android\ Open\ Source\ Project $HOME/.local/share/Android\ Open\ Source\ Project/
cd $HOME && rm --recursive --force $HOME/.java/ $HOME/.gradle/ $HOME/.android/ $HOME/.cache/go-build/ $HOME/.cache/JNA/
[ -n "`pidof java`" ] && kill `ps -ef | grep -v grep | grep -E "org.gradle.wrapper.GradleWrapperMain|org.gradle.launcher.daemon.bootstrap.GradleDaemon" | awk '{print $2}'`

[ -d $HOME/android ] && find $HOME/android -mindepth 1 -depth -exec rm -rf {} \;
git clone --quiet https://github.com/lavabit/pahoehoe.git android && cd android && FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch --subdirectory-filter android

# The version string has to exist as a tag or the build will fail. So we create it here, if it doesn't exist already.
if [ -n "$VERSTR" ]; then
  git rev-parse --quiet --verify "${VERSTR}" &>/dev/null || git tag "${VERSTR}"
fi

# Update the Development Fingerprints
curl --silent --insecure https://api.centos.local/provider.json > $HOME/android/app/src/test/resources/preconfigured/centos.local.json
curl --silent --insecure https://api.centos.local/ca.crt > $HOME/android/app/src/test/resources/preconfigured/centos.local.pem

curl --silent --insecure https://api.debian.local/provider.json > $HOME/android/app/src/test/resources/preconfigured/debian.local.json
curl --silent --insecure https://api.debian.local/ca.crt > $HOME/android/app/src/test/resources/preconfigured/debian.local.pem

cat <<-EOF > $HOME/android/local.properties

org.gradle.jvmargs=-Xincgc -Xmx8192m -XX:MaxMetaspaceSize=2048m
cmake.dir=/opt/android-sdk-linux/cmake/3.10.2.4988404/
sdk.dir=/opt/android-sdk-linux/
android.ndkVersion=21.4.7075529

android.enableSeparateAnnotationProcessing=false
org.gradle.parallel=true
org.gradle.workers.max=6
org.gradle.caching=true
org.gradle.daemon=true

EOF

cat <<-EOF > $HOME/android/app/local.properties

sdk.dir=/opt/android-sdk-linux/

EOF

# If the key script is available, and the key is missing, then run the key script.
if [ -f $HOME/key.sh ]; then
  bash $HOME/key.sh
fi

# Remove any key/signing settings that might be in the gradle.properties file to
# avoid ending up with duplicates, if the logic below gets used.
sed -i -e '/storeFileProperty/d' -e '/storePasswordProperty/d' -e '/keyAliasProperty/d' -e '/keyPasswordProperty/d' \
-e '/v1SigningEnabledProperty/d' -e '/v2SigningEnabledProperty/d' $HOME/android/gradle.properties

# If the signing key is present, use it to create release ready files.
if [ -f $HOME/android/lavabit.jks ]; then
cat <<-EOF >> $HOME/android/gradle.properties

storeFileProperty=/home/vagrant/android/lavabit.jks
storePasswordProperty=lavabit.com
keyAliasProperty=Lavabit Encrypted Proxy
keyPasswordProperty=lavabit.com

v1SigningEnabledProperty=true
v2SigningEnabledProperty=true

EOF
fi

git update-index --assume-unchanged $HOME/android/ics-openvpn/main/build/ovpnassets/.empty
git update-index --assume-unchanged $HOME/android/app/src/test/resources/preconfigured/centos.local.json
git update-index --assume-unchanged $HOME/android/app/src/test/resources/preconfigured/centos.local.pem
git update-index --assume-unchanged $HOME/android/app/src/test/resources/preconfigured/debian.local.json
git update-index --assume-unchanged $HOME/android/app/src/test/resources/preconfigured/debian.local.pem

./scripts/build_deps.sh
./gradlew --console plain --warning-mode none assembleLavabit assembleNormalProductionFatweb
./gradlew --console plain --warning-mode none bundleLavabit
./gradlew --console plain --warning-mode none check

# ./gradlew --warning-mode none bundle
# ./gradlew --warning-mode none assembleDebug
# ./gradlew --warning-mode none assembleRelease
# ./gradlew --warning-mode none connectedCheck

echo "All finished."
sudo fstrim --all


