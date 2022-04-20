#!/bin/bash

# Calculate how much memory we have so we can tune Java appropriately.
export TOTALMEM=`free -m | grep -E "^Mem:" | awk -F' ' '{print $2}'`
export MOSTMEM=`echo $(($TOTALMEM/3))`
export QUARTERMEM=`echo $(($TOTALMEM/4))`

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

cmake.dir=/opt/android-sdk-linux/cmake/3.10.2.4988404/
sdk.dir=/opt/android-sdk-linux/
android.ndkVersion=21.4.7075529

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

# Override the default JVM options.
sed -i -e '/org.gradle.jvmargs/d' $HOME/android/local.properties
sed -i -e '/org.gradle.jvmargs/d' $HOME/android/gradle.properties
sed -i -e '/org.gradle.jvmargs/d' $HOME/android/ics-openvpn/gradle.properties
printf "org.gradle.jvmargs=-Xmx${MOSTMEM}m -XX:MaxPermSize=${QUARTERMEM}m -XX:MaxMetaspaceSize=${QUARTERMEM}m -XX:+HeapDumpOnOutOfMemoryError -Dfile.encoding=UTF-8" >> $HOME/android/local.properties
printf "org.gradle.jvmargs=-Xmx${MOSTMEM}m -XX:MaxPermSize=${QUARTERMEM}m -XX:MaxMetaspaceSize=${QUARTERMEM}m -XX:+HeapDumpOnOutOfMemoryError -Dfile.encoding=UTF-8" >> $HOME/android/gradle.properties
printf "org.gradle.jvmargs=-Xmx${MOSTMEM}m -XX:MaxPermSize=${QUARTERMEM}m -XX:MaxMetaspaceSize=${QUARTERMEM}m -XX:+HeapDumpOnOutOfMemoryError -Dfile.encoding=UTF-8" >> $HOME/android/ics-openvpn/gradle.properties

git update-index --assume-unchanged $HOME/android/ics-openvpn/main/build/ovpnassets/.empty
git update-index --assume-unchanged $HOME/android/app/src/test/resources/preconfigured/centos.local.json
git update-index --assume-unchanged $HOME/android/app/src/test/resources/preconfigured/centos.local.pem
git update-index --assume-unchanged $HOME/android/app/src/test/resources/preconfigured/debian.local.json
git update-index --assume-unchanged $HOME/android/app/src/test/resources/preconfigured/debian.local.pem

./scripts/build_deps.sh || \
  { RESULT=$? ; tput setaf 1 ; printf "Android dependencies failed to build. [ DEPS = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }
 
./gradlew --console plain --warning-mode none assembleLavabit || \
  { RESULT=$? ; tput setaf 1 ; printf "Android app packages failed to build. [ APKS = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }
 
./gradlew --console plain --warning-mode none bundleLavabit || \
  { RESULT=$? ; tput setaf 1 ; printf "Android app bundles failed to build. [ BUNDLES = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

# Test the secure and insecure fat debug, and a secure fatweb debug builds.
./gradlew --console plain --warning-mode none \
  testLavabitInsecureFatDebugUnitTest \
  testLavabitProductionFatDebugUnitTest \
  testLavabitProductionFatwebDebugUnitTest || \
  { RESULT=$? ; tput setaf 1 ; printf "Android unit tests failed. [ DEBUG = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

# Test the beta builds, first the ABI variants, then the combined fat version.
./gradlew --console plain --warning-mode none \
  testLavabitProductionX86BetaUnitTest testLavabitProductionX86_64BetaUnitTest \
  testLavabitProductionArmv7BetaUnitTest testLavabitProductionArm64BetaUnitTest \
  testLavabitProductionFatBetaUnitTest || \
  { RESULT=$? ; tput setaf 1 ; printf "Android unit tests failed. [ BETA = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

# Finally, test the release builds, first the ABI variants, then the fat and fatweb versions.
./gradlew --console plain --warning-mode none \
  testLavabitProductionX86ReleaseUnitTest testLavabitProductionX86_64ReleaseUnitTest \
  testLavabitProductionArmv7ReleaseUnitTest testLavabitProductionArm64ReleaseUnitTest \
  testLavabitProductionFatReleaseUnitTest testLavabitProductionFatwebReleaseUnitTest || \
  { RESULT=$? ; tput setaf 1 ; printf "Android unit tests failed. [ RELEASE = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

# Build the Bitmask Fatweb variant for comparison testing.
PAHOEHOE_NORMAL=$(echo $PAHOEHOE_NORMAL | tr "[:lower:]" "[:upper:]")
if [ "$PAHOEHOE_NORMAL" == "YES" ]; then
  ./gradlew --warning-mode none assembleNormalInsecureFat assembleNormalProductionFat || \
    { RESULT=$? ; tput setaf 1 ; printf "Android bitmask app packages failed to build. [ APKS = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

  ./gradlew --warning-mode none bundleNormalInsecureFat bundleNormalProductionFat || \
    { RESULT=$? ; tput setaf 1 ; printf "Android bitmask app bundles failed to build. [ BUNDLES = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

  ./gradlew --warning-mode none testNormalInsecureFatDebugUnitTest testNormalProductionFatReleaseUnitTest || \
    { RESULT=$? ; tput setaf 1 ; printf "Android bitmask unit tests failed. [ FAT = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }
fi

# The lint checks provide too much info at the moment so don't run them for every CI build.
PAHOEHOE_LINT=$(echo $PAHOEHOE_LINT | tr "[:lower:]" "[:upper:]")
if [ "$PAHOEHOE_LINT" == "YES" ]; then
  ./gradlew --console plain --warning-mode none check 2>&1 | \
  grep -v "without required default value." | grep -v "multiple substitutions specified in non-positional format" | \
  grep -v "Make sure all annotation processors are incremental to improve your build speed." | \
  grep -v "The following annotation processors are not incremental: jetified-dagger-compiler-1.2.2.jar" | \
  grep -v "Some input files use or override a deprecated API." | grep -v "Some input files use unchecked or unsafe operations." | \
  grep -v "Recompile with -Xlint:unchecked for details."
fi

echo "All finished."
sudo fstrim --all


