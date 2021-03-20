#!/bin/bash

# Install build dependencies.
export DEBIAN_FRONTEND=noninteractive
export DEBCONF_NONINTERACTIVE_SEEN=true

sudo apt-add-repository --yes --enable-source 'deb http://deb.debian.org/debian/ buster main contrib non-free'
sudo apt-add-repository --yes --enable-source 'deb http://deb.debian.org/debian/ buster-updates main contrib non-free'

sudo apt-get -qq -y update < /dev/null > /dev/null && sudo apt-get -qq -y remove "openjdk-*" "default-jre*" && sudo apt-get -qq -y install build-essential cmake cmake-data cmake-extras dbus dbus-java-bin dbus-x11 debhelper git libboost-all-dev libboost-dev libboost-filesystem-dev libboost-iostreams-dev libboost-log-dev libboost-program-options-dev libboost-system-dev libboost-test-dev libboost-thread-dev libcap-dev libd3dadapter9-mesa libdouble-conversion1 libegl1-mesa-dev libgl1-mesa-dev libgles2-mesa-dev libglm-dev liblxc1 libpcre2-16-0 libproperties-cpp-dev libprotobuf-dev libqt5core5a libqt5dbus5 libqt5gui5 libqt5network5 libqt5svg5 libqt5widgets5 libqt5x11extras5 libsdl2-dev libsdl2-image-dev libsystemd-dev libvulkan1 libvulkan-dev libx11-dev libxcb-composite0 libxcb-composite0-dev libxcb-cursor0 libxcb-cursor-dev libxcb-damage0 libxcb-damage0-dev libxcb-dpms0 libxcb-dpms0-dev libxcb-ewmh2 libxcb-ewmh-dev libxcb-icccm4 libxcb-icccm4-dev libxcb-image0 libxcb-image0-dev libxcb-imdkit0 libxcb-imdkit-dev libxcb-keysyms1 libxcb-keysyms1-dev libxcb-record0 libxcb-record0-dev libxcb-render-util0 libxcb-render-util0-dev libxcb-res0 libxcb-res0-dev libxcb-screensaver0 libxcb-screensaver0-dev libxcb-shm0-dev libxcb-xf86dri0 libxcb-xf86dri0-dev libxcb-xinerama0 libxcb-xinerama0-dev libxcb-xinput0 libxcb-xinput-dev libxcb-xkb1 libxcb-xkb-dev libxcb-xrm0 libxcb-xrm-dev libxcb-xtest0 libxcb-xtest0-dev libxcb-xv0 libxcb-xv0-dev libxcb-xvmc0 libxcb-xvmc0-dev libxkbcommon-x11-0 linux-headers-`uname -r` lxc-dev magics++ mesa-utils mesa-utils-extra mesa-vulkan-drivers multistrap pkg-config protobuf-compiler qt5dxcb-plugin qt5-gtk-platformtheme qt5-qmltooling-plugins qttranslations5-l10n qtwayland5 vulkan-tools vulkan-utils xbase-clients xorg libxcb-xinerama0-dev flex bison gperf libicu-dev libxslt-dev ruby libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev libegl1-mesa-dev gperf bison libasound2-dev libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev vim vim-nox wget curl gnupg mlocate sysstat lsof pciutils usbutils bc bison build-essential curl flex g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses-dev lib32readline-dev lib32z1-dev libesd-java liblz4-tool libncurses5 libncurses5-dev libsdl1.2-dev libssl-dev libwxgtk3.0-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev ninja-build icedtea-netx maven libatk-wrapper-java libatk-wrapper-java-jni libpng16-16 libsctp1 libdw-dev binutils-dev libdwarf-dev golang locales-all jaxws ant-doc antlr javacc junit junit4 jython libbcel-java libbsf-java libcommons-net-java libmail-java libjaxp1.3-java libjdepend-java libjsch-java liblog4j1.2-java liboro-java libregexp-java libxalan2-java libavalon-framework-java libcommons-logging-java-doc libexcalibur-logkit-java libdom4j-java-doc libxpp2-java libxpp3-java libdtd-parser-java-doc libjdom1-java libxom-java libjsoup-java-doc libmaven-file-management-java-doc libmaven-shared-io-java-doc libosgi-compendium-java libosgi-core-java testng libxerces2-java-doc libxml-commons-resolver1.1-java-doc bsh rhino libbcpkix-java libcompress-lzf-java liblog4j2-java libprotobuf-java jaxb adoptopenjdk-8-hotspot adoptopenjdk-8-hotspot-jre adoptopenjdk-8-openj9 adoptopenjdk-8-openj9-jre adoptopenjdk-8-openj9xl adoptopenjdk-8-openj9xl-jre < /dev/null > /dev/null && sudo apt-get -qq -y build-dep qt5-default < /dev/null > /dev/null


# Download and compile Googletest first, then compile the Anbox runtime.
cd $HOME && git clone https://github.com/google/googletest/ && cd googletest
mkdir build && cd build && cmake .. && make && sudo make install
cd $HOME && git clone https://github.com/anbox/anbox/ $HOME/anbox && cd $HOME/anbox
sed -i "s/max_id = 100000/max_id = 65535/g" $HOME/anbox/src/anbox/container/lxc_container.cpp
git submodule init && git submodule update --init --recursive && mkdir $HOME/anbox/build && cd $HOME/anbox/build && cmake .. && make && sudo make install

sudo install $HOME/anbox/scripts/anbox-bridge.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/anbox-init.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/anbox-shell.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/android-sync.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/app-android-settings.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/app-appmgr.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/build-debs.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/build.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/build-with-docker.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/clean-format.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/collect-bug-info.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/container-manager.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/create-package.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/gen-emugl-entries.py /usr/local/bin/
sudo install $HOME/anbox/scripts/gen-emugl-headers.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/load-kmods.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/mount-android.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/setup-partial-chroot.sh /usr/local/bin/
# sudo install $HOME/anbox/scripts/snap-wrapper.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/start-container.sh /usr/local/bin/
sudo install $HOME/anbox/scripts/update-emugl-sources.sh /usr/local/bin/

sudo modprobe binder_linux
sudo modprobe ashmem_linux

## Instead of compiling from source, use the following to install the snap.
# sudo apt-get -qq -y install snapd ifupdown2 android-tools-adb libdbus-c++-dev
# sudo snap -y install core snap-store
# sudo snap install --devmode --beta anbox
# ls -1 /dev/{ashmem,binder}
# sudo snap install --classic anbox-installer && anbox-installer
# adb install $HOME/proxy/apk/normalInsecureFat/debug/Bitmask_debug.apk
# anbox.appmgr
# export LD_LIBRARY_PATH=/snap/anbox/186/usr/lib/x86_64-linux-gnu/:/snap/anbox/186/lib/:/snap/core/10583/lib/x86_64-linux-gnu/:/snap/anbox/186/usr/lib/x86_64-linux-gnu/pulseaudio/:/snap/anbox/186/usr/lib/x86_64-linux-gnu/:/snap/core/10583/lib/i386-linux-gnu/

# The Anbox runtime is ready, now we need to build the image.
[ ! -d $HOME/temp ] && mkdir -p $HOME/temp
[ ! -d $HOME/cache ] && mkdir -p $HOME/cache
[ ! -d $HOME/anbox-work ] && mkdir -p $HOME/anbox-work

cat <<-EOF >> $HOME/.profile
# Setup the branch and enable the distributed cache.
export USE_CCACHE=1
export TMPDIR="\$HOME/temp"
export ANDROID_CCACHE_SIZE="20G"
export ANDROID_CCACHE_DIR="\$HOME/cache"
export PROCESSOR_COUNT=`cat /proc/cpuinfo  | grep processor | wc -l`

# Jack is the Java compiler used by LineageOS 14.1+, and it is memory hungry.
# We specify a memory limit of 8gb to avoid 'out of memory' errors.
export ANDROID_JACK_VM_ARGS="-Dfile.encoding=UTF-8 -XX:+TieredCompilation -Xmx8G"
EOF

export USE_CCACHE=1
export TMPDIR="$HOME/temp"
export ANDROID_CCACHE_SIZE="20G"
export ANDROID_CCACHE_DIR="$HOME/cache"
export PROCESSOR_COUNT=`cat /proc/cpuinfo  | grep processor | wc -l`
export ANDROID_JACK_VM_ARGS="-Dfile.encoding=UTF-8 -XX:+TieredCompilation -Xmx8G"

# Goto the build root.
cd $HOME/anbox-work

sudo curl --output /usr/bin/repo --location https://storage.googleapis.com/git-repo-downloads/repo
sudo chmod +x /usr/bin/repo

# Configure the default git username and email address.
git config --global user.name "nobody"
git config --global user.email "nobody@nowhere.com"
git config --global color.ui false

repo init -u https://github.com/anbox/platform_manifests.git -b anbox
# repo sync -j12 -c -d --no-tags --no-clone-bundle
repo sync -j12
sed -i "s/max_id = 100000/max_id = 65535/g" $HOME/anbox-work/vendor/anbox/src/anbox/container/lxc_container.cpp

. build/envsetup.sh
lunch anbox_x86_64-userdebug
make -j8

cd $HOME/anbox-work/vendor/anbox
scripts/create-package.sh $PWD/../../out/target/product/x86_64/ramdisk.img $PWD/../../out/target/product/x86_64/system.img

cd $HOME
sudo anbox-bridge.sh start
sudo mkdir --parents /var/lib/anbox/
sudo install $HOME/anbox-work/vendor/anbox/android.img /var/lib/anbox/android.img

## The alternative to building the image from scratch is downloading it.
# sudo mkdir --parents /var/lib/anbox/
# sudo curl --output /var/lib/anbox/android.img https://build.anbox.io/android-images/2018/07/19/android_amd64.img
# printf "6b04cd33d157814deaf92dccf8a23da4dc00b05ca6ce982a03830381896a8cca  /var/lib/anbox/android.img" | sha256sum -c || exit 1






# First attempt install/run Anbox.
# sudo apt-add-repository --yes --enable-source 'deb http://deb.debian.org/debian/ buster main contrib non-free'
# sudo apt-add-repository --yes --enable-source 'deb http://deb.debian.org/debian/ buster-updates main contrib non-free'
# sudo apt-get -qq -y update < /dev/null > /dev/null && sudo apt-get -qq -y install anbox build-essential cmake cmake-data cmake-extras dbus dbus-java-bin dbus-x11 debhelper g++-arm-linux-gnueabihf git google-mock libboost-all-dev libboost-dev libboost-filesystem-dev libboost-iostreams-dev libboost-log-dev libboost-program-options-dev libboost-system-dev libboost-test-dev libboost-thread-dev libcap-dev libd3dadapter9-mesa libdouble-conversion1 libegl1-mesa-dev libgl1-mesa-dev libgles2-mesa-dev libglm-dev libgtest-dev liblxc1 libpcre2-16-0 libproperties-cpp-dev libprotobuf-dev libqt5core5a libqt5dbus5 libqt5gui5 libqt5network5 libqt5svg5 libqt5widgets5 libqt5x11extras5 libsdl2-dev libsdl2-image-dev libsystemd-dev libvulkan1 libvulkan-dev libx11-dev libxcb-composite0 libxcb-composite0-dev libxcb-cursor0 libxcb-cursor-dev libxcb-damage0 libxcb-damage0-dev libxcb-dpms0 libxcb-dpms0-dev libxcb-ewmh2 libxcb-ewmh-dev libxcb-icccm4 libxcb-icccm4-dev libxcb-image0 libxcb-image0-dev libxcb-imdkit0 libxcb-imdkit-dev libxcb-keysyms1 libxcb-keysyms1-dev libxcb-record0 libxcb-record0-dev libxcb-render-util0 libxcb-render-util0-dev libxcb-res0 libxcb-res0-dev libxcb-screensaver0 libxcb-screensaver0-dev libxcb-shm0-dev libxcb-xf86dri0 libxcb-xf86dri0-dev libxcb-xinerama0 libxcb-xinerama0-dev libxcb-xinput0 libxcb-xinput-dev libxcb-xkb1 libxcb-xkb-dev libxcb-xrm0 libxcb-xrm-dev libxcb-xtest0 libxcb-xtest0-dev libxcb-xv0 libxcb-xv0-dev libxcb-xvmc0 libxcb-xvmc0-dev libxkbcommon-x11-0 linux-headers-`uname -r` lxc-dev magics++ mesa-utils mesa-utils-extra mesa-vulkan-drivers multistrap pkg-config protobuf-compiler qt5dxcb-plugin qt5-gtk-platformtheme qt5-qmltooling-plugins qttranslations5-l10n qtwayland5 vulkan-tools vulkan-utils xbase-clients xorg libgmock-dev < /dev/null > /dev/null

# sudo apt-get -qq -y build-dep qt5-default
# sudo apt-get -qq -y install libxcb-xinerama0-dev flex bison gperf libicu-dev libxslt-dev ruby libssl-dev libxcursor-dev libxcomposite-dev libxdamage-dev libxrandr-dev libdbus-1-dev libfontconfig1-dev libcap-dev libxtst-dev libpulse-dev libudev-dev libpci-dev libnss3-dev libasound2-dev libxss-dev libegl1-mesa-dev gperf bison libasound2-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
# sudo apt-get -qq -y install '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev

# sudo apt-get -qq -y update && sudo apt-get -qq -y install anbox dbus-x11 libx11-dev dbus-java-bin libgl1-mesa-dev
# sudo apt-get -y install build-essential linux-headers-`uname -r` cmake cmake-data debhelper dbus google-mock libboost-dev libboost-filesystem-dev libboost-log-dev libboost-iostreams-dev libboost-program-options-dev libboost-system-dev libboost-test-dev libboost-thread-dev libcap-dev libsystemd-dev libegl1-mesa-dev libgles2-mesa-dev libglm-dev libgtest-dev liblxc1 libproperties-cpp-dev libprotobuf-dev libsdl2-dev libsdl2-image-dev lxc-dev pkg-config protobuf-compiler
# sudo systemctl enable anbox-container-manager.service && sudo systemctl start anbox-container-manager.service && systemctl status anbox-container-manager.service
#systemctl --user enable anbox-session-manager.service && systemctl --user start anbox-session-manager.service && systemctl --user status anbox-session-manager.service
