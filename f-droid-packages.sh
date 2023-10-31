#!/bin/bash

# Download the f-droid index.
{ touch /tmp/index-v1.jar && INDEX_JAR="/tmp/index-v1.jar" ; } || INDEX_JAR="$HOME/index-v1.jar"
[ -f "$INDEX_JAR" ] && rm -f "$INDEX_JAR"
curl -Lfso "$INDEX_JAR" "https://f-droid.org/repo/index-v1.jar" || exit 1

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "com.termux" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do 
cat <<-EOF

# Termux version 118 requires at least v24 of the Android SDK.
[ -d \$BASE/build/termux/ ] && rm --force --recursive \$BASE/build/termux/ ; mkdir --parents \$BASE/build/termux/
curl --fail --silent --show-error --location --output \$BASE/build/termux/$APK https://f-droid.org/repo/$APK || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/termux/$APK" | sha256sum -c --quiet || exit 1

EOF
done

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "com.termux.api" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do
cat <<-EOF
curl --fail --silent --show-error --location --output \$BASE/build/termux/$APK https://f-droid.org/repo/$APK || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/termux/$APK" | sha256sum -c --quiet || exit 1

EOF
done

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "com.termux.boot" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do
cat <<-EOF
curl --fail --silent --show-error --location --output \$BASE/build/termux/$APK https://f-droid.org/repo/$APK || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/termux/$APK" | sha256sum -c --quiet || exit 1

EOF
done

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "com.termux.widget" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do
cat <<-EOF
curl --fail --silent --show-error --location --output \$BASE/build/termux/$APK https://f-droid.org/repo/$APK || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/termux/$APK" | sha256sum -c --quiet || exit 1

EOF
done

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "com.termux.styling" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do
cat <<-EOF
curl --fail --silent --show-error --location --output \$BASE/build/termux/$APK https://f-droid.org/repo/$APK || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/termux/$APK" | sha256sum -c --quiet || exit 1

EOF
done

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "org.connectbot" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do
cat <<-EOF
# Download ConnectBot, which will work on devices with Android SDK v14 and higher..
[ -d \$BASE/build/connectbot/ ] && rm --force --recursive \$BASE/build/connectbot/ ; mkdir --parents \$BASE/build/connectbot/
curl --fail --silent --show-error --location --output \$BASE/build/connectbot/$APK https://f-droid.org/repo/$APK || 
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/connectbot/$APK" | sha256sum -c --quiet || exit 1

EOF
done

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "de.blinkt.openvpn" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do
cat <<-EOF
# Download the OpenVPN Android GUI
[ -d \$BASE/build/openvpn/ ] && rm --force --recursive \$BASE/build/openvpn/ ; mkdir --parents \$BASE/build/openvpn/
curl --fail --silent --show-error --location --output \$BASE/build/openvpn/$APK https://f-droid.org/repo/$APK || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/openvpn/$APK" | sha256sum -c --quiet || exit 1

EOF
done

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "com.lavabit.pahoehoe" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do
cat <<-EOF
# Download the currently released Lavabit App
[ -d \$BASE/build/lavabit/ ] && rm --force --recursive \$BASE/build/lavabit/ ; mkdir --parents \$BASE/build/lavabit/
curl --fail --silent --show-error --location --output \$BASE/build/lavabit/$APK https://f-droid.org/repo/$APK || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/lavabit/$APK" | sha256sum -c --quiet || exit 1

EOF
done

unzip -qq -c "$INDEX_JAR" index-v1.json | jq -r '[ .packages[] | .[] | select( .packageName == "com.kgurgul.cpuinfo" )] | sort_by(.versionCode) | reverse[0] | [ .apkName, .hash ] | @tsv' | while read APK HASH; do
cat <<-EOF
# Download CPU Info App
[ -d \$BASE/build/cpuinfo/ ] && rm --force --recursive \$BASE/build/cpuinfo/ ; mkdir --parents \$BASE/build/cpuinfo/
curl --fail --silent --show-error --location --output \$BASE/build/cpuinfo/$APK https://f-droid.org/repo/$APK || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = $APK ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "$HASH  \$BASE/build/cpuinfo/$APK" | sha256sum -c --quiet || exit 1

EOF
done

# This termux relased is archived so the name/hash/location won't change.
cat <<-EOF
# Termux version 75 requires at least v21 of the Android SDK.
curl --fail --silent --show-error --location --output \$BASE/build/termux/com.termux_75.apk https://f-droid.org/archive/com.termux_75.apk || \\
{ tput setaf 1 ; printf "An APK download failed. [ FILE = com.termux_75.apk ]\n\n" ; tput sgr0 ; exit 1 ; }
printf "d88444d9df4049c47f12678feb9579aaf2814a89e411d52653dc0a2509f883b5  \$BASE/build/termux/com.termux_75.apk" | sha256sum -c --quiet || exit 1


EOF

rm -f "$INDEX_JAR"

