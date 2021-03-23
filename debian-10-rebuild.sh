#!/bin/bash -x

cd $HOME/android

./scripts/build_deps.sh
./gradlew --warning-mode none build
./gradlew --warning-mode none assembleDebug
./gradlew --warning-mode none assembleRelease

echo "All finished."
sudo fstrim --all
( for i in {1..10}; do printf "\a" ; sleep 1; done ) &
