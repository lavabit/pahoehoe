
cd $HOME
mkdir boringdroid-x86
cd boringdroid-x86
repo init -u https://github.com/boringdroid/manifest.git -b boringdroid-x86-9.0.0
repo sync -c -d --no-tags --no-clone-bundle

source build/envsetup.sh
lunch android_x86_64-userdebug

