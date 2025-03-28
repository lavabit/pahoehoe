#!/bin/bash

# clear; vagrant destroy -f ; time ./run.sh ; for i in {1..10}; do printf  "\a" ; sleep 1 ; done


# git clone https://github.com/vagrant-libvirt/vagrant-libvirt.git && \
# cd vagrant-libvirt && \
# git checkout 0.12.2 && \
# sed -i "s/'fog-libvirt', '>= 0.6.0'/'fog-libvirt', '<= 0.12.2'/g" vagrant-libvirt.gemspec && \
# /opt/vagrant/embedded/bin/gem build vagrant-libvirt.gemspec && \
# vagrant plugin install vagrant-libvirt-0.12.2.gem && \
# cd ..

# If the version string/numbers aren't setup already, we declare them here. The version 
# string is what defines this build. The version number will be used as the version code
# for F-Droid builds. The APK files uploaded to Google Play will use the build.gradle
# logic to generate unique version numbers (aka codes) for each ABI variant.

# The values used here should match the android/app/build.gradle file. Specifically, the 
# android -> lavabit -> versionName and android -> lavabit -> versionCode values. 
[ ! -n "$VERNUM" ] && export VERNUM="203"
[ ! -n "$VERSTR" ] && export VERSTR="1.0.3-RC"

# Set DEBUG to "yes" to increase the script output verbosity.
[ ! -n "$DEBUG" ] || [ "$DEBUG" != "yes" ] && export DEBUG="no"

# Set KEEPGOING to "yes" to continue running the shell scripts even if one fails.
[ ! -n "$KEEPGOING" ] || [ "$KEEPGOING" != "yes" ] && export KEEPGOING="no"

# Set libvirt as the default provider, but allow an environment variable to override it.
[ ! -n "$PROVIDER" ] && export PROVIDER="libvirt"

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

# By default we abort if an error occurs, but enabling debug will output each command, making 
# it easier to see where the failure occurred.
if [ "$DEBUG" == "yes" ] && [ "$KEEPGOING" == "no" ]; then
  PAHOEHOE_SHELL_OPTS=" -ex "
  # set -x
  set -
elif [ "$DEBUG" == "yes" ] && [ "$KEEPGOING" == "yes" ]; then
  PAHOEHOE_SHELL_OPTS=" -x "
  # set -x
  set -
elif [ "$DEBUG" == "no" ] && [ "$KEEPGOING" == "no" ]; then
  PAHOEHOE_SHELL_OPTS=" -e "
  set -
elif [ "$DEBUG" == "no" ] && [ "$KEEPGOING" == "yes" ]; then
  PAHOEHOE_SHELL_OPTS=" - "
  set -

# All the possible combos should be handled above, but just 
fi

# Unset the DEBUG/KEEPGOING variables to avoid issues with others scripts/tools.
unset DEBUG ; unset KEEPGOING

# Cleanup.
[ -d $BASE/build/source/ ] && sudo umount --force $BASE/build/source/ &>/dev/null || true
[ -d $BASE/build/source/ ] && rmdir $BASE/build/source/ &>/dev/null || true
[ -d $BASE/build/ ] && rm --force --recursive $BASE/build/ || true
vagrant destroy -f &>/dev/null || true

# Create a build directory, and a log sub-directory.
[ ! -d $BASE/build/ ] && mkdir $BASE/build/
[ ! -d $BASE/build/logs/ ] && mkdir $BASE/build/logs/

# Try and catch any problems with the download URLs early, so we don't waste time.
if [ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/archive/com.termux_75.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/com.kgurgul.cpuinfo_60300.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/com.lavabit.pahoehoe_202.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/com.termux.api_51.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/com.termux.boot_7.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/com.termux.styling_31.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/com.termux.widget_13.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/com.termux_118.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/de.blinkt.openvpn_210.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://f-droid.org/repo/org.connectbot_10909000.apk)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://github.com/google/bundletool/releases/download/1.5.0/bundletool-all-1.5.0.jar)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/android/maven2/com/android/tools/build/aapt2/4.1.3-6503028/aapt2-4.1.3-6503028-linux.jar)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/android/repository/android-ndk-r16b-linux-x86_64.zip)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/android/repository/commandlinetools-linux-8092744_latest.zip)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://dl.google.com/android/repository/commandlinetools-linux-8092744_latest.zip)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/android/repository/commandlinetools-linux-11076708_latest.zip)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/android/repository/sdk-tools-linux-3859397.zip)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/android/studio/ide-zips/4.1.2.0/android-studio-ide-201.7042882-linux.tar.gz)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/android/studio/ide-zips/2024.3.1.13/android-studio-2024.3.1.13-linux.tar.gz)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://dl.google.com/android/studio/ide-zips/2024.3.1.13/android-studio-2024.3.1.13-linux.tar.gz)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/go/getgo/installer.exe)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/go/getgo/installer_darwin)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/go/getgo/installer_linux)" != "200" ] || \
[ "$(curl --fail --head --silent --location --max-time 60 --output /dev/null --write-out '%{http_code}' https://redirector.gvt1.com/edgedl/go/go1.14.2.linux-amd64.tar.gz)" != "200" ]; then
printf "\n\e[1;91m# One of the download URLs this build relies upon is invalid.\e[0;0m\n\n" ;
exit 1
fi

# Try and update the box. If we already have the current version, proceed. Otherwise
# download the latest box file. If the box is missing, or the download fails, we 
# use the add command to download it, and we try that three times, which will hopefully
# reduce the number of CI failures caused by temporal cloud issues.
vagrant box update --provider $PROVIDER --box "generic/alma9" &> "$BASE/build/logs/vagrant_box_add.txt" || \
  { vagrant box add --clean --force --provider $PROVIDER "generic/alma9" &> "$BASE/build/logs/vagrant_box_add.txt" ; } || \
  { sleep 120 ; vagrant box add --clean --force --provider $PROVIDER "generic/alma9" &>> "$BASE/build/logs/vagrant_box_add.txt" ; } || \
  { sleep 180 ; vagrant box add --clean --force --provider $PROVIDER "generic/alma9" &>> "$BASE/build/logs/vagrant_box_add.txt" ; } || \
  { tput setaf 1 ; printf "\nBox download failed. [ BOX = generic/alma9 ]\n\n" ; tput sgr0 ; \
  vagrant box remove --force --all --provider $PROVIDER "generic/alma9" &> /dev/null ; exit 1 ; }

vagrant box update --provider $PROVIDER --box "generic/debian10" &> "$BASE/build/logs/vagrant_box_add.txt" || \
  { vagrant box add --clean --force --provider $PROVIDER "generic/debian10" &>> "$BASE/build/logs/vagrant_box_add.txt" ; } || \
  { sleep 120 ; vagrant box add --clean --force --provider $PROVIDER "generic/debian10" &>> "$BASE/build/logs/vagrant_box_add.txt" ; } || \
  { sleep 180 ; vagrant box add --clean --force --provider $PROVIDER "generic/debian10" &>> "$BASE/build/logs/vagrant_box_add.txt" ; } || \
  { tput setaf 1 ; printf "\nBox download failed. [ BOX = generic/debian10 ]\n\n" ; tput sgr0 ; \
  vagrant box remove --force --all --provider $PROVIDER "generic/debian10" &> /dev/null ; exit 1 ; }

printf "\nBox download complete.\n"

# Create the virtual machines.
vagrant up --provider=$PROVIDER &> "$BASE/build/logs/vagrant_up.txt" && BOOTED=1 || \
  { RESULT=$? ; tput setaf 1 ; printf "Box startup error. Retrying. [ UP = $RESULT ]\n\n" ; tput sgr0 ; \
vagrant destroy -f ; sleep 120 ; vagrant up --provider=$PROVIDER &>> "$BASE/build/logs/vagrant_up.txt" && BOOTED=1 ; } || \
  { RESULT=$? ; tput setaf 1 ; printf "Box startup error. Retrying. [ UP = $RESULT ]\n\n" ; tput sgr0 ; \
vagrant destroy -f ; sleep 120 ; vagrant up --provider=$PROVIDER &>> "$BASE/build/logs/vagrant_up.txt" && BOOTED=1 ; } || \
  { RESULT=$? ; tput setaf 1 ; printf "Box startup error. Exiting. [ UP = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

printf "Box startup complete.\n"

# Any errors past this point would be critical failures.
set ${PAHOEHOE_SHELL_OPTS}

# Upload the scripts.
vagrant upload alma-9-vpnweb.sh vpnweb.sh alma_vpn &>> "$BASE/build/logs/vagrant_setup.txt"
vagrant upload alma-9-openvpn.sh openvpn.sh alma_vpn &>> "$BASE/build/logs/vagrant_setup.txt"
vagrant upload debian-10-vpnweb.sh vpnweb.sh debian_vpn &>> "$BASE/build/logs/vagrant_setup.txt"
vagrant upload debian-10-openvpn.sh openvpn.sh debian_vpn &>> "$BASE/build/logs/vagrant_setup.txt"

vagrant upload debian-10-build-setup.sh setup.sh debian_build &>> "$BASE/build/logs/vagrant_setup.txt"
vagrant upload debian-10-rebuild.sh rebuild.sh debian_build &>> "$BASE/build/logs/vagrant_setup.txt"
vagrant upload debian-10-build.sh build.sh debian_build &>> "$BASE/build/logs/vagrant_setup.txt"

[ -f debian-10-build-key.sh ] && vagrant upload debian-10-build-key.sh key.sh debian_build &>> "$BASE/build/logs/vagrant_setup.txt"

vagrant ssh -c 'chmod +x vpnweb.sh openvpn.sh' alma_vpn &>> "$BASE/build/logs/vagrant_setup.txt"
vagrant ssh -c 'chmod +x vpnweb.sh openvpn.sh' debian_vpn &>> "$BASE/build/logs/vagrant_setup.txt"
vagrant ssh -c 'chmod +x setup.sh build.sh rebuild.sh' debian_build &>> "$BASE/build/logs/vagrant_setup.txt"

[ -f debian-10-build-key.sh ] && vagrant ssh -c 'chmod +x key.sh' debian_build &>> "$BASE/build/logs/vagrant_setup.txt"

# Provision the VPN service.
vagrant ssh --no-tty -c "sudo --login TZ=$TZ TERM=$TERM bash ${PAHOEHOE_SHELL_OPTS} < vpnweb.sh" alma_vpn &> "$BASE/build/logs/alma_vpn.txt" || \
 { RESULT=$? ; tput setaf 1 ; printf "Alma VPN error. [ VPNWEB = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }
vagrant ssh --no-tty -c "sudo --login TZ=$TZ TERM=$TERM bash ${PAHOEHOE_SHELL_OPTS} < openvpn.sh" alma_vpn &>> "$BASE/build/logs/alma_vpn.txt" || \
 { RESULT=$? ; tput setaf 1 ; printf "Alma VPN error. [ OPENVPN = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

printf "Alma VPN stage complete.\n"

vagrant ssh --no-tty -c "sudo --login TZ=$TZ TERM=$TERM bash ${PAHOEHOE_SHELL_OPTS} < vpnweb.sh" debian_vpn &> "$BASE/build/logs/debian_vpn.txt" || \
  { RESULT=$? ; tput setaf 1 ; printf "Debian VPN error. [ VPNWEB = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }
vagrant ssh --no-tty -c "sudo --login TZ=$TZ TERM=$TERM bash ${PAHOEHOE_SHELL_OPTS} < openvpn.sh" debian_vpn &>> "$BASE/build/logs/debian_vpn.txt" || \
  { RESULT=$? ; tput setaf 1 ; printf "Debian VPN error. [ OPENVPN = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }
 
printf "Debian VPN stage complete.\n"

# Compile the Android client.
vagrant ssh --no-tty -c "TZ=$TZ TERM=$TERM bash ${PAHOEHOE_SHELL_OPTS} setup.sh" debian_build &> "$BASE/build/logs/debian_build.txt" || \
  { RESULT=$? ; tput setaf 1 ; printf "Android build setup error. [ SETUP = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

printf "Android build setup complete.\n"
 
vagrant ssh --no-tty -c "TZ=$TZ TERM=$TERM VERNUM=$VERNUM VERSTR=$VERSTR bash ${PAHOEHOE_SHELL_OPTS} build.sh" debian_build &>> "$BASE/build/logs/debian_build.txt" || \
  { RESULT=$? ; tput setaf 1 ; printf "Android build stage error. [ BUILD = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }

printf "Android build stage complete.\n"

# Save an SSH config file so we can extract the build artifacts.
vagrant ssh-config debian_build > $BASE/build/config

# If there is an output folder, extract the development APKs from the build environment.
vagrant ssh -c "test -d \$HOME/android/app/build/outputs/" debian_build &> /dev/null && {
  [ ! -d $BASE/build/outputs/ ] && mkdir $BASE/build/outputs/
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitInsecureFat/debug/Lavabit_Proxy_debug_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_insecure_debug_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionFat/debug/Lavabit_Proxy_debug_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_debug_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionFatweb/debug/Lavabit_Proxy_web_debug_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_web_debug_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionX86/beta/Lavabit_Proxy_x86_beta_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_x86_beta_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionX86_64/beta/Lavabit_Proxy_x86_64_beta_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_x86_64_beta_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionArmv7/beta/Lavabit_Proxy_armeabi-v7a_beta_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_armeabi-v7a_beta_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionArm64/beta/Lavabit_Proxy_arm64-v8a_beta_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_arm64-v8a_beta_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionFat/beta/Lavabit_Proxy_beta_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_beta_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionX86/release/Lavabit_Proxy_x86_release_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_x86_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionX86_64/release/Lavabit_Proxy_x86_64_release_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_x86_64_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionArmv7/release/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionArm64/release/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionFat/release/Lavabit_Proxy_release_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/app/build/outputs/apk/lavabitProductionFatweb/release/Lavabit_Proxy_web_release_$VERSTR.apk $BASE/build/outputs/Lavabit_Proxy_web_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" || \
  { RESULT=$? ; tput setaf 1 ; printf "Build output download error. [ OUTPUTS = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }
  
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_insecure_debug_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_insecure_debug_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_debug_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_debug_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_web_debug_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_web_debug_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_x86_beta_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_x86_beta_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_x86_64_beta_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_x86_64_beta_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_armeabi-v7a_beta_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_armeabi-v7a_beta_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_arm64-v8a_beta_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_arm64-v8a_beta_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_beta_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_beta_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_x86_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_x86_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_x86_64_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_x86_64_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/outputs/Lavabit_Proxy_web_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A build output is missing. [ FILE = Lavabit_Proxy_web_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }  
}

# If there is a releases folder, extract the signed release APKs files from the build environment.
vagrant ssh -c "test -d \$HOME/android/releases/" debian_build &> /dev/null && {
  [ ! -d $BASE/build/releases/ ] && mkdir $BASE/build/releases/
  printf "get /home/vagrant/android/releases/SHA256SUMS $BASE/build/releases/SHA256SUMS\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_release_$VERSTR.apk $BASE/build/releases/Lavabit_Proxy_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_release_$VERSTR.apk.sig $BASE/build/releases/Lavabit_Proxy_release_$VERSTR.apk.sig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_release_$VERSTR.apk.idsig $BASE/build/releases/Lavabit_Proxy_release_$VERSTR.apk.idsig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_release_$VERSTR.apk.version $BASE/build/releases/Lavabit_Proxy_release_$VERSTR.apk.version\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_web_release_$VERSTR.apk $BASE/build/releases/Lavabit_Proxy_web_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_web_release_$VERSTR.apk.sig $BASE/build/releases/Lavabit_Proxy_web_release_$VERSTR.apk.sig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_web_release_$VERSTR.apk.idsig $BASE/build/releases/Lavabit_Proxy_web_release_$VERSTR.apk.idsig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_web_release_$VERSTR.apk.version $BASE/build/releases/Lavabit_Proxy_web_release_$VERSTR.apk.version\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_x86_release_$VERSTR.apk $BASE/build/releases/Lavabit_Proxy_x86_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.sig $BASE/build/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.sig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.idsig $BASE/build/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.idsig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.version $BASE/build/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.version\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk $BASE/build/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.sig $BASE/build/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.sig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.idsig $BASE/build/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.idsig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.version $BASE/build/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.version\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk $BASE/build/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.sig $BASE/build/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.sig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.idsig $BASE/build/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.idsig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.version $BASE/build/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.version\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk $BASE/build/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.sig $BASE/build/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.sig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.idsig $BASE/build/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.idsig\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" && \
  printf "get /home/vagrant/android/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.version $BASE/build/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.version\n" | sftp -F $BASE/build/config debian_build &>> "$BASE/build/logs/debian_build.txt" || \
  { RESULT=$? ; tput setaf 1 ; printf "Release download error. [ RELEASES = $RESULT ]\n\n" ; tput sgr0 ; exit 1 ; }
  
  [ ! -f $BASE/build/releases/SHA256SUMS ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = $BASE/build/releases/SHA256SUMS ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_release_$VERSTR.apk.sig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_release_$VERSTR.apk.sig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_release_$VERSTR.apk.idsig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_release_$VERSTR.apk.idsig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_release_$VERSTR.apk.version ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_release_$VERSTR.apk.version ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_web_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_web_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_web_release_$VERSTR.apk.sig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_web_release_$VERSTR.apk.sig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_web_release_$VERSTR.apk.idsig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_web_release_$VERSTR.apk.idsig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_web_release_$VERSTR.apk.version ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_web_release_$VERSTR.apk.version ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_x86_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_x86_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.sig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_x86_release_$VERSTR.apk.sig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.idsig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_x86_release_$VERSTR.apk.idsig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_x86_release_$VERSTR.apk.version ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_x86_release_$VERSTR.apk.version ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_x86_64_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.sig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_x86_64_release_$VERSTR.apk.sig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.idsig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_x86_64_release_$VERSTR.apk.idsig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_x86_64_release_$VERSTR.apk.version ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_x86_64_release_$VERSTR.apk.version ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.sig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.sig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.idsig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.idsig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.version ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_armeabi-v7a_release_$VERSTR.apk.version ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.sig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.sig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.idsig ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.idsig ]\n\n" ; tput sgr0 ; exit 1 ; }
  [ ! -f $BASE/build/releases/Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.version ] && { tput setaf 1 ; printf "A release output is missing. [ FILE = Lavabit_Proxy_arm64-v8a_release_$VERSTR.apk.version ]\n\n" ; tput sgr0 ; exit 1 ; }
}


# Download the various F-Droid packages we use during testing/development. Use the f-droid-packages.sh script to generate an updated
# list package files/hashes. 

# To download the F-Droid index and lookup package metadata manually use the following commands:
# curl -Lfso index-v1.jar https://f-droid.org/repo/index-v1.jar
# unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "%%%PACKAGENAME%%%" )] ' \
# ' | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv'

# To update the section below:
# bash f-droid-packages.sh | ( copyq copy - | clipit | gpaste )

# To sanity check the package script output run (it counts the f-droid.org download URLs to ensure nothing was skipped):
# [ "$(bash f-droid-packages.sh | grep -E "^curl .* https:\/\/f\-droid.org\/" | wc -l)" == "10" ] && echo "F-Droid package script sanity check passed." || echo "F-Droid package script sanity check failed."

### BEGIN F-DROID-PACKAGES.SH

# Termux version 118 requires at least v24 of the Android SDK.
[ -d $BASE/build/termux/ ] && rm --force --recursive $BASE/build/termux/ ; mkdir --parents $BASE/build/termux/
curl --fail --silent --show-error --location --output $BASE/build/termux/com.termux_118.apk https://f-droid.org/repo/com.termux_118.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.termux_118.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "822ac152bd7c2d9770b87c1feea03f22f2349a91b94481b268c739493a260f0b  $BASE/build/termux/com.termux_118.apk" | sha256sum -c --quiet || exit 1

curl --fail --silent --show-error --location --output $BASE/build/termux/com.termux.api_51.apk https://f-droid.org/repo/com.termux.api_51.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.termux.api_51.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "781ff805619b104115fbf15499414715b4ea6ceb93c4935086a7e35966024f20  $BASE/build/termux/com.termux.api_51.apk" | sha256sum -c --quiet || exit 1

curl --fail --silent --show-error --location --output $BASE/build/termux/com.termux.boot_7.apk https://f-droid.org/repo/com.termux.boot_7.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.termux.boot_7.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "35cae49192d073151e3177956ea4f1d6309c2330fed42ec046cbb44cee072a32  $BASE/build/termux/com.termux.boot_7.apk" | sha256sum -c --quiet || exit 1

curl --fail --silent --show-error --location --output $BASE/build/termux/com.termux.widget_13.apk https://f-droid.org/repo/com.termux.widget_13.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.termux.widget_13.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "7ec99c3bd53e1fb8737f688bc26fdd0ae931f1f2f7eb9c855de1a0e4eb6147ae  $BASE/build/termux/com.termux.widget_13.apk" | sha256sum -c --quiet || exit 1

curl --fail --silent --show-error --location --output $BASE/build/termux/com.termux.styling_31.apk https://f-droid.org/repo/com.termux.styling_31.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.termux.styling_31.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "66abbb79fbffb5dfc38d9db3da88b8a0d31a9e6f7ffe8c68b5b448cda2db3b59  $BASE/build/termux/com.termux.styling_31.apk" | sha256sum -c --quiet || exit 1

# Download ConnectBot, which will work on devices with Android SDK v14 and higher..
[ -d $BASE/build/connectbot/ ] && rm --force --recursive $BASE/build/connectbot/ ; mkdir --parents $BASE/build/connectbot/
curl --fail --silent --show-error --location --output $BASE/build/connectbot/org.connectbot_10909000.apk https://f-droid.org/repo/org.connectbot_10909000.apk || 
{ tput setaf 1 ; printf "An APK download failed. [ FILE = org.connectbot_10909000.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "d5e0bc3f3be1702eb5822abc3d62469c0440cddc6366126b6cc1cfbaf8b0814f  $BASE/build/connectbot/org.connectbot_10909000.apk" | sha256sum -c --quiet || exit 1

# Download the OpenVPN Android GUI
[ -d $BASE/build/openvpn/ ] && rm --force --recursive $BASE/build/openvpn/ ; mkdir --parents $BASE/build/openvpn/
curl --fail --silent --show-error --location --output $BASE/build/openvpn/de.blinkt.openvpn_204.apk https://f-droid.org/repo/de.blinkt.openvpn_210.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = de.blinkt.openvpn_204.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "2d68d0dcb3de86d9bdb56349331b1fa2e8d7a647a160578444f901da69a74ba9  $BASE/build/openvpn/de.blinkt.openvpn_204.apk" | sha256sum -c --quiet || exit 1

# Download the currently released Lavabit App
[ -d $BASE/build/lavabit/ ] && rm --force --recursive $BASE/build/lavabit/ ; mkdir --parents $BASE/build/lavabit/
curl --fail --silent --show-error --location --output $BASE/build/lavabit/com.lavabit.pahoehoe_202.apk https://f-droid.org/repo/com.lavabit.pahoehoe_202.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.lavabit.pahoehoe_202.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "700d0cdc53eea321cfb2da25ca01707ba9a508d35419ea0877d520148a6fb906  $BASE/build/lavabit/com.lavabit.pahoehoe_202.apk" | sha256sum -c --quiet || exit 1

# Download CPU Info App
[ -d $BASE/build/cpuinfo/ ] && rm --force --recursive $BASE/build/cpuinfo/ ; mkdir --parents $BASE/build/cpuinfo/
curl --fail --silent --show-error --location --output $BASE/build/cpuinfo/com.kgurgul.cpuinfo_40700.apk https://f-droid.org/repo/com.kgurgul.cpuinfo_60300.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.kgurgul.cpuinfo_40700.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "cc826cc5f9284caf338f211788f55914bcb4f64c82f8bc4911275d5ed962d17b  $BASE/build/cpuinfo/com.kgurgul.cpuinfo_40700.apk" | sha256sum -c --quiet || exit 1

# Termux version 75 requires at least v21 of the Android SDK.
curl --fail --silent --show-error --location --output $BASE/build/termux/com.termux_75.apk https://f-droid.org/archive/com.termux_75.apk || \
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.termux_75.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "d88444d9df4049c47f12678feb9579aaf2814a89e411d52653dc0a2509f883b5  $BASE/build/termux/com.termux_75.apk" | sha256sum -c --quiet || exit 1

### END F-DROID-PACKAGES.SH

# [ ! -d $BASE/build/source/ ] && mkdir $BASE/build/source/
# sshfs vagrant@192.168.221.50:/home/vagrant/android $BASE/build/source -o uidfile=1000 -o gidfile=1000 \
# -o StrictHostKeyChecking=no -o IdentityFile=$BASE/.vagrant/machines/debian_build/libvirt/private_key

printf "Android download stage complete.\n"
