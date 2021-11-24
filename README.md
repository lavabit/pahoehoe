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
To build the code in this repo, clone the repo and execute the `run.sh` script. That script will setup CentOS and Debian virtual machines as local VPN servers, which you can then test against, and it will setup a third Debian virtual machine with the dependencies needed to build the Android APK files. If you would like to build the APK without using the Vagrant VMs consult the `debian-10-build-setup.sh` to see what dependencies are required, and the `debian-10-build.sh` for how to build the APK using the command line.

Note that building the APK using Android Studio will fail unless you manually build the VPN libraries using the command line, before importing the project. Use the `android/scripts/build_deps.sh` to build the binary libraries via the command line.

### Direct Access
The `lavabitvpn.sh` script demonstrates how to use retrieve the client config, and generate access keys for the proxy service without the client apps and then connect using the OpenVPN binary. Unfortunately this script currently requires root permissions so it can properly secure itself. We've also found that even with root, it may not work properly on all distros.

### Security Note
The production releases keys/creation is handled by the `debian-10-build-key.sh` script module. Since this script is likely to contain sensitive information, like the release signing keys, it should never be committed to the repo, or pushed to a server. The git ignore directive should prevent this, but for extra security, consider adding the following script as `.git/hooks/pre-commit` to the production build system git repo:

```
#! /bin/sh -e
git ls-files --cached | grep -qx 'debian-10-build-key.sh' && { echo "The release build keys file has been included in this commit. Remove it and try again." >&2; exit 1; }
exit 0
```

### Upstream Code
While the management, and build scripts for the project were developed by Lavabit, and are unique to this repo, the client and server code wasn't created scratch. Rather it created by modifying code found in other pre-existing F/OSS projects.

Most notably, the Android application code was based off the [Bitmask codebase](https://0xacab.org/leap/bitmask_android), which in turn relied on a number of F/OSS projects for support, with the the most significant contribution to the Android application coming from the [ICS OpenVPN project](https://github.com/schwabe/ics-openvpn).

The HTTP server daemon used to facilitate client setup and access was also adapted from existing code found in the [VPNweb repository](https://0xacab.org/leap/vpnweb/). This daemon is written in Go, and relies on F/OSS Go modules to function.

While the code developed by Lavabit has been released, under the GPL, and most of the incorporated code has also been released under the GPL, there may be modules and/or code found inside sub-folders that is subject to a different license. 
