# https://github.com/pokowaka/android-emulator-container-scripts
# https://github.com/vgaidarji/docker-android
export DEBIAN_FRONTEND=noninteractive
export DEBCONF_NONINTERACTIVE_SEEN=true

# Update video libs
sudo apt-get -qq -y install mesa-utils mesa-vulkan-drivers mesa-utils-extra libd3dadapter9-mesa mesa-utils mesa-utils-extra mesa-vulkan-drivers vulkan-tools vulkan-utils libvulkan-dev libvulkan1 binutils-dev binutils-multiarch binutils-multiarch-dbg binutils-multiarch-dev bridge-utils multiarch-support lib32z1 lib32stdc++6 software-properties-common sshfs

curl --location --silent https://adoptopenjdk.jfrog.io/adoptopenjdk/api/gpg/key/public | sudo apt-key add -
export GNUPGHOME=$(mktemp -d /tmp/gnupg-XXXXXX)
[ "`gpg --quiet --no-options --keyring /etc/apt/trusted.gpg --list-keys 8ED17AF5D7E675EB3EE3BCE98AC3B29174885C03 | wc -l`" != "5" ] && exit 1
rm --force --recursive $GNUPGHOME
sudo add-apt-repository --yes 'deb [arch=amd64] https://adoptopenjdk.jfrog.io/adoptopenjdk/deb/ buster main'
sudo apt-get -qq -y update && sudo apt-get -qq -y install adoptopenjdk-8-hotspot < /dev/null > /dev/null

sudo update-alternatives --set java /usr/lib/jvm/adoptopenjdk-8-hotspot-amd64/bin/java
sudo update-alternatives --set javac /usr/lib/jvm/adoptopenjdk-8-hotspot-amd64/bin/javac

# Install the Command Line Tools
[ -d /opt/cmdline-tools/ ] && sudo rm --force --recursive /opt/cmdline-tools/
[ -d /opt/android-sdk-linux/ ] && sudo rm --force --recursive /opt/android-sdk-linux/

# Install the Command Line Tools
curl --silent --show-error --location --output $HOME/commandlinetools-linux-6858069_latest.zip https://dl.google.com/android/repository/commandlinetools-linux-6858069_latest.zip
printf "87f6dcf41d4e642e37ba03cb2e387a542aa0bd73cb689a9e7152aad40a6e7a08  $HOME/commandlinetools-linux-6858069_latest.zip" | sha256sum -c || exit 1
sudo unzip -qq $HOME/commandlinetools-linux-6858069_latest.zip -d /opt/ && sudo mv /opt/cmdline-tools/ /opt/android-cmdline-tools/ && rm --force $HOME/commandlinetools-linux-6858069_latest.zip
yes | sudo sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --licenses > /dev/null

sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --install \
"extras;google;m2repository" "extras;android;m2repository" \
"cmdline-tools;latest" "build-tools;30.0.3" "platforms;android-30" "skiaparser;1" \
"system-images;android-30;google_apis;x86" "system-images;android-30;google_apis;x86_64" \
"system-images;android-30;google_apis_playstore;x86" "system-images;android-30;google_apis_playstore;x86_64"

sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --list_installed | awk -F' ' '{print $1}' | tail -n +4

[ -f /usr/lib/android-sdk/platform-tools/adb ] && sudo update-alternatives --install /usr/bin/adb adb /usr/lib/android-sdk/platform-tools/adb 10

sudo update-alternatives --install /usr/bin/adb adb /opt/android-sdk-linux/platform-tools/adb 20
sudo update-alternatives --install /usr/bin/emulator emulator /opt/android-sdk-linux/emulator/emulator 20
sudo update-alternatives --install /usr/bin/sdkmanager sdkmanager /opt/android-sdk-linux/cmdline-tools/latest/bin/sdkmanager 20
sudo update-alternatives --install /usr/bin/avdmanager avdmanager /opt/android-sdk-linux/cmdline-tools/latest/bin/avdmanager 20

## The alternatve install command.
# sudo /opt/android-cmdline-tools/bin/sdkmanager --sdk_root=/opt/android-sdk-linux/ --install \
# "extras;google;simulators" "extras;google;webdriver" \
# "extras;google;m2repository" "extras;android;m2repository" \
# "cmdline-tools;latest" "ndk;21.3.6528147" "cmake;3.10.2.4988404" "skiaparser;1" \
# "build-tools;23.0.3" "build-tools;24.0.3" "build-tools;25.0.3" "build-tools;26.0.3" \
# "build-tools;27.0.3" "build-tools;28.0.3" "build-tools;29.0.3" "build-tools;30.0.3" \
# "platforms;android-23" "platforms;android-24" "platforms;android-25" "platforms;android-26" \
# "platforms;android-27" "platforms;android-28" "platforms;android-29" "platforms;android-30" \
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
# "system-images;android-30;google_apis_playstore;x86" "system-images;android-30;google_apis_playstore;x86_64"

## Install the Android integrated development studio.
# [ ! -d /opt/android-sdk-linux/studio/ ] && sudo mkdir /opt/android-sdk-linux/studio/
# curl --silent --show-error --location --output $HOME/android-studio-ide-201.7042882-linux.tar.gz https://dl.google.com/android/studio/ide-zips/4.1.2.0/android-studio-ide-201.7042882-linux.tar.gz
# printf "89f7c3a03ed928edeb7bbb1971284bcb72891a77b4f363557a7ad4ed37652bb9  $HOME/android-studio-ide-201.7042882-linux.tar.gz" | sha256sum -c || exit 1
# sudo tar --extract --ungzip --strip-components=1 --directory=/opt/android-sdk-linux/studio/ --file=$HOME/android-studio-ide-201.7042882-linux.tar.gz && rm --force $HOME/android-studio-ide-201.7042882-linux.tar.gz
#
# sudo update-alternatives --install /usr/bin/studio studio /opt/android-sdk-linux/studio/bin/studio.sh 20
# sudo update-alternatives --install /usr/bin/studio.sh studio.sh /opt/android-sdk-linux/studio/bin/studio.sh 20

sudo tee /opt/android-sdk-linux/analytics.settings <<-EOF > /dev/null
{"userId":"00d88208-fba6-4128-bcab-43ea24471a29","hasOptedIn":false,"debugDisablePublishing":true,"saltValue":252009482191130365845997296475239957800466822540021702098,"saltSkew":666}
EOF

sudo chmod 664 /opt/android-sdk-linux/analytics.settings
HUMAN=$USER sudo --preserve-env=HUMAN sh -c 'chown $HUMAN:$HUMAN /opt/android-sdk-linux/analytics.settings'
# HUMAN=$USER sudo --preserve-env=HUMAN sh -c 'chown --recursive $HUMAN:$HUMAN /opt/android-sdk-linux/'

cat <<-EOF >> $HOME/.profile

export ANDROID_HOME=/opt/android-sdk-linux
export ANDROID_SDK_ROOT=/opt/android-sdk-linux

EOF

export ANDROID_HOME=/opt/android-sdk-linux
export ANDROID_SDK_ROOT=/opt/android-sdk-linux



# [ ! -d /opt/android-sdk-linux/emulator/qemu/linux-x86_64/lib64/ ] && sudo ln -s /opt/android-sdk-linux/emulator/lib64/ /opt/android-sdk-linux/emulator/qemu/linux-x86_64/lib64
