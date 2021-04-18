#!/bin/bash -x

cd $HOME/android

./scripts/build_deps.sh
./gradlew --console plain --warning-mode none consoleassembleLavabit assembleNormalProductionFatweb
./gradlew --console plain --warning-mode none bundleLavabit
./gradlew --console plain --warning-mode none check

echo "All finished."
sudo fstrim --all
( for i in {1..5}; do printf "\a" ; sleep 0.2; done ; sleep 2 ; for i in {1..5}; do printf "\a" ; sleep 0.2; done ) &
