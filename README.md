# The Pahoehoe Encrypted Proxy Project
The client and server code used by Lavabit to operate its encrypted proxy service. This repo contains the code for Android client application. It also contains the scripts we use to automate the building, and testing of that app.

These same scripts can also be used to setup local VPN instances for testing, or be adapted to setup production VPN servers for those interested in forking the app, and creating their own variation.

[<img src="https://fdroid.gitlab.io/artwork/badge/get-it-on.png"
     alt="Get it on F-Droid"
     height="80">](https://f-droid.org/packages/com.lavabit.pahoehoe/)
[<img src="https://play.google.com/intl/en_us/badges/images/generic/en-play-badge.png"
     alt="Get it on Google Play"
     height="80">](https://play.google.com/store/apps/details?id=com.lavabit.pahoehoe)

### Building (Easy Method)
To build the code in this repo, clone the repo and execute the `run.sh` script. That script will setup Alma and Debian virtual machines as local VPN servers, which you can then test against, and it will setup a third Debian virtual machine with the dependencies needed to build the Android APK files. If you would like to build the APK without using the Vagrant VMs consult the `debian-10-build-setup.sh` to see what dependencies are required, and the `debian-10-build.sh` for how to build the APK using the command line.

Note that building the APK using Android Studio will fail unless you manually build the VPN libraries using the command line, before importing the project. Use the `android/scripts/build_deps.sh` to build the binary libraries via the command line.

**Release Checklist**   
To create a new release, update the `run.sh` and the `android/app/build.gradle` files. Remove the `RC` from the version string, and commit the code base. Then create a matching `git tag`. While finalizing a release build and generating the files for the Play Store, and website, it's critical that the `VERSTR` and `VERCODE` values in the `run.sh` script match up with the values in `android/app/build.gradle`, at least those values used by the Lavabit variant. And then it's also critical that the procelain commit gets tagged with an identical version string. This will ensure the F-Droid store is updated, and that their version strings match those distributed elsewhere.

Once the release files have been generated, increment the version code and version string values used by the `run.sh` and `android/app/build.gradle` files, while also appending `RC` back onto the end of the version string. Please also note that the `android-11-installer.sh` and `debian-10-emulator.sh` should be updated with the new version string values (including the RC), so they can find properly predict the build filenames to install.

**Direct Access**
For those looking to use the Lavabit proxy servers directly, and without the assistance of our apps, the `lavabitvpn.sh` script may provide a guide. It tries to create a proxy client configuration that can be used directly with any generic OpenVPN client. Unfortunately this script is rather fragile, and may require root permissions on some systems, so it can properly safeguard the configuration it generates. We've also found that even with root, it may not work properly on all distros.

### Tasks
  
- Update the config so the automatic retry window will only grow to a maxium of 10 seconds.
- Randomize which proxy node is used if multiple  nodes are online at a single location.
- Remove the QUERY_ALL_PACKAGES permission from the manifest, or fix and restore access to the excluder.
- Allow users to manually switch to using UDP, instead of TCP.
- Properly implement dnsmasq on the proxy nodes, so that DNS queries get tunneled properly.
- Once the DNS issue is fixed, submit a pull request upstream with our certificate and requisite config info.
- Add a caching layer to the VPNweb daemon to reduce the lag a user might face during setup.
- Adapt the updated description we use on the Play Store for F-droid, and enable the translations.
- Update the OpenVPN version on the server, and along with the embedded OpenVPN client, and enable TLS 1.3.
- Incorporate post quantum cipher support (and allow users to enable their use).
- Update the SDK target from 30 (Android 11) to 32 (Android 12L), or even 33 (Android 13).
- Begin uploading our Android app to the Samsung and Amazon app stores, and allow direct downloads via the Lavabit website.
- Potentially offer the app via APKPure, Uptodow, SlideMe and Aptoide.
  
<sub><sub>The Aurora/Yalp stores are Play Store proxies, and the Huawei, Xiaomi, OPPO and VIVO stores are based in China, where VPN apps are illegal.</sub></sub>
  
**Future Release Work**
Integrate support for the [Google Play Publisher](https://github.com/Triple-T/gradle-play-publisher/blob/master/README.md#managing-play-store-metadata) Gradle radleplugin which will allow us to automate the submission of new builds and the management of metadata in the same way we already integrate with the F-Droid store.

### Security Note
The production releases keys/creation is handled by the `debian-10-build-key.sh` script module. Since this script is likely to contain sensitive information, like the release signing keys, it should never be committed to the repo, or pushed to a server. The git ignore directive should prevent this, but for extra security, consider adding the following script as `.git/hooks/pre-commit` to the production build system git repo:

```
#! /bin/sh -e
git ls-files --cached | grep -qx 'debian-10-build-key.sh' && { echo "The release build keys file has been included in this commit. Remove it and try again." >&2; exit 1; }
exit 0
```

### Upstream Code
While the management, and build scripts for the project were developed by Lavabit, and are unique to this repo, the client and server code wasn't created from nothingness. Rather it is the byproduct of a polygamus relationship between several pre-existing F/OSS projects.

Our Android application code is a fork of the [Bitmask codebase](https://0xacab.org/leap/bitmask_android), which acts as a wrapper around the [ICS OpenVPN project](https://github.com/schwabe/ics-openvpn).

The HTTPS server daemon we use to facilitate client setup, and provide access credentials is an adaptation of the [VPNweb repository](https://0xacab.org/leap/vpnweb/). The server utilizes a several Go modules, all of which are also F/OSS, to function.

All of the code developed by Lavabit is hereby released under the GPL. Most of the code we incorporate, but not all of it, is also available under the GPL. Just beware that some of the code you may find buried in sub-folder may be subject to a different license. Even though the license may not be the GPL, all of the licenses are OSI approved.
