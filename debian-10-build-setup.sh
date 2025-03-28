#!/bin/bash -e

sudo tee /etc/modprobe.d/nested.conf <<-EOF > /dev/null
options kvm_intel nested=1
EOF
sudo tee /etc/sysctl.d/50-inotify.conf <<-EOF > /dev/null
fs.inotify.max_user_watches = 100000
user.max_inotify_watches = 100000
EOF

sudo sysctl --quiet -p --system
sudo sed -i "s/1024/3072/g" /etc/default/haveged
sudo sed -i "s/ENABLED=.*/ENABLED=\"true\"/g" /etc/default/sysstat
sudo systemctl --quiet restart haveged && sudo systemctl --quiet restart sysstat

# Point us at the development environment.
sudo tee --append /etc/hosts <<-EOF > /dev/null
192.168.221.145 api.debian.local
192.168.221.146 vpn.debian.local

192.168.221.245 api.alma.local
192.168.221.246 vpn.alma.local
EOF

# Create a swap file.
sudo dd if=/dev/zero of=/swap bs=1M count=12384
sudo chmod 600 /swap
sudo mkswap /swap
sudo swapoff --all
sudo sed -i "s/swap    sw/swap    pri=1,discard,sw/g" /etc/fstab
sudo tee -a /etc/fstab <<-EOF > /dev/null
# Swap file added to avoid out of memory crashes.
/swap       none    swap    pri=10,discard,sw      0       0
EOF
sudo swapon --all

sudo tee -a /etc/sysctl.conf <<-EOF > /dev/null
vm.swappiness=10
vm.vfs_cache_pressure=50
vm.overcommit_ratio=400
EOF
sudo sysctl --quiet vm.vfs_cache_pressure=50
sudo sysctl --quiet vm.swappiness=10
sudo sysctl --quiet vm.overcommit_ratio=400

# Trim the drive to free space.
sudo sed -i "s/OnCalendar.*/OnCalendar=hourly/g" /lib/systemd/system/fstrim.timer
sudo sed -i "s/AccuracySec.*/AccuracySec=5m/g" /lib/systemd/system/fstrim.timer
sudo systemctl --quiet daemon-reload && sudo systemctl --quiet enable fstrim.timer

# Prevent an error.
sudo mkdir --parents /etc/ssl/certs/java/

# swap swap defaults
sudo tee /etc/apt/apt.conf.d/99options <<-EOF > /dev/null
APT::Install-Recommends "0";
APT::Install-Suggests "0";
APT::Acquire::Retries "20";
APT::Get::Assume-Yes "true";
Dpkg::Use-Pty "0";
EOF

{ sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y upgrade && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install androguard apt-file bash-builtins bash-completion bzip2 curl diffoscope dnsutils file gcc git gnupg gnutls-bin haveged lib32stdc++6 lib32z1 libcanberra-gtk-module libcanberra-gtk3-module libffi-dev libjpeg-dev libssl-dev make meld net-tools nload openssh-client openssl openjdk-11-jdk packagekit-gtk3-module python3-asn1crypto python3-babel python3-clint python3-defusedxml python3-dev python3-git python3-libcloud python3-mwclient python3-paramiko python3-pil python3-pip python3-pyasn1 python3-pyasn1-modules python3-qrcode python3-requests python3-ruamel.yaml python3-setuptools python3-vagrant python3-venv python3-yaml qemu qemu-kvm qemu-user-static rake rsync ruby ruby-bundler ruby-dev software-properties-common swig sysfsutils unzip vm wget zlib1g-dev libc6:i386 libncurses5:i386 libstdc++6:i386 lib32z1 libbz2-1.0:i386 < /dev/null > /dev/null ; } || \
{ echo 'Apt update/upgrade/install failed.' ; exit 1 ; }

# Android client build.
cd $HOME

sudo chown vagrant:vagrant $HOME/.bashrc

# Remove history limits.
sed -i "/HISTSIZE/d" $HOME/.bashrc
sed -i "/HISTCONTROL/d" $HOME/.bashrc
sed -i "/HISTFILESIZE/d" $HOME/.bashrc

tee -a $HOME/.bashrc <<-EOF > /dev/null

unset HISTCONTROL
unset HISTIGNORE
export HISTSIZE=1000000
export HISTFILESIZE=1000000

shopt -s histappend

EOF

sudo sed -i "/HISTSIZE/d" /root/.bashrc
sudo sed -i "/HISTCONTROL/d" /root/.bashrc
sudo sed -i "/HISTFILESIZE/d" /root/.bashrc

sudo tee -a /root/.bashrc <<-EOF > /dev/null

unset HISTCONTROL
unset HISTIGNORE
export HISTSIZE=1000000
export HISTFILESIZE=1000000

shopt -s histappend

EOF

sudo chown vagrant:vagrant $HOME/.vimrc
cat <<-EOF > $HOME/.vimrc
set mouse-=a
EOF

# Setup NFS share
# sudo tee -a /etc/exports <<-EOF > /dev/null
# /home/vagrant/android 192.168.221.1(rw,async,no_subtree_check,anonuid=1000,anongid=1000)
# EOF

[ ! -d /home/vagrant/android ] && mkdir /home/vagrant/android
# sudo systemctl enable nfs-server && sudo systemctl start nfs-server

# Install a text editor.
curl -Lso $HOME/sublimehq-pub.gpg https://download.sublimetext.com/sublimehq-pub.gpg || { echo 'Sublime key download failed.' ; exit 1 ; }
cat $HOME/sublimehq-pub.gpg | gpg --dearmor | sudo apt-key add -
echo "deb https://download.sublimetext.com/ apt/stable/" | sudo tee /etc/apt/sources.list.d/sublime-text.list > /dev/null
sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install sublime-text < /dev/null > /dev/null || \
{ echo 'Sublime install failed ... non-critical ... continuing.' ; }

# Install JDK v8. The Adoptium repos/servers frequently fail. which generates
# non-useful errors during CI builds. To reduce the number of failures, we retry 
# the setup/update/install commands several times, after a delay, before giving up  
# and triggering a fatal setup error.
{ curl -Lso $HOME/adoptopenjdk-pub.gpg https://packages.adoptium.net/artifactory/api/gpg/key/public ; } || \
{ sleep 120 ; curl -Lso $HOME/adoptopenjdk-pub.gpg https://packages.adoptium.net/artifactory/api/gpg/key/public ; } || \
{ sleep 120 ; curl -Lso $HOME/adoptopenjdk-pub.gpg https://packages.adoptium.net/artifactory/api/gpg/key/public ; } || \
{ sleep 180 ; curl -Lso $HOME/adoptopenjdk-pub.gpg https://packages.adoptium.net/artifactory/api/gpg/key/public ; } || \
{ sleep 300 ; curl -Lso $HOME/adoptopenjdk-pub.gpg https://packages.adoptium.net/artifactory/api/gpg/key/public ; } || \
{ sleep 300 ; curl -Lso $HOME/adoptopenjdk-pub.gpg https://packages.adoptium.net/artifactory/api/gpg/key/public ; } || \
{ sleep 300 ; curl -Lso $HOME/adoptopenjdk-pub.gpg https://packages.adoptium.net/artifactory/api/gpg/key/public ; } || \
{ sleep 300 ; curl -Lso $HOME/adoptopenjdk-pub.gpg https://packages.adoptium.net/artifactory/api/gpg/key/public ; } || \
{ echo 'Adoptium key download failed.' ; exit 1 ; }

cat $HOME/adoptopenjdk-pub.gpg | gpg --dearmor | sudo apt-key add -
sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true add-apt-repository --yes 'deb [arch=amd64] https://packages.adoptium.net/artifactory/deb/ buster main'

{ sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install temurin-8-jdk < /dev/null > /dev/null ; } || \
{ sleep 120 ; sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install temurin-8-jdk < /dev/null > /dev/null ; } || \
{ sleep 120 ; sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install temurin-8-jdk < /dev/null > /dev/null ; } || \
{ sleep 180 ; sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install temurin-8-jdk < /dev/null > /dev/null ; } || \
{ sleep 300 ; sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install temurin-8-jdk < /dev/null > /dev/null ; } || \
{ sleep 300 ; sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install temurin-8-jdk < /dev/null > /dev/null ; } || \
{ sleep 300 ; sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install temurin-8-jdk < /dev/null > /dev/null ; } || \
{ sleep 300 ; sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y update && sudo DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true apt-get -qq -y install temurin-8-jdk < /dev/null > /dev/null ; } || \
{ echo 'JDK 8 install failed.' ; exit 1 ; }

sudo update-alternatives --set java /usr/lib/jvm/temurin-8-jdk-amd64/bin/java
sudo update-alternatives --set javac /usr/lib/jvm/temurin-8-jdk-amd64/bin/javac

# Update the apt-file cache.
sudo apt-file update &> /dev/null || echo 'apt-file update failed ... non-critical ... continuing.'

# Cleanup.
rm -f $HOME/adoptopenjdk-pub.gpg $HOME/sublimehq-pub.gpg

# Install the Android command line tools.
curl --silent --show-error --location --output $HOME/commandlinetools-linux-11076708_latest.zip https://redirector.gvt1.com/edgedl/android/repository/commandlinetools-linux-11076708_latest.zip
printf "2d2d50857e4eb553af5a6dc3ad507a17adf43d115264b1afc116f95c92e5e258  $HOME/commandlinetools-linux-11076708_latest.zip" | sha256sum -c || \
( rm -f $HOME/commandlinetools-linux-11076708_latest.zip ; curl --silent --show-error --location --output $HOME/commandlinetools-linux-11076708_latest.zip https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip )
printf "2d2d50857e4eb553af5a6dc3ad507a17adf43d115264b1afc116f95c92e5e258  $HOME/commandlinetools-linux-11076708_latest.zip" | sha256sum -c || exit 1
sudo unzip -qq $HOME/commandlinetools-linux-11076708_latest.zip -d /opt/ && sudo mv /opt/cmdline-tools/ /opt/android-cmdline-tools/ && rm --force $HOME/commandlinetools-linux-11076708_latest.zip

[ -d /opt/android-sdk-linux/ ] && sudo rm --force --recursive /opt/android-sdk-linux/
yes | sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --licenses > /dev/null

# Required tools/packages.
sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --install \
"extras;google;m2repository" "extras;android;m2repository" 

sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --install \
"cmdline-tools;latest" "emulator" "platform-tools" \
"ndk;21.4.7075529" "cmake;3.10.2.4988404" \
"build-tools;30.0.3" "platforms;android-30" \
"build-tools;29.0.3" "platforms;android-29"

# New tools/packages.
sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --install \
"ndk;24.0.8215888" "cmake;3.18.1" \
"build-tools;32.0.0" "platforms;android-32" \
"build-tools;31.0.0" "platforms;android-31"

# The alternative install command.
# sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --install \
# "extras;google;simulators" "extras;google;webdriver" \
# "extras;google;m2repository" "extras;android;m2repository" \
# "cmdline-tools;latest" "ndk;21.4.7075529" "ndk;24.0.8215888" \
# "cmake;3.10.2.4988404" "cmake;3.18.1" \
# "skiaparser;1" "skiaparser;2" "skiaparser;3" \
# "build-tools;23.0.3" "build-tools;24.0.3" "build-tools;25.0.3" "build-tools;26.0.3" \
# "build-tools;27.0.3" "build-tools;28.0.3" "build-tools;29.0.3" "build-tools;30.0.3" \
# "build-tools;31.0.0" "build-tools;32.0.0" \
# "platforms;android-23" "platforms;android-24" "platforms;android-25" "platforms;android-26" \
# "platforms;android-27" "platforms;android-28" "platforms;android-29" "platforms;android-30" \
# "platforms;android-31" "platforms;android-32" \
# "system-images;android-26;default;x86" "system-images;android-26;default;x86_64" \
# "system-images;android-26;google_apis;x86" "system-images;android-26;google_apis_playstore;x86" \
# "system-images;android-27;default;x86" "system-images;android-27;default;x86_64" \
# "system-images;android-27;google_apis;x86" "system-images;android-27;google_apis_playstore;x86" \
# "system-images;android-28;default;x86" "system-images;android-28;default;x86_64" \
# "system-images;android-28;google_apis;x86" "system-images;android-28;google_apis;x86_64" \
# "system-images;android-28;google_apis_playstore;x86" "system-images;android-28;google_apis_playstore;x86_64" \
# "system-images;android-29;default;x86" "system-images;android-29;default;x86_64" \
# "system-images;android-29;google_apis;x86" "system-images;android-29;google_apis;x86_64" \
# "system-images;android-29;google_apis_playstore;x86" "system-images;android-29;google_apis_playstore;x86_64" \
# "system-images;android-30;google_apis;x86" "system-images;android-30;google_apis;x86_64" \
# "system-images;android-30;google_apis_playstore;x86" "system-images;android-30;google_apis_playstore;x86_64" \
# "system-images;android-31;default;x86_64" \
# "system-images;android-31;google_apis;x86_64" "system-images;android-31;google_apis_playstore;x86_64" \
# "system-images;android-32;google_apis;x86_64" "system-images;android-32;google_apis_playstore;x86_64"

sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --list_installed | awk -F' ' '{print $1}' | tail -n +4

# Setup bundletool.
sudo curl --silent --location --output /opt/android-sdk-linux/platform-tools/bundletool.jar https://github.com/google/bundletool/releases/download/1.5.0/bundletool-all-1.5.0.jar
printf "b7452e243a8bb32762ef74017f68291c685be0b3006b4b199fb94a7e7793dc85  /opt/android-sdk-linux/platform-tools/bundletool.jar" | sha256sum -c || exit 1
sudo tee /opt/android-sdk-linux/platform-tools/bundletool <<-EOF > /dev/null
#!/bin/bash

java -jar /opt/android-sdk-linux/platform-tools/bundletool.jar \$@
EOF

sudo chmod 755 /opt/android-sdk-linux/platform-tools/bundletool
sudo chmod 644 /opt/android-sdk-linux/platform-tools/bundletool.jar

# Setup aapt2.
sudo curl --silent --location --output /opt/android-sdk-linux/platform-tools/aapt2.jar https://redirector.gvt1.com/edgedl/android/maven2/com/android/tools/build/aapt2/4.1.3-6503028/aapt2-4.1.3-6503028-linux.jar
printf "023326dc41058a283c3092f95f894ca024199579627f85118a39843591ed4196  /opt/android-sdk-linux/platform-tools/aapt2.jar" | sha256sum -c || exit 1
sudo unzip -q -d /opt/android-sdk-linux/platform-tools/ /opt/android-sdk-linux/platform-tools/aapt2.jar aapt2
sudo rm --force /opt/android-sdk-linux/platform-tools/aapt2.jar

sudo chmod 755 /opt/android-sdk-linux/platform-tools/aapt2

# Platform Tools
[ -f /usr/lib/android-sdk/platform-tools/adb ] && sudo update-alternatives --install /usr/bin/adb adb /usr/lib/android-sdk/platform-tools/adb 10
sudo update-alternatives --install /usr/bin/adb adb /opt/android-sdk-linux/platform-tools/adb 20
sudo update-alternatives --install /usr/bin/aapt2 aapt2 /opt/android-sdk-linux/platform-tools/aapt2 20
sudo update-alternatives --install /usr/bin/bundletool bundletool /opt/android-sdk-linux/platform-tools/bundletool 20

# Build Tools (Platform Version Specific)
sudo update-alternatives --install /usr/bin/d8 d8 /opt/android-sdk-linux/build-tools/30.0.3/d8 20
sudo update-alternatives --install /usr/bin/dx dx /opt/android-sdk-linux/build-tools/30.0.3/dx 20
sudo update-alternatives --install /usr/bin/aidl aidl /opt/android-sdk-linux/build-tools/30.0.3/aidl 20
sudo update-alternatives --install /usr/bin/dexdump dexdump /opt/android-sdk-linux/build-tools/30.0.3/dexdump 20
sudo update-alternatives --install /usr/bin/zipalign zipalign /opt/android-sdk-linux/build-tools/30.0.3/zipalign 20
sudo update-alternatives --install /usr/bin/apksigner apksigner /opt/android-sdk-linux/build-tools/30.0.3/apksigner 20
sudo update-alternatives --install /usr/bin/apkanalyzer apkanalyzer /opt/android-sdk-linux/cmdline-tools/latest/bin/apkanalyzer 20

# Emulator
sudo update-alternatives --install /usr/bin/emulator emulator /opt/android-sdk-linux/emulator/emulator 20

# Command Line Tools (SDK/AVD Management)
sudo update-alternatives --install /usr/bin/sdkmanager sdkmanager /opt/android-sdk-linux/cmdline-tools/latest/bin/sdkmanager 20
sudo update-alternatives --install /usr/bin/avdmanager avdmanager /opt/android-sdk-linux/cmdline-tools/latest/bin/avdmanager 20

sudo tee /opt/android-sdk-linux/analytics.settings <<-EOF > /dev/null
{"userId":"00d88208-fba6-4128-bcab-43ea24471a29","hasOptedIn":false,"debugDisablePublishing":true,"saltValue":252009482191130365845997296475239957800466822540021702098,"saltSkew":666}
EOF

sudo chmod 664 /opt/android-sdk-linux/analytics.settings
HUMAN=$USER sudo --preserve-env=HUMAN sh -c 'chown $HUMAN:$HUMAN /opt/android-sdk-linux/analytics.settings'
# HUMAN=$USER sudo --preserve-env=HUMAN sh -c 'chown --recursive $HUMAN:$HUMAN /opt/android-sdk-linux/'

# Android Studio Setup
curl --silent --location --output $HOME/android-studio-2024.3.1.13-linux.tar.gz https://redirector.gvt1.com/edgedl/android/studio/ide-zips/2024.3.1.13/android-studio-2024.3.1.13-linux.tar.gz
printf "e3325ea03e457782a00adfe8373c6107061e1ee1c18269807854c87d90849510  $HOME/android-studio-2024.3.1.13-linux.tar.gz" | sha256sum -c || ( rm -f $HOME/android-studio-2024.3.1.13-linux.tar.gz ; curl --silent --show-error --location --output $HOME/android-studio-2024.3.1.13-linux.tar.gz https://dl.google.com/android/studio/ide-zips/2024.3.1.13/android-studio-2024.3.1.13-linux.tar.gz )
printf "e3325ea03e457782a00adfe8373c6107061e1ee1c18269807854c87d90849510  $HOME/android-studio-2024.3.1.13-linux.tar.gz" | sha256sum -c || exit 1

tar xzf android-studio-2024.3.1.13-linux.tar.gz || exit 1
( cd $HOME/bin ; ln -s $HOME/android-studio/bin/studio.sh studio ) || exit 1
rm -f android-studio-2024.3.1.13-linux.tar.gz

cat <<-EOF >> $HOME/.profile

export ANDROID_AVD_HOME=\$HOME/.avd
export ANDROID_SDK_HOME=\$HOME/.android
export ANDROID_PREFS_ROOT=\$HOME/.android
export ANDROID_HOME=/opt/android-sdk-linux
export ANDROID_SDK_ROOT=/opt/android-sdk-linux

EOF
