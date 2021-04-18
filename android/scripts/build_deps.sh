#!/bin/bash

function quit {
    echo "Task failed. $1."
    exit 1
}


# Handle self referencing, sourcing etc.
if [[ $0 != $BASH_SOURCE ]]; then
  export CMD=$BASH_SOURCE
else
  export CMD=$0
fi

# Ensure a consistent working directory so relative paths work.
pushd `dirname $CMD` > /dev/null
SCRIPT_DIR=`pwd -P`
popd > /dev/null
cd $SCRIPT_DIR

BASE_DIR="$SCRIPT_DIR/.."
DIR_OVPNASSETS=$BASE_DIR/ics-openvpn/main/build/ovpnassets
DIR_OVPNLIBS=$BASE_DIR/ics-openvpn/main/build/intermediates/cmake/noovpn3/release/obj
DIR_GOLIBS=$BASE_DIR/go/lib/
FILE_X86=$BASE_DIR/go/out/x86/piedispatcherlib
FILE_ARM=$BASE_DIR/go/out/armeabi-v7a/piedispatcherlib

# init
# look for empty dir

cd $BASE_DIR
if [[ $(ls -A ${DIR_OVPNASSETS} &> /dev/null) && $(ls -A ${DIR_OVPNLIBS} &> /dev/null) ]]
then
    echo "Dirty build: skipped externalNativeBuild - reusing existing libs"
else
    echo "Clean build: starting externalNativeBuild"
    cd $BASE_DIR/ics-openvpn || quit "Directory ics-opevpn not found"
    ./gradlew --console plain clean main:externalNativeBuildCleanSkeletonRelease main:externalNativeBuildSkeletonRelease || quit "Build ics-openvpn native libraries failed"
    cd ..
fi

if [[ $(ls -A ${DIR_GOLIBS} &> /dev/null) ]]
then
    echo "Dirty build: Reusing go libraries"
else
    echo "Clean build: compiling Go libraries"
    cd $BASE_DIR/go || quit "Directory go not found"
    export ANDROID_NDK_HOME=$ANDROID_SDK_ROOT/ndk/21.4.7075529
    ./install_go.sh || quit "install_go.sh failed"
    ./android_build_web_core.sh || quit "android_build_web_core.sh (shapeshifter + pgpverify) failed"
    ./android_build_core.sh || quit "android build core (shapeshifter) failed"
    cd $BASE_DIR
fi
