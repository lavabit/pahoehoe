#!/bin/bash




# Setup libvirt / vagrant / qemu / kvm on Ubuntu so it can function as a build host. Adds a user to the kvm / libvirt groups. Note the username may need to be changed.
# env DEBCONF_NONINTERACTIVE_SEEN=true DEBIAN_FRONTEND=noninteractive sudo apt-get -y install binutils-multiarch binutils-multiarch-dbg binutils-multiarch-dev bridge-utils libvirt-bin libvirt-daemon multiarch-support qemu-kvm ubuntu-vm-builder virt-manager lib32z1 lib32stdc++6 && sudo systemctl enable libvirtd.service && sudo systemctl start libvirtd.service && sudo usermod -aG kvm ladar ; sudo usermod -aG libvirt ladar ; sudo usermod -aG libvirt-qemu ladar

# Update video libs
# sudo apt-get -y install mesa-utils mesa-vulkan-drivers mesa-utils-extra libd3dadapter9-mesa
# sudo apt-get -y install mesa-utils mesa-utils-extra mesa-vulkan-drivers vulkan-tools vulkan-utils libvulkan-dev libvulkan1


# env DEBCONF_NONINTERACTIVE_SEEN=true DEBIAN_FRONTEND=noninteractive sudo apt-get -y upgrade && sudo apt-get -y install aapt abootimg adb aidl androguard android-framework-res android-libaapt android-libadb android-libadb-dev android-libandroidfw android-libandroidfw-dev android-libart android-libbacktrace android-libbacktrace-dev android-libbase android-libbase-dev android-libboringssl android-libboringssl-dev android-libcrypto-utils android-libcrypto-utils-dev android-libcutils android-libcutils-dev android-libetc1 android-libetc1-dev android-libext4-utils android-libext4-utils-dev android-libf2fs-utils android-libf2fs-utils-dev android-liblog android-liblog-dev android-libnativebridge android-libnativebridge-dev android-libnativehelper android-libnativehelper-dev android-libnativeloader android-libnativeloader-dev android-libselinux android-libselinux-dev android-libsepol android-libsepol-dev android-libsparse android-libsparse-dev android-libunwind android-libunwind-dev android-libutils android-libutils-dev android-libziparchive android-libziparchive-dev android-logtags-tools android-platform-frameworks-native-headers android-platform-libcore-headers android-platform-system-core-headers android-sdk android-sdk-build-tools android-sdk-build-tools-common android-sdk-common android-sdk-ext4-utils android-sdk-libsparse-utils android-sdk-platform-23 android-sdk-platform-tools android-sdk-platform-tools-common android-tools-adb android-tools-adbd android-tools-fastboot android-tools-mkbootimg apkinfo apksigner apktool clementine cpuinfo crtmpserver crtmpserver-apps crtmpserver-dev crtmpserver-libs dalvik-exchange dexdump dexlist dmtracedump doclava-aosp dsmidiwifi dummydroid enjarify etc1tool fastboot fcitx-googlepinyin fdroidcl fdroidserver fonts-noto-color-emoji fonts-roboto fonts-roboto-hinted fonts-roboto-slab fonts-roboto-unhinted fp-units-castle-game-engine fwknop-gui gajim-omemo go-mtpfs golang-github-mvdan-fdroidcl-dev google-cloud-print-connector gpgv-static gradle gradle-doc hashcat hprof-conv ibus-keyman jmtpfs kdeconnect keysync kirigami2-dev libam7xxx-dev libam7xxx0.1 libam7xxx0.1-bin libam7xxx0.1-doc libandroid-23-java libandroid-json-java libandroid-json-org-java libandroid-json-org-java-doc libandroid-uiautomator-23-java libapksig-java libavcodec-extra58 libaxmlrpc-java libaxmlrpc-java-doc libbullet-dev libcpuinfo-dev libcpuinfo0 libgooglepinyin0 libgooglepinyin0-dev libgradle-core-java libgradle-plugins-java libjacoco-java libjacoco-java-doc libjsilver-aosp-java libkf5kirigami2-5 libkf5kirigami2-doc libkiwix-dev libkiwix3 libkmnkbp-dev libkmnkbp0-0 libokhttp-java libopencore-amrnb-dev libopencore-amrnb0 libopencore-amrnb0-dbg libopencore-amrwb-dev libopencore-amrwb0 libopencore-amrwb0-dbg libpam-barada libsmali-java libsqlcipher-dev libsqlcipher0 libtrident-java libvo-aacenc-dev libvo-aacenc0 libvo-amrwbenc-dev libvo-amrwbenc0 libvoaacenc-ocaml libvoaacenc-ocaml-dev lime-forensics-dkms linssid mkbootimg mobile-atlas-creator novnc nuntius openorienteering-mapper pidcat python-apns-client python-axolotl python-gcm-client python-novnc python3-axolotl python3-novnc python3-pyaxmlparser qml-module-org-kde-kirigami2 renderdoc signapk signtos skales split-select sqlcipher torch-core-free utox vo-aacenc-dbg vo-amrwbenc-dbg volatility zipalign ziptime firefox-esr


# VirtualBox for use by the emulator.
# sudo bash -c 'echo "deb [arch=amd64] https://download.virtualbox.org/virtualbox/debian bionic contrib" > /etc/apt/sources.list.d/virtualbox.conf'
# curl https://www.virtualbox.org/download/oracle_vbox_2016.asc | sudo apt-key add -
# curl https://www.virtualbox.org/download/oracle_vbox.asc | sudo apt-key add -
# sudo apt-get update && sudo apt-get -y install virtualbox-6.1 && sudo usermod -aG vboxusers ladar
#
# # Genymotion installation steps.
# curl -o $HOME/genymotion-3.1.2-linux_x64.bin https://dl.genymotion.com/releases/genymotion-3.1.2/genymotion-3.1.2-linux_x64.bin && chmod +x $HOME/genymotion-3.1.2-linux_x64.bin && sudo ./genymotion-3.1.2-linux_x64.bin --yes && rm --force $HOME/genymotion-3.1.2-linux_x64.bin


# cd $HOME && rm --force androidx86_hda.img android-x86_64-9.0-r2.iso && curl -o $HOME/android-x86_64-9.0-r2.iso https://mirrors.xtom.com/osdn/android-x86/71931/android-x86_64-9.0-r2.iso && qemu-img create -f qcow2 $HOME/androidx86_hda.img 10G && qemu-system-x86_64 -enable-kvm -m 4096 -smp 4 -cpu host -soundhw es1370 -device virtio-mouse-pci -device virtio-keyboard-pci -serial mon:stdio -boot menu=off -net nic -net user,hostfwd=tcp::5555-:22 -vga qxl -hda $HOME/androidx86_hda.img -cdrom $HOME/android-x86_64-9.0-r2.iso

# qemu-system-x86_64 -enable-kvm -m 4096 -smp 4 -cpu host -soundhw es1370 -device virtio-mouse-pci -device virtio-keyboard-pci -serial mon:stdio -boot menu=off -net nic -net user,hostfwd=tcp::5555-:22 -vga qxl -hda $HOME/androidx86_hda.img

# /opt/android-sdk-linux/cmdline-tools/latest/bin/avdmanager create avd --force --device "3.3in WQVGA" --package "system-images;android-28;default;x86_64" --name vpn
#
# $HOME/android-sdk-linux/cmdline-tools/latest/bin/avdmanager --clear-cache create avd --force --device "Nexus 5" --name vpn --package "system-images;android-24;default;x86"
# cd /opt/android-sdk-linux/emulator/ ; ./emulator -verbose -no-window -qemu -device help
# cd /opt/android-sdk-linux/emulator/ ; env ANDROID_EMULATOR_USE_SYSTEM_LIBS=1 ./emulator -verbose -avd generic -no-audio -cores 4 -netfast -no-snapshot -qemu -cpu host -enable-kvm -device virtio-vga -display virtio
# /opt/android-sdk-linux/emulator/emulator -avd vpn-32
# /opt/android-sdk-linux/emulator/emulator -avd vpn-64

# Run the test class.
# adb shell am instrument -w <test_package_name>/<runner_class>

# Cleanup
# /opt/android-sdk-linux/cmdline-tools/latest/bin/avdmanager delete avd --name vpn
# /opt/android-sdk-linux/cmdline-tools/latest/bin/avdmanager delete avd --name generic
# To see only the errors.
## adb logcat *:E

# [ -b /dev/vdb1 ] && [ ! -d $HOME/android-sdk-linux/ ] && mkdir $HOME/android-sdk-linux/
# [ -b /dev/vdb1 ] && ( sudo mount /dev/vdb1 $HOME/android-sdk-linux/ && sudo chown vagrant:vagrant $HOME/android-sdk-linux/ )

# /opt/android-studio/bin/studio.sh


# /opt/android-sdk-linux/cmdline-tools/latest/bin/avdmanager delete avd --name vpn-32
# /opt/android-sdk-linux/cmdline-tools/latest/bin/avdmanager delete avd --name vpn-64

# GPU alternative == swiftshader_indirect

# cd .sdk/emulator/ ; env ANDROID_EMULATOR_USE_SYSTEM_LIBS=1 primusrun ./emulator -use-system-libs -no-boot-anim -screen no-touch -avd Nexus_5_API_30 -wipe-data -cache-size 1000 -no-audio -no-snapshot -engine qemu2 -gpu host -qemu '-enable-kvm'
