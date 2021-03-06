#!/bin/bash

# Copyright (c) 2019 LEAP Encryption Access Project and contributers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


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

function quit {
    echo -e "${RED}Task failed. Exit value: $?.${NC}"
    cleanUp
    exit 1
}

function cleanUp {
    if [[ -f "${ALIGNED_UNSIGNED_APK}" ]]
    then
        rm "${ALIGNED_UNSIGNED_APK}"
    fi
    if [[ -f "${ALIGNED_SIGNED_APK}" ]]
    then
        rm "${ALIGNED_SIGNED_APK}"
    fi
    if [[ -f "${ALIGNED_SIGNED_APK}.idsig" ]]
    then
        rm "${ALIGNED_SIGNED_APK}.idsig"
    fi
}

function sign {
    #---- ALIGN AND JARSIGN APK  -----
    if [[ -z $FILE_NAME_STRING ]]
    then
        FILE_NAME_STRING=$1
        FILE_NAME=${FILE_NAME_STRING##*/} #remove everything till the last '/'
        FILE_DIR=${FILE_NAME_STRING%/*} #remove everything after the last '/'
    fi

    FINAL_APK="${FILE_DIR}/${FILE_NAME}"
    ALIGNED_UNSIGNED_APK="${FILE_DIR}/aligned-${FILE_NAME}"
    ALIGNED_SIGNED_APK="${FILE_DIR}/aligned-signed-${FILE_NAME}"

    echo -e "${GREEN} -> zipalign ${ALIGNED_UNSIGNED_APK}${NC}"f
    # Check whether the APK is aligned already. If so we simply copy it.
    ${ANDROID_BUILD_TOOLS}/zipalign -v -p -c 4 "${FINAL_APK}" > /dev/null && cp "${FINAL_APK}" "${ALIGNED_UNSIGNED_APK}" || quit
    # If the aligned APK file is missing, then it wasn't already aligned, so we make the attempt here.
    [ ! -f "${ALIGNED_UNSIGNED_APK}" ] && { ${ANDROID_BUILD_TOOLS}/zipalign -v -p 4 "${FINAL_APK}" "${ALIGNED_UNSIGNED_APK}" > /dev/null || quit ; }

    echo " -> zipalign successful"

    echo -e "${GREEN} -> apksign ${ALIGNED_UNSIGNED_APK}${NC}"
    ${ANDROID_BUILD_TOOLS}/apksigner sign --debuggable-apk-permitted false --verity-enabled true --v1-signing-enabled true --v2-signing-enabled true --v3-signing-enabled true --v4-signing-enabled true --key "${KEY_STORE_STRING}" --cert "${CERT_STORE_STRING}" --force-stamp-overwrite true --stamp-signer --key "${KEY_STORE_STRING}" --cert "${CERT_STORE_STRING}" --set-auth false --set-rollback false --set-permission false --set-shared-uid false --set-installed-data false --in ${ALIGNED_UNSIGNED_APK} --out ${ALIGNED_SIGNED_APK} || quit

    echo " -> apksign successful"

    echo -e "${GREEN} -> apkverify ${ALIGNED_SIGNED_APK}${NC}"

    CERTPRINT=$(keytool -printcert -file "${CERT_STORE_STRING}" | grep -E 'SHA256: [0-9A-F\:]{95}' | tr -d '[:space:])' | sed 's/^SHA256\:\|\://g') || quit
    FINGERPRINT=$(unzip -p "${ALIGNED_SIGNED_APK}" META-INF/*.RSA | keytool -printcert | grep -E "SHA256: [0-9A-F\:]{95}" | tr -d '[:space:]') || quit
    EXPECTED_FINGERPRINT=$(keytool -printcert -file "${CERT_STORE_STRING}" | grep -E "SHA256: [0-9A-F\:]{95}" | tr -d '[:space:])') || quit

    ${ANDROID_BUILD_TOOLS}/apksigner verify --in "${ALIGNED_SIGNED_APK}" --v4-signature-file "${ALIGNED_SIGNED_APK}.idsig" || quit
    ${ANDROID_BUILD_TOOLS}/apksigner verify --in "${ALIGNED_SIGNED_APK}" --verify-source-stamp true --stamp-cert-digest "${CERTPRINT}" || quit

    if [[ ${FINGERPRINT} == ${EXPECTED_FINGERPRINT} ]]
    then
        echo "    certificate fingerprint is: ${FINGERPRINT}"
    else
        echo -e "${RED}    certificate fingerprint \n${FINGERPRINT} \ndid not match expected certificate fingerprint \n\t${EXPECTED_FINGERPRINT}${NC}"
        quit
    fi

    echo " -> apkverify successful"

    echo -e "${GREEN} -> rename the signed and aligned apk to ${FINAL_APK}${NC}"
    mv "${FINAL_APK}" "${FINAL_APK}.original"
    cp ${ALIGNED_SIGNED_APK} ${FINAL_APK} || quit
    cp ${ALIGNED_SIGNED_APK}.idsig ${FINAL_APK}.idsig || quit
    cleanUp

    echo " -> rename successful"

    #---- GPG SIGNING ----
    if [[ -z ${GPG_KEY} && -z "${GPG_KEY_USER}" ]]
    then
        echo -e "${ORANGE}WARNING: Could not do gpg signing!${NC}"
        exit
    fi

    if [[ ${GPG_KEY} ]]
    then
        echo -e "${GREEN} -> gpg sign ${NC} (using key ${GPG_KEY})"
        gpg --default-key "${GPG_KEY}" --armor --output "${FINAL_APK}.sig" --detach-sign "${FINAL_APK}" || quit
    else
        echo -e "${GREEN} -> gpg sign ${NC} (using key ${GPG_KEY_USER})"
        gpg -u "${GPG_KEY_USER}" --armor --output "${FINAL_APK}.sig" --detach-sign "${FINAL_APK}" || quit
    fi

    echo " -> gpg sign successful"

    echo -e "${GREEN} -> gpg verify ${FINAL_APK}${NC}"
    gpg --quiet --verify "${FINAL_APK}.sig" || quit

    echo " -> gpg verify successful"
    # If the apkanalyzer is available generate the version code files for the website.
    ! which apkanalyzer &> /dev/null || {
        echo -e "${GREEN} -> vercode ${FINAL_APK}${NC}"
        apkanalyzer manifest version-code ${FINAL_APK} > "${FINAL_APK}.version"
        echo " -> vercode successful"
    }

}

# ----Main-----

DO_BUILD=false
DO_SIGN=false
BETA=false
NO_TAG=false
FLAVOR="Normal"
FLAVOR_LOWERCASE="normal"
EXPECTED_FINGERPRINT=""
GREEN='\033[0;32m'
RED='\033[0;31m'
ORANGE='\033[0;33m'
NC='\033[0m'

export GREEN=${GREEN}
export RED=${RED}
export ORANGE=${ORANGE}
export NC=${NC}
export EXPECTED_FINGERPRINT=${EXPECTED_FINGERPRINT}
export -f sign
export -f quit
export -f cleanUp


# init parameters
for ((i=1;i<=$#;i++));
do
    if [[ ${!i} = "b" || ${!i} = "build" ]]
    then
        DO_BUILD=true

    elif [[ ${!i} = "s" || ${!i} = "sign" ]]
    then
        DO_SIGN=true

    elif [[ ${!i} = "-f" || ${!i} = "-file" ]]
    then
        ((i++))
        FILE_NAME_STRING=${!i}
        FILE_NAME=${FILE_NAME_STRING##*/} #remove everything till the last '/'
        FILE_DIR=${FILE_NAME_STRING%/*} #remove everything after the last '/'

    elif [[ ${!i} = "-d" || ${!i} = "-dir" ]]
    then
        ((i++))
        FILE_DIR=${!i}
        MULTIPLE_APKS=true
    elif [[ ${!i} = "-ks" || ${!i} = "-keystore" ]]
    then
        ((i++))
        KEY_STORE_STRING=${!i};
        KEY_STORE_NAME=${KEY_STORE_STRING##*/}
        KEY_STORE_DIR=${KEY_STORE_STRING%/*}
        export KEY_STORE_STRING=${KEY_STORE_STRING}

    elif [[ ${!i} = "-cs" || ${!i} = "-certstore" ]]
    then
        ((i++))
        CERT_STORE_STRING=${!i};
        CERT_STORE_NAME=${CERT_STORE_STRING##*/}
        CERT_STORE_DIR=${CERT_STORE_STRING%/*}
        export CERT_STORE_STRING=${CERT_STORE_STRING}

    elif [[ ${!i} = "-v" || ${!i} = "-version" ]]
    then
        ((i++))
        VERSION_NAME=${!i};
        if [[ -z $(git tag --list | grep -w ${VERSION_NAME}) ]]
        then
            echo -e "${RED}ERROR: Version name has to be a git tag!${NC}"
            exit
        fi
    elif [[ ${!i} = "-k" || ${!i} = "-key" ]];
    then
        ((i++))
        GPG_KEY=${!i}
        export GPG_KEY=${GPG_KEY}
    elif [[ ${!i} = "-u" || ${!i} = "-user" ]];
    then
        ((i++))
        GPG_KEY_USER=${!i}
        export GPG_KEY_USER="${GPG_KEY_USER}"
    elif [[ ${!i} = "-b" || ${!i} = "-beta" ]];
    then
        BETA=true
    elif [[ ${!i} = "-no-tag" ]];
    then
        NO_TAG=true
    elif [[  ${!i} = "-c" || ${!i} = "-custom" ]]
    then
        ((i++))
        FLAVOR="Custom"
        FLAVOR_LOWERCASE="custom"
    elif [[  ${!i} = "-l" || ${!i} = "-lavabit" ]]
    then
        ((i++))
        FLAVOR="Lavabit"
        FLAVOR_LOWERCASE="lavabit"
    elif [[ ${!i} = "-h" || ${!i} = "-help" ]];
    then
        echo -e "
        sign [-ks -fp -f -b -u -k]            sign a given apk (both app signing and GPG signing)
        -ks / -keystore [path] -------------- define path to key for signing (required)
        -cs / -certstore {path} ------------- define path to cert for signing (required)
        -fp / -fingerprint [fingerprint] ---- define the fingerprint for the app (required for non-LEAP
                                              signed apps)
        -f / -file [inputfile] -------------- define path to apk going to be signed
        -d / -dir [path] -------------------- define path to directory including apks to be signed
        -u / -user [gpguser] ---------------- define the gpg user whose key will be used for GPG signing
                                              (optional)
        -k / -key [gpgkey] ------------------ define the key used for GPG signing. Using this option,
                                              -u will be ignored (optional)


        build [-v, -c, -b, -no-tag]
        -v / -version [gittag] -------------- define the git version tag that needs to be checked out
                                              for building. It's also part of the resulting apk file
                                              name. (required if you don't use -no-tag)
        -c / -custom ------------------------ build custom Bitmask client instead of main Bitmask client
                                              (optional)
        -b / -beta -------------------------- build beta version with .beta appended to applicationId (optional)
        -no-tag ----------------------------- force to build current checked out git commit instead of an
                                              official release version


        -h / -help                            print out this help


        example Usages:
        ---------------

        * jarsign only:
        ./prepareForDistribution.sh sign -f app/build/outputs/apk/app-production-beta.apk -ks ~/path/to/bitmask-android.keystore

        * jarsign and gpg sign only:
        ./prepareForDistribution.sh sign -f app/build/outputs/apk/app-production-beta.apk -ks ~/path/to/bitmask-android.keystore -u GPG_USER

        * jarsign and gpg sign all apks in directory:
        ./prepareForDistribution.sh sign -d releases/ -ks ~/path/to/bitmask-android.keystore -u GPG_USER

        * build custom stable
        ./prepareForDistribution.sh build -v 0.9.7 -c

        * build and sign custom stable:
        ./prepareForDistribution.sh build sign -ks ~/path/to/bitmask-android.keystore -u GPG_USER -c -v 0.9.7

        * build and sign custom beta:
        ./prepareForDistribution.sh build sign -ks ~/path/to/bitmask-android.keystore -u GPG_USER -c -b -v 0.9.7RC2

        * build and sign stable:
        ./prepareForDistribution.sh build sign -ks ~/path/to/bitmask-android.keystore -u GPG_USER -v 0.9.7

        * build and sign current git HEAD
        ./prepareForDistribution.sh build sign -ks ~/path/to/bitmask-android.keystore -u GPG_USER -no-tag"
        exit

    else
        echo -e "${RED}Invalid argument: ${!i}${NC}"
        exit
    fi

done;


# check what to do
if [[ ${DO_BUILD} == false && ${DO_SIGN} == false ]]
then
    echo -e "${RED}ERROR: No action set. Please check  ./prepareForDistribution -help!${NC}"
    exit
fi

if [[ ${DO_BUILD} == true ]]
then
    if [[ ${NO_TAG} == false && -z ${VERSION_NAME} ]]
    then
        echo -e "${RED}ERROR: You didn't enter the version (git tag) to be built. If you really want to force building the current checked out commit, use -no-tag.${NC}"
        quit
    fi
    if [[ ${NO_TAG} == false ]]
    then
        #---- COMPARE TAG COMMIT WITH CURRENT COMMIT AND CHECK OUT TAG COMMIT IF NECESSARY ----
        TAG_COMMIT=$(git log -n 1 ${VERSION_NAME} --format="%H")
        CURRENT_COMMIT=$(git log -n 1 --format="%H")
        if [[ ${TAG_COMMIT} != ${CURRENT_COMMIT} ]]
        then
            echo "CHECKING OUT VERSION: ${VERSION_NAME} ..."
            git checkout ${VERSION_NAME} || quit
        fi
    fi

    # ${SCRIPT_DIR}/cleanProject.sh || quit
    ${SCRIPT_DIR}/build_deps.sh || quit
    ${SCRIPT_DIR}/fix_gradle_lock.sh || quit

    cd ${BASE_DIR}
    BASE_OUTPUT_DIR="./app/build/outputs/apk"
    RELEASES_FILE_DIR="./releases"
    if [[ ! -d $RELEASES_FILE_DIR ]]
    then
        mkdir $RELEASES_FILE_DIR
    fi
    rm -rf $RELEASES_FILE_DIR/*

    if [[ ${BETA} == true ]]
    then
        echo "${GREEN} -> build beta releases for flavor ${FLAVOR}${NC}"
        ./gradlew clean assemble${FLAVOR}ProductionFatBeta --stacktrace --console plain || quit
 #       echo "copy file:  $(ls $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionFat/beta/*.apk)"
        cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionFat/beta/*.apk $RELEASES_FILE_DIR/.

        # custom builds might have disabled split apks -> check if build task exist
        if [[ $(./gradlew tasks --console plain | grep ${FLAVOR}ProductionX86Beta) ]]; then
            ./gradlew clean assemble${FLAVOR}ProductionX86Beta --stacktrace --console plain || quit
            cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionX86/beta/*.apk $RELEASES_FILE_DIR/.
        fi
        if [[  $(./gradlew tasks --console plain | grep ${FLAVOR}ProductionX86_64Beta) ]]; then
            ./gradlew clean assemble${FLAVOR}ProductionX86_64Beta --stacktrace --console plain || quit
            cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionX86_64/beta/*.apk $RELEASES_FILE_DIR/.
        fi
        if [[ $(./gradlew tasks --console plain | grep ${FLAVOR}ProductionArmv7Beta) ]]; then
            ./gradlew clean assemble${FLAVOR}ProductionArmv7Beta --stacktrace --console plain || quit
            cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionArmv7/beta/*.apk $RELEASES_FILE_DIR/.
        fi
        if [[ $(./gradlew tasks --console plain | grep ${FLAVOR}ProductionArmv7Beta) ]]; then
            ./gradlew clean assemble${FLAVOR}ProductionArm64Beta --stacktrace --console plain || quit
            cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionArm64/beta/*.apk $RELEASES_FILE_DIR/.
        fi
    else
        echo -e "${GREEN} -> build stable releases for flavor ${FLAVOR}${NC}"
        ./gradlew clean assemble${FLAVOR}ProductionFatRelease --stacktrace --console plain || quit
        cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionFat/release/*.apk $RELEASES_FILE_DIR/.

        ./gradlew clean assemble${FLAVOR}ProductionFatwebRelease --stacktrace --console plain || quit
        cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionFatweb/release/*.apk $RELEASES_FILE_DIR/.

        # custom builds might have disabled split apks -> check if build task exist
        if [[ $(./gradlew tasks --console plain | grep ${FLAVOR}ProductionX86Release) ]]; then
            ./gradlew clean assemble${FLAVOR}ProductionX86Release --stacktrace --console plain || quit
            cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionX86/release/*.apk $RELEASES_FILE_DIR/.
        fi
        if [[ $(./gradlew tasks --console plain | grep ${FLAVOR}ProductionX86_64Release) ]]; then
            ./gradlew clean assemble${FLAVOR}ProductionX86_64Release --stacktrace --console plain || quit
            cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionX86_64/release/*.apk $RELEASES_FILE_DIR/.
        fi
        if [[ $(./gradlew tasks --console plain | grep ${FLAVOR}ProductionArmv7Release) ]]; then
            ./gradlew clean assemble${FLAVOR}ProductionArmv7Release --stacktrace --console plain || quit
            cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionArmv7/release/*.apk $RELEASES_FILE_DIR/.
        fi
        if [[ $(./gradlew tasks --console plain | grep ${FLAVOR}ProductionArm64Release) ]]; then
            ./gradlew clean assemble${FLAVOR}ProductionArm64Release --stacktrace --console plain || quit
            cp $BASE_OUTPUT_DIR/${FLAVOR_LOWERCASE}ProductionArm64/release/*.apk $RELEASES_FILE_DIR/.
        fi
    fi

    cd -
fi

if [[ ${DO_SIGN} == true ]]
then
    cd ${BASE_DIR}
    # check global vars
    if [[ -z ${ANDROID_BUILD_TOOLS} ]]
    then
        echo -e "${RED}ERROR: Environment variable ANDROID_BUILD_TOOLS not set! Please add it to your environment variables. Exiting.${NC}"
        exit
    fi

    if [[ -z ${FILE_NAME} && -z ${FILE_DIR} && ${DO_BUILD} == false ]]
    then
        echo -e "${RED}ERROR: Sign only needs a file name or a directory. Please check ./prepareForDistribution -help!${NC}"
        exit
    fi
    if [[ -z ${KEY_STORE_NAME} ]]
    then
        echo -e "${RED}ERROR: Key store not set. Please check ./prepareForDistribution -help${NC}"
        exit
    fi
    if [[ -n ${FILE_NAME_STRING} && ${DO_BUILD} == true ]]
    then
        echo -e "${ORANGE}WARNING: Ignoring parameter -file. Built APK will be used instead.${NC}"
    fi

    #---- OPT: SELECT APK FROM LAST BUILD ----
    if [[ ${DO_BUILD} == true ]]
    then
        FILE_DIR=$RELEASES_FILE_DIR
        echo -e "${GREEN} -> sign apks:${NC}"
        ls -w 1 $FILE_DIR/*\.apk | xargs -I {} echo {}
        xargs -I _ -ra <(ls -w 1 $FILE_DIR/*\.apk) bash -c 'sign _'
    elif [[ ${MULTIPLE_APKS} == true ]]
    then
        echo -e "${GREEN} -> sign apks:${NC}"
        ls -w 1 $FILE_DIR/*\.apk | xargs -I {} echo {}
        xargs -I _ -ra <(ls -w 1 $FILE_DIR/*\.apk) bash -c 'sign _'
    else
        echo -e "${GREEN} -> sign apk: ${FILE_NAME_STRING}${NC}"
        sign $FILE_NAME_STRING
    fi
    cd -
fi
