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

# Cleanup.
[ -d $BASE/build/source/ ] && sudo umount --force $BASE/build/source/ &>/dev/null
[ -d $BASE/build/source/ ] && rmdir $BASE/build/source/ &>/dev/null
[ -d $BASE/build/ ] && rm -rf $BASE/build/

set -
vagrant destroy -f &>/dev/null

# # Create virtual machines..
set -e
vagrant up --provider=libvirt

# Upload the scripts.
vagrant upload centos-8-vpnweb.sh vpnweb.sh centos_vpn &> /dev/null
vagrant upload centos-8-openvpn.sh openvpn.sh centos_vpn &> /dev/null
vagrant upload debian-10-vpnweb.sh vpnweb.sh debian_vpn &> /dev/null
vagrant upload debian-10-openvpn.sh openvpn.sh debian_vpn &> /dev/null

vagrant upload debian-10-build-setup.sh setup.sh debian_build &> /dev/null
vagrant upload debian-10-rebuild.sh rebuild.sh debian_build &> /dev/null
vagrant upload debian-10-build.sh build.sh debian_build &> /dev/null

[ -f debian-10-build-key.sh ] && vagrant upload debian-10-build-key.sh key.sh debian_build &> /dev/null

vagrant ssh -c 'chmod +x vpnweb.sh openvpn.sh' centos_vpn &> /dev/null
vagrant ssh -c 'chmod +x vpnweb.sh openvpn.sh' debian_vpn &> /dev/null
vagrant ssh -c 'chmod +x setup.sh build.sh rebuild.sh' debian_build &> /dev/null

[ -f debian-10-build-key.sh ] && vagrant ssh -c 'chmod +x key.sh' debian_build &> /dev/null

# Provision the VPN service.
vagrant ssh --tty -c 'sudo --login bash -e < vpnweb.sh' centos_vpn
vagrant ssh --tty -c 'sudo --login bash -e < openvpn.sh' centos_vpn
vagrant ssh --tty -c 'sudo --login bash -e < vpnweb.sh' debian_vpn
vagrant ssh --tty -c 'sudo --login bash -e < openvpn.sh' debian_vpn

# Compile the Android client.
vagrant ssh --tty -c 'bash -e setup.sh' debian_build
vagrant ssh --tty -c 'bash -e build.sh' debian_build

# Extract the Android APKs from the build environment.
[ -d $BASE/build/ ] && rm --force --recursive $BASE/build/ ; mkdir $BASE/build/
vagrant ssh-config debian_build > $BASE/build/config
printf "cd /home/vagrant/android/app/build/\nlcd $BASE/build/\nget -r outputs\n" | sftp -q -F $BASE/build/config debian_build

# If there is a releases folder, we want to download that as well, otherwise we skip this step.
vagrant ssh -c "test ! -d \$HOME/android/releases/" debian_build &> /dev/null && printf "cd /home/vagrant/android/\nlcd $BASE/build/\nget -r releases\n" | sftp -q -F $BASE/build/config debian_build

# Download Termux
[ -d $BASE/build/termux/ ] && rm --force --recursive $BASE/build/termux/ ; mkdir --parents $BASE/build/termux/
curl --silent --location --output $BASE/build/termux/com.termux_108.apk https://f-droid.org/repo/com.termux_108.apk
printf "0aa703cf809604cf39d7bf7613cdb96fe4c4a91814aeb67aa91aa3763ecda31f  $BASE/build/termux/com.termux_108.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/build/termux/com.termux.api_47.apk https://f-droid.org/repo/com.termux.api_47.apk
printf "086b8d7f098cee431bfac615213eae2e2cbb44f6f2543ee38a12e0f36b3098f8  $BASE/build/termux/com.termux.api_47.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/build/termux/com.termux.boot_7.apk https://f-droid.org/repo/com.termux.boot_7.apk
printf "35cae49192d073151e3177956ea4f1d6309c2330fed42ec046cbb44cee072a32  $BASE/build/termux/com.termux.boot_7.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/build/termux/com.termux.widget_11.apk https://f-droid.org/repo/com.termux.widget_11.apk
printf "934cfb004993348d207ad3e21928e94fba07cb8185ba292ab5209eab09c15dcc  $BASE/build/termux/com.termux.widget_11.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/build/termux/com.termux.styling_29.apk https://f-droid.org/repo/com.termux.styling_29.apk
printf "77bafdb6c4374de0cdabe68f103aca37ef7b81e18272ea663bb9842c82920bec  $BASE/build/termux/com.termux.styling_29.apk" | sha256sum -c || exit 1

# Download ConnectBot
[ -d $BASE/build/connectbot/ ] && rm --force --recursive $BASE/build/connectbot/ ; mkdir --parents $BASE/build/connectbot/
curl --silent --location --output $BASE/build/connectbot/org.connectbot_10906000.apk https://f-droid.org/repo/org.connectbot_10906000.apk
printf "7b8fb2aa289c99eaa22bcf267c650609660173b244f83636603b2acf231fa25b  $BASE/build/connectbot/org.connectbot_10906000.apk" | sha256sum -c || exit 1

# Download the OpenVPN Android GUI
[ -d $BASE/build/openvpn/ ] && rm --force --recursive $BASE/build/openvpn/ ; mkdir --parents $BASE/build/openvpn/
curl --silent --location --output $BASE/build/openvpn/de.blinkt.openvpn_175.apk https://f-droid.org/repo/de.blinkt.openvpn_175.apk
printf "359dd465e81c796c98f9cb4deb493956715c5834ebf31b643f366dcf6a713037  $BASE/build/openvpn/de.blinkt.openvpn_175.apk" | sha256sum -c || exit 1

# Download CPU Info App
[ -d $BASE/build/cpuinfo/ ] && rm --force --recursive $BASE/build/cpuinfo/ ; mkdir --parents $BASE/build/cpuinfo/
curl --silent --location --output $BASE/build/cpuinfo/com.kgurgul.cpuinfo_40403.apk https://f-droid.org/repo/com.kgurgul.cpuinfo_40403.apk
printf "6c0f6bff9ffd3416ab66b705dca0cff6f02af2ca153a1922b91c2287b8516fc0  $BASE/build/cpuinfo/com.kgurgul.cpuinfo_40403.apk" | sha256sum -c || exit 1

[ ! -d $BASE/build/source/ ] && mkdir $BASE/build/source/
sshfs vagrant@192.168.221.50:/home/vagrant/android $BASE/build/source -o uidfile=1000 -o gidfile=1000 \
-o StrictHostKeyChecking=no -o IdentityFile=$BASE/.vagrant/machines/debian_build/libvirt/private_key

tput setaf 2; printf "Lavabit proxy build process complete.\n"; tput sgr0
