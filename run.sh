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
# git diff -U0 -w --no-color FILE

# Enable NFSv3 over UDP.
# [ "`nfsconf --get nfsd udp`" != "y" ] && sudo nfsconf --set nfsd udp y

#sudo firewall-cmd --quiet --zone=libvirt --query-service=nfs || sudo firewall-cmd --zone=libvirt --add-service=nfs
#sudo firewall-cmd --quiet --zone=libvirt --query-service=nfs3 || sudo firewall-cmd --zone=libvirt --add-service=nfs3
# sudo firewall-cmd --quiet --zone=libvirt --query-service=mountd || sudo firewall-cmd --zone=libvirt --add-service=mountd
# sudo firewall-cmd --quiet --zone=libvirt --query-service=rpc-bind || sudo firewall-cmd --zone=libvirt --add-service=rpc-bind

# Cleanup.
[ -d $BASE/android/ ] && sudo umount --force $BASE/android/ &>/dev/null
[ -d $BASE/android/ ] && rmdir $BASE/android/ &>/dev/null
[ -d $BASE/apk/ ] && rm -rf $BASE/apk/

set -
vagrant destroy -f &>/dev/null

# # Create virtual machines..
set -e
vagrant up --provider=libvirt

# Disable error checking, since the commands below might fail, if your not using libvirt (for example).
# If you aren't using libvirt, you can comment out the following lines, till you
# see us enable error checking again.
set -

# if [ "`vagrant provider centos_vpn`" == "libvirt" ]; then
#   virsh --connect qemu:///system shutdown --domain proxy_centos_vpn &> /dev/null
#   set -
#   for i in {1..10}; do
#     [ "`virsh --connect qemu:///system domstate --domain proxy_centos_vpn | head -1`" == "shut off" ] && break
#     sleep 1
#   done
#   set -e
#   virt-xml --connect qemu:///system proxy_centos_vpn --quiet --edit --disk /var/lib/libvirt/images/proxy_centos_vpn.img,discard=unmap,detect_zeroes=unmap,cache=unsafe,io=threads,bus=scsi,target=sda,address.type=drive,address.controller=0,address.bus=0,address.target=0,address.unit=0 &> /dev/null
#   virt-xml --connect qemu:///system proxy_centos_vpn --quiet --edit scsi --controller type=scsi,model=virtio-scsi &> /dev/null
#   virsh --connect qemu:///system start --domain proxy_centos_vpn &> /dev/null
#   set -
#   for i in {1..10}; do
#     export STATUS="`vagrant ssh centos_vpn -c 'printf 1' -- -o='ConnectTimeout=1s' -o 'ForwardX11=no' 2> /dev/null | tail -1`"
#     [ -n "$STATUS" ] && [ "$STATUS" == "1" ] && break
#     sleep 1
#   done
#   set -e
# fi

if [ "`vagrant provider debian_vpn`" == "libvirt" ]; then
  virsh --connect qemu:///system shutdown --domain proxy_debian_vpn &> /dev/null
  set -
  for i in {1..10}; do
    [ "`virsh --connect qemu:///system domstate --domain proxy_debian_vpn | head -1`" == "shut off" ] && break
    sleep 1
  done
  set -e
  virt-xml --connect qemu:///system proxy_debian_vpn --quiet --edit --disk /var/lib/libvirt/images/proxy_debian_vpn.img,discard=unmap,detect_zeroes=unmap,cache=unsafe,io=threads,bus=scsi,target=sda,address.type=drive,address.controller=0,address.bus=0,address.target=0,address.unit=0 &> /dev/null
  virt-xml --connect qemu:///system proxy_debian_vpn --quiet --edit scsi --controller type=scsi,model=virtio-scsi &> /dev/null
  virsh --connect qemu:///system start --domain proxy_debian_vpn &> /dev/null
  set -
  for i in {1..10}; do
    export STATUS="`vagrant ssh debian_vpn -c 'printf 1' -- -o='ConnectTimeout=1s' -o 'ForwardX11=no' 2> /dev/null | tail -1`"
    [ -n "$STATUS" ] && [ "$STATUS" == "1" ] && break
    sleep 1
  done
  set -e
fi

if [ "`vagrant provider debian_build`" == "libvirt" ]; then
  virsh --connect qemu:///system shutdown --domain proxy_debian_build &> /dev/null
  set -
  for i in {1..10}; do
    [ "`virsh --connect qemu:///system domstate --domain proxy_debian_build | head -1`" == "shut off" ] && break
    sleep 1
  done
  set -e
  virt-xml --connect qemu:///system proxy_debian_build --quiet --edit --disk /var/lib/libvirt/images/proxy_debian_build.img,discard=unmap,detect_zeroes=unmap,cache=unsafe,io=threads,bus=scsi,target=sda,address.type=drive,address.controller=0,address.bus=0,address.target=0,address.unit=0 &> /dev/null
  virt-xml --connect qemu:///system proxy_debian_build --quiet --edit scsi --controller type=scsi,model=virtio-scsi &> /dev/null
  virsh --connect qemu:///system start --domain proxy_debian_build &> /dev/null
  set -
  for i in {1..10}; do
    export STATUS="`vagrant ssh debian_build -c 'printf 1' -- -o='ConnectTimeout=1s' -o 'ForwardX11=no' 2> /dev/null | tail -1`"
    [ -n "$STATUS" ] && [ "$STATUS" == "1" ] && break
    sleep 1
  done
  set -e
fi

# Enable error checking again.
set -e

# Upload the scripts.
vagrant upload centos-8-vpnweb.sh vpnweb.sh centos_vpn &> /dev/null
vagrant upload centos-8-openvpn.sh openvpn.sh centos_vpn &> /dev/null
vagrant upload debian-10-vpnweb.sh vpnweb.sh debian_vpn &> /dev/null
vagrant upload debian-10-openvpn.sh openvpn.sh debian_vpn &> /dev/null

vagrant upload debian-10-build-setup.sh setup.sh debian_build &> /dev/null
vagrant upload debian-10-rebuild.sh rebuild.sh debian_build &> /dev/null
vagrant upload debian-10-build.sh build.sh debian_build &> /dev/null

vagrant ssh -c 'chmod +x vpnweb.sh openvpn.sh' centos_vpn &> /dev/null
vagrant ssh -c 'chmod +x vpnweb.sh openvpn.sh' debian_vpn &> /dev/null
vagrant ssh -c 'chmod +x setup.sh build.sh rebuild.sh' debian_build &> /dev/null

# Provision the VPN service.
vagrant ssh --tty -c 'sudo --login bash -e < vpnweb.sh' centos_vpn
vagrant ssh --tty -c 'sudo --login bash -e < openvpn.sh' centos_vpn
vagrant ssh --tty -c 'sudo --login bash -e < vpnweb.sh' debian_vpn
vagrant ssh --tty -c 'sudo --login bash -e < openvpn.sh' debian_vpn

# Compile the Android client.
vagrant ssh --tty -c 'bash -ex setup.sh' debian_build
vagrant ssh --tty -c 'bash -ex build.sh' debian_build

# Extract the Android APKs from the build environment.
[ -d $BASE/apk/ ] && rm --force --recursive $BASE/apk/ ; mkdir $BASE/apk/
vagrant ssh-config debian_build > $BASE/apk/config
printf "cd /home/vagrant/bitmask_android_leap/app/build/outputs\nget -r apk\n" | sftp -q -F $BASE/apk/config debian_build

# Download Termux
[ -d $BASE/apk/termux/ ] && rm --force --recursive $BASE/apk/termux/ ; mkdir --parents $BASE/apk/termux/
curl --silent --location --output $BASE/apk/termux/com.termux_108.apk https://f-droid.org/repo/com.termux_108.apk
printf "0aa703cf809604cf39d7bf7613cdb96fe4c4a91814aeb67aa91aa3763ecda31f  $BASE/apk/termux/com.termux_108.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/apk/termux/com.termux.api_47.apk https://f-droid.org/repo/com.termux.api_47.apk
printf "086b8d7f098cee431bfac615213eae2e2cbb44f6f2543ee38a12e0f36b3098f8  $BASE/apk/termux/com.termux.api_47.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/apk/termux/com.termux.boot_7.apk https://f-droid.org/repo/com.termux.boot_7.apk
printf "35cae49192d073151e3177956ea4f1d6309c2330fed42ec046cbb44cee072a32  $BASE/apk/termux/com.termux.boot_7.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/apk/termux/com.termux.widget_11.apk https://f-droid.org/repo/com.termux.widget_11.apk
printf "934cfb004993348d207ad3e21928e94fba07cb8185ba292ab5209eab09c15dcc  $BASE/apk/termux/com.termux.widget_11.apk" | sha256sum -c || exit 1

curl --silent --location --output $BASE/apk/termux/com.termux.styling_29.apk https://f-droid.org/repo/com.termux.styling_29.apk
printf "77bafdb6c4374de0cdabe68f103aca37ef7b81e18272ea663bb9842c82920bec  $BASE/apk/termux/com.termux.styling_29.apk" | sha256sum -c || exit 1

# Download ConnectBot
[ -d $BASE/apk/connectbot/ ] && rm --force --recursive $BASE/apk/connectbot/ ; mkdir --parents $BASE/apk/connectbot/
curl --silent --location --output $BASE/apk/connectbot/org.connectbot_10906000.apk https://f-droid.org/repo/org.connectbot_10906000.apk
printf "7b8fb2aa289c99eaa22bcf267c650609660173b244f83636603b2acf231fa25b  $BASE/apk/connectbot/org.connectbot_10906000.apk" | sha256sum -c || exit 1

# Download the OpenVPN Android GUI
[ -d $BASE/apk/openvpn/ ] && rm --force --recursive $BASE/apk/openvpn/ ; mkdir --parents $BASE/apk/openvpn/
curl --silent --location --output $BASE/apk/openvpn/de.blinkt.openvpn_175.apk https://f-droid.org/repo/de.blinkt.openvpn_175.apk
printf "359dd465e81c796c98f9cb4deb493956715c5834ebf31b643f366dcf6a713037  $BASE/apk/openvpn/de.blinkt.openvpn_175.apk" | sha256sum -c || exit 1

# Download CPU Info App
[ -d $BASE/apk/cpuinfo/ ] && rm --force --recursive $BASE/apk/cpuinfo/ ; mkdir --parents $BASE/apk/cpuinfo/
curl --silent --location --output $BASE/apk/cpuinfo/com.kgurgul.cpuinfo_40403.apk https://f-droid.org/repo/com.kgurgul.cpuinfo_40403.apk
printf "6c0f6bff9ffd3416ab66b705dca0cff6f02af2ca153a1922b91c2287b8516fc0  $BASE/apk/cpuinfo/com.kgurgul.cpuinfo_40403.apk" | sha256sum -c || exit 1

[ ! -d $BASE/android/ ] && mkdir $BASE/android/
sshfs vagrant@192.168.221.50:/home/vagrant/bitmask_android_leap android -o uidfile=1000 -o gidfile=1000 \
-o StrictHostKeyChecking=no -o IdentityFile=$BASE/.vagrant/machines/debian_build/libvirt/private_key

cat <<-EOF
adb install apk/termux/com.termux_108.apk
adb install apk/termux/com.termux.api_47.apk
adb install apk/termux/com.termux.boot_7.apk
adb install apk/termux/com.termux.widget_11.apk
adb install apk/termux/com.termux.styling_29.apk
adb install apk/openvpn/de.blinkt.openvpn_175.apk
adb install apk/connectbot/org.connectbot_10906000.apk
adb install apk/cpuinfo/com.kgurgul.cpuinfo_40403.apk

adb install apk/normalProductionFat/release/Bitmask_release_1.0.0-RC1.apk
adb install apk/customProductionFat/release/LavabitProxy_release_1.0.0-RC1.apk
EOF
