#!/bin/bash -e

export DEBIAN_FRONTEND=noninteractive
export DEBCONF_NONINTERACTIVE_SEEN=true

sudo tee /etc/modprobe.d/nested.conf <<-EOF > /dev/null
options kvm_intel nested=1
EOF
sudo tee /etc/sysctl.d/50-inotify.conf <<-EOF > /dev/null
fs.inotify.max_user_watches = 100000
user.max_inotify_watches = 100000
EOF

sudo sysctl -p --system
sudo sed -i "s/1024/3072/g" /etc/default/haveged
sudo sed -i "s/ENABLED=.*/ENABLED=\"true\"/g" /etc/default/sysstat
sudo systemctl restart haveged && sudo systemctl restart sysstat

# Point us at the development environment.
sudo tee --append /etc/hosts <<-EOF
192.168.221.146 api.debian.local
192.168.221.142 vpn.debian.local
192.168.221.142 142.vpn.debian.local
192.168.221.143 143.vpn.debian.local
192.168.221.144 144.vpn.debian.local
192.168.221.145 145.vpn.debian.local

192.168.221.246 api.centos.local
192.168.221.242 vpn.centos.local
192.168.221.242 242.vpn.centos.local
192.168.221.243 243.vpn.centos.local
192.168.221.244 244.vpn.centos.local
192.168.221.245 245.vpn.centos.local
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
EOF
sudo sysctl vm.vfs_cache_pressure=50
sudo sysctl vm.swappiness=10

# Trim the drive to free space.
sudo sed -i "s/OnCalendar.*/OnCalendar=hourly/g" /lib/systemd/system/fstrim.timer
sudo sed -i "s/AccuracySec.*/AccuracySec=5m/g" /lib/systemd/system/fstrim.timer
sudo systemctl daemon-reload && sudo systemctl enable fstrim.timer

# Prevent an error.
#sudo mkdir --parents /etc/ssl/certs/java/cacerts

# swap swap defaults
sudo tee /etc/apt/apt.conf.d/99options <<-EOF > /dev/null
APT::Install-Recommends "0";
APT::Install-Suggests "0";
APT::Acquire::Retries "20";
APT::Get::Assume-Yes "true";
Dpkg::Use-Pty "0";
EOF

sudo apt-get -qq -y update && sudo apt-get -qq -y install androguard apt-file bash-builtins bash-completion bzip2 curl diffoscope dnsutils file gcc git gnupg gnutls-bin haveged lib32stdc++6 lib32z1 libcanberra-gtk-module libcanberra-gtk3-module libffi-dev libjpeg-dev libssl-dev make meld net-tools nload openssh-client openssl packagekit-gtk3-module python3-asn1crypto python3-babel python3-clint python3-defusedxml python3-dev python3-git python3-libcloud python3-mwclient python3-paramiko python3-pil python3-pip python3-pyasn1 python3-pyasn1-modules python3-qrcode python3-requests python3-ruamel.yaml python3-setuptools python3-vagrant python3-venv python3-yaml qemu qemu-kvm qemu-user-static rake rsync ruby ruby-bundler ruby-dev software-properties-common swig sysfsutils unzip vm wget zlib1g-dev < /dev/null > /dev/null

# Android client build.
cd $HOME

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

# Setup NFS share
# sudo tee -a /etc/exports <<-EOF > /dev/null
# /home/vagrant/android 192.168.221.1(rw,async,no_subtree_check,anonuid=1000,anongid=1000)
# EOF

[ ! -d /home/vagrant/android ] && mkdir /home/vagrant/android
# sudo systemctl enable nfs-server && sudo systemctl start nfs-server

# Install Atom editor.
curl --location --silent https://packagecloud.io/AtomEditor/atom/gpgkey | sudo apt-key add -
export GNUPGHOME=$(mktemp -d /tmp/gnupg-XXXXXX)
[ "`gpg --quiet --no-options --keyring /etc/apt/trusted.gpg --list-keys 0A0FAB860D48560332EFB581B75442BBDE9E3B09 | wc -l`" != "5" ] && exit 1
rm --force --recursive $GNUPGHOME
sudo add-apt-repository --yes 'deb [arch=amd64] https://packagecloud.io/AtomEditor/atom/any/ any main'
sudo apt-get -qq -y update < /dev/null > /dev/null && sudo apt-get -qq -y install atom < /dev/null > /dev/null

# Setup Atom with an initial config, that matches our personal preferences.
[ ! -d $HOME/.atom/ ] && mkdir $HOME/.atom/
cat <<-EOF > $HOME/.atom/config.cson
"*":
  "atom-beautify":
    html:
      indent_size: 2
  "atom-material-ui":
    colors:
      paintCursor: true
    tabs:
      compactTabs: true
  "autocomplete-plus":
    enableAutoConfirmSingleSuggestion: false
  autosave: {}
  core:
    autoHideMenuBar: false
    automaticallyUpdate: false
    closeEmptyWindows: false
    disabledPackages: [
      "welcome"
      "metrics"
      "exception-reporting"
      "github"
    ]
    telemetryConsent: "no"
    useTreeSitterParsers: true
    reopenProjectMenuCount: 30
    useTreeSitterParsers: false
  editor:
    atomicSoftTabs: false
    defaultFontSize: 16
    fontSize: 16
    maxScreenLineLength: 1000
    showIndentGuide: true
    showInvisibles: true
    softWrap: false
    tabType: "soft"
  "find-and-replace": {}
  "git-diff":
    showIconsInEditorGutter: true
  "spell-check":
    addKnownWords: true
    grammars: [
      "source.shell"
      "source.c"
    ]
    knownWords: [
      "runtime"
      "virtualization"
    ]
    localePaths: [
      "/usr/share/myspell/"
    ]
    locales: [
      "en-US"
    ]
    useSystem: false
  github:
    showDiffIconGutter: true
  "one-dark-ui":
    fontSize: 16
  "open-recent":
    maxRecentDirectories: 30
    maxRecentFiles: 100
  "tree-view":
    squashDirectoryNames: true
  welcome:
    showOnStartup: false
  whitespace:
    ensureSingleTrailingNewline: false
    removeTrailingWhitespace: false
EOF

# Hide the tree view when Atom loads.
cat <<-EOF > $HOME/.atom/init.coffee

# waitForPackageActivation = (causeTreeToggle) ->
#   disposable = atom.workspace.onDidOpen ({item}) ->
#     atom.commands.dispatch atom.views.getView(atom.workspace), 'tree-view:toggle'
#     disposable.dispose()
# waitForPackageActivation()

atom.workspace.onDidOpen ({item}) ->
  itemName = item.constructor.name
  if (itemName  != 'TreeView')
    dock = atom.workspace.paneContainerForURI('atom://tree-view')
    if dock && dock.isVisible()
      dock.hide()

EOF

# Install the Atom packages we like having.
apm install sort-lines > /dev/null
apm install open-recent > /dev/null
apm install atom-beautify > /dev/null
apm install language-cmake > /dev/null
apm install language-groovy > /dev/null
apm install language-gradle > /dev/null
apm install language-kotlin > /dev/null
apm install language-openvpn > /dev/null

# Install JDK v8
curl --location --silent https://adoptopenjdk.jfrog.io/adoptopenjdk/api/gpg/key/public | sudo apt-key add -
export GNUPGHOME=$(mktemp -d /tmp/gnupg-XXXXXX)
[ "`gpg --quiet --no-options --keyring /etc/apt/trusted.gpg --list-keys 8ED17AF5D7E675EB3EE3BCE98AC3B29174885C03 | wc -l`" != "5" ] && exit 1
rm --force --recursive $GNUPGHOME
sudo add-apt-repository --yes 'deb [arch=amd64] https://adoptopenjdk.jfrog.io/adoptopenjdk/deb/ buster main'
sudo apt-get -qq -y update && sudo apt-get -qq -y install adoptopenjdk-8-hotspot < /dev/null > /dev/null

sudo update-alternatives --set java /usr/lib/jvm/adoptopenjdk-8-hotspot-amd64/bin/java
sudo update-alternatives --set javac /usr/lib/jvm/adoptopenjdk-8-hotspot-amd64/bin/javac

# Update the apt-file cache.
sudo apt-file update &> /dev/null

# Install the Android command line tools.
curl --silent --show-error --location --output $HOME/commandlinetools-linux-8092744_latest.zip https://dl.google.com/android/repository/commandlinetools-linux-8092744_latest.zip
printf "d71f75333d79c9c6ef5c39d3456c6c58c613de30e6a751ea0dbd433e8f8b9cbf  $HOME/commandlinetools-linux-8092744_latest.zip" | sha256sum -c || \
( rm -f $HOME/commandlinetools-linux-8092744_latest.zip ; curl --silent --show-error --location --output $HOME/commandlinetools-linux-8092744_latest.zip https://dl-ssl.google.com/android/repository/commandlinetools-linux-8092744_latest.zip )
printf "d71f75333d79c9c6ef5c39d3456c6c58c613de30e6a751ea0dbd433e8f8b9cbf  $HOME/commandlinetools-linux-8092744_latest.zip" | sha256sum -c || exit 1
sudo unzip -qq $HOME/commandlinetools-linux-8092744_latest.zip -d /opt/ && sudo mv /opt/cmdline-tools/ /opt/android-cmdline-tools/ && rm --force $HOME/commandlinetools-linux-8092744_latest.zip

[ -d /opt/android-sdk-linux/ ] && sudo rm --force --recursive /opt/android-sdk-linux/
yes | sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --licenses > /dev/null

# Required tools/packages.
sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --install \
"extras;google;m2repository" "extras;android;m2repository" 

sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --install \
"cmdline-tools;latest" \
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
sudo curl --silent --location --output /opt/android-sdk-linux/platform-tools/aapt2.jar https://dl.google.com/android/maven2/com/android/tools/build/aapt2/4.1.3-6503028/aapt2-4.1.3-6503028-linux.jar
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
sudo update-alternatives --install /usr/bin/apkanalyzer apkanalyzer /opt/android-sdk-linux/tools/bin/apkanalyzer 20

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

cat <<-EOF >> $HOME/.profile

export ANDROID_AVD_HOME=\$HOME/.avd
export ANDROID_SDK_HOME=\$HOME/.android
export ANDROID_PREFS_ROOT=\$HOME/.android
export ANDROID_HOME=/opt/android-sdk-linux
export ANDROID_SDK_ROOT=/opt/android-sdk-linux

EOF
