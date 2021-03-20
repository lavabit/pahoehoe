#!/bin/bash

export ANDROID_AVD_HOME=$HOME/.avd
export ANDROID_SDK_HOME=$HOME/.android
export ANDROID_PREFS_ROOT=$HOME/.android
export ANDROID_HOME=/opt/android-sdk-linux
export ANDROID_SDK_ROOT=/opt/android-sdk-linux

export GRADLE_USER_HOME=$HOME/.gradle
export GRADLE_OPTS=-Dorg.gradle.daemon=false

alias javac='javac -Xlint:-deprecation -Xlint:-unchecked'
export JDK_JAVAC_OPTIONS="-Xlint:-deprecation -Xlint:-unchecked"

# Remove everything that might be left over from a previous build.
cd $HOME && rm --recursive --force $HOME/.cache/Google/ $HOME/.config/Google/ $HOME/.local/share/Google/ $HOME/.cache/Android\ Open\ Source\ Project/ $HOME/.config/Android\ Open\ Source\ Project $HOME/.local/share/Android\ Open\ Source\ Project/
cd $HOME && rm --recursive --force $HOME/.java/ $HOME/.gradle/ $HOME/.android/ $HOME/.cache/go-build/ $HOME/.cache/JNA/
[ -n "`pidof java`" ] && kill `ps -ef | grep -v grep | grep -E "org.gradle.wrapper.GradleWrapperMain|org.gradle.launcher.daemon.bootstrap.GradleDaemon" | awk '{print $2}'`

# [ -d  $HOME/bitmask_android_calyx ] && find $HOME/bitmask_android_calyx -mindepth 1 -depth -exec rm -rf {} \;
# cd $HOME && rm --recursive --force $HOME/bitmask_android_calyx
# git clone https://gitlab.com/CalyxOS/bitmask_android $HOME/bitmask_android_calyx && cd $HOME/bitmask_android_calyx
# git submodule init && git submodule update --init --recursive # git submodule update --force --recursive --init --remote

export VERNUM="200"
export VERSTR="1.0.0-RC1"

[ -d $HOME/bitmask_android_leap ] && find $HOME/bitmask_android_leap -mindepth 1 -depth -exec rm -rf {} \;
git clone https://0xacab.org/leap/bitmask_android $HOME/bitmask_android_leap && cd $HOME/bitmask_android_leap
#git checkout -b local 45d5dcecde2a4af0585346f581aea2ce7884eb5f
#git checkout -b local remotes/origin/fix_custom_preshipping
git submodule init && git submodule update --init --recursive
git tag --delete $VERSTR &> /dev/null | git tag $VERSTR

sed -i "/bitmask_log/d" $HOME/bitmask_android_leap/app/src/main/res/values-vi/strings.xml
sed -i "/bitmask_log/d" $HOME/bitmask_android_leap/app/src/main/res/values-in/strings.xml
sed -i "/bitmask_log/d" $HOME/bitmask_android_leap/app/src/main/res/values-pt/strings.xml
sed -i "/signingup_message/d" $HOME/bitmask_android_leap/app/src/main/res/values-pt/strings.xml
sed -i "/title_activity_main/d" $HOME/bitmask_android_leap/app/src/main/res/values-pt/strings.xml
sed -i "/logged_in_user_status/d" $HOME/bitmask_android_leap/app/src/main/res/values-pt/strings.xml
sed -i "/logged_out_user_status/d" $HOME/bitmask_android_leap/app/src/main/res/values-pt/strings.xml
sed -i "/logging_in_user_status/d" $HOME/bitmask_android_leap/app/src/main/res/values-pt/strings.xml
sed -i "/logging_out_user_status/d" $HOME/bitmask_android_leap/app/src/main/res/values-pt/strings.xml
sed -i "/not_logged_in_user_status/d" $HOME/bitmask_android_leap/app/src/main/res/values-pt/strings.xml
sed -i "/donatewithpaypal/d" $HOME/bitmask_android_leap/app/src/main/res/values-id/strings-icsopenvpn.xml
sed -i "/mobile_info_extended/d" $HOME/bitmask_android_leap/app/src/main/res/values-id/strings-icsopenvpn.xml
# sed -i "s/javac \*\.java/javac -Xlint:deprecation -Xlint:unchecked *.java/g" ics-openvpn/main/src/main/cpp/openvpn3/javacli/build-linux

mkdir --parents $HOME/bitmask_android_leap/ics-openvpn/main/build/ovpnassets

sed -i "/android.enableUnitTestBinaryResources=false/d" $HOME/bitmask_android_leap/ics-openvpn/gradle.properties
printf "android.ndkVersion=21.4.7075529\n" >> $HOME/bitmask_android_leap/gradle.properties
printf "android.ndkVersion=21.4.7075529\n" >> $HOME/bitmask_android_leap/ics-openvpn/gradle.properties
printf "cmake.dir=/opt/android-sdk-linux/cmake/3.10.2.4988404/\n" >> $HOME/bitmask_android_leap/gradle.properties
printf "cmake.dir=/opt/android-sdk-linux/cmake/3.10.2.4988404/\n" >> $HOME/bitmask_android_leap/ics-openvpn/gradle.properties

# printf "android.enableSeparateAnnotationProcessing=false\n" >> $HOME/bitmask_android_leap/gradle.properties
# printf "android.enableSeparateAnnotationProcessing=false\n" >> $HOME/bitmask_android_leap/ics-openvpn/gradle.properties

sed -i "s/org.gradle.jvmargs=-Xmx4096m/org.gradle.jvmargs=-Xmx8192m/g" $HOME/bitmask_android_leap/gradle.properties
sed -i "s/org.gradle.jvmargs=-Xmx4096m/org.gradle.jvmargs=-Xmx8192m/g" $HOME/bitmask_android_leap/ics-openvpn/gradle.properties
sed -i "s/gradle-5.1.1-all.zip/gradle-6.5-all.zip/g" $HOME/bitmask_android_leap/gradle/wrapper/gradle-wrapper.properties
sed -i "s/com.android.tools.build:gradle:3.4.1/com.android.tools.build:gradle:4.1.2/g" $HOME/bitmask_android_leap/build.gradle
#sed -i "s/com.android.tools.build:gradle:4.0.1/com.android.tools.build:gradle:4.1.2/g" $HOME/bitmask_android_leap/ics-openvpn/build.gradle.kts

[ ! -d $HOME/bitmask_android_leap/.run/ ] && mkdir $HOME/bitmask_android_leap/.run/
cat <<-EOF > $HOME/bitmask_android_leap/.run/app.run.xml
<component name="ProjectRunConfigurationManager">
  <configuration default="false" name="app" type="AndroidRunConfigurationType" factoryName="Android App" activateToolWindowBeforeRun="false">
    <module name="bitmask.app" />
    <option name="DEPLOY" value="true" />
    <option name="DEPLOY_APK_FROM_BUNDLE" value="false" />
    <option name="DEPLOY_AS_INSTANT" value="false" />
    <option name="ARTIFACT_NAME" value="" />
    <option name="PM_INSTALL_OPTIONS" value="" />
    <option name="ALL_USERS" value="false" />
    <option name="DYNAMIC_FEATURES_DISABLED_LIST" value="" />
    <option name="ACTIVITY_EXTRA_FLAGS" value="" />
    <option name="MODE" value="default_activity" />
    <option name="CLEAR_LOGCAT" value="true" />
    <option name="SHOW_LOGCAT_AUTOMATICALLY" value="true" />
    <option name="SKIP_NOOP_APK_INSTALLATIONS" value="true" />
    <option name="FORCE_STOP_RUNNING_APP" value="true" />
    <option name="TARGET_SELECTION_MODE" value="DEVICE_AND_SNAPSHOT_COMBO_BOX" />
    <option name="SELECTED_CLOUD_MATRIX_CONFIGURATION_ID" value="-1" />
    <option name="SELECTED_CLOUD_MATRIX_PROJECT_ID" value="" />
    <option name="DEBUGGER_TYPE" value="Auto" />
    <Auto>
      <option name="USE_JAVA_AWARE_DEBUGGER" value="false" />
      <option name="SHOW_STATIC_VARS" value="true" />
      <option name="WORKING_DIR" value="" />
      <option name="TARGET_LOGGING_CHANNELS" value="lldb process:gdb-remote packets" />
      <option name="SHOW_OPTIMIZED_WARNING" value="true" />
      <symbol_dirs symbol_path="\$PROJECT_DIR\$" />
    </Auto>
    <Hybrid>
      <option name="USE_JAVA_AWARE_DEBUGGER" value="false" />
      <option name="SHOW_STATIC_VARS" value="true" />
      <option name="WORKING_DIR" value="" />
      <option name="TARGET_LOGGING_CHANNELS" value="lldb process:gdb-remote packets" />
      <option name="SHOW_OPTIMIZED_WARNING" value="true" />
    </Hybrid>
    <Java />
    <Native>
      <option name="USE_JAVA_AWARE_DEBUGGER" value="false" />
      <option name="SHOW_STATIC_VARS" value="true" />
      <option name="WORKING_DIR" value="" />
      <option name="TARGET_LOGGING_CHANNELS" value="lldb process:gdb-remote packets" />
      <option name="SHOW_OPTIMIZED_WARNING" value="true" />
    </Native>
    <Profilers>
      <option name="ADVANCED_PROFILING_ENABLED" value="false" />
      <option name="STARTUP_PROFILING_ENABLED" value="false" />
      <option name="STARTUP_CPU_PROFILING_ENABLED" value="false" />
      <option name="STARTUP_CPU_PROFILING_CONFIGURATION_NAME" value="Sample Java Methods" />
      <option name="STARTUP_NATIVE_MEMORY_PROFILING_ENABLED" value="false" />
      <option name="NATIVE_MEMORY_SAMPLE_RATE_BYTES" value="2048" />
    </Profilers>
    <option name="DEEP_LINK" value="" />
    <option name="ACTIVITY_CLASS" value="" />
    <option name="SEARCH_ACTIVITY_IN_GLOBAL_SCOPE" value="false" />
    <option name="SKIP_ACTIVITY_VALIDATION" value="false" />
    <method v="2">
      <option name="Android.Gradle.BeforeRunTask" enabled="true" />
    </method>
  </configuration>
</component>
EOF

cat <<-EOF >> $HOME/bitmask_android_leap/build.gradle

/**
allprojects {
  gradle.projectsEvaluated {
    tasks.withType(JavaCompile) {
      options.compilerArgs << "-Xlint:unchecked" << "-Xlint:deprecation"
    }
  }
}
**/

EOF

cat <<-EOF >> $HOME/bitmask_android_leap/ics-openvpn/build.gradle.kts

/**
afterEvaluate {
  tasks.withType(JavaCompile::class) {
    options.compilerArgs.add("-Xlint:unchecked")
    options.compilerArgs.add("-Xlint:deprecation")
  }
}
**/

EOF

patch -p1 <<-EOF
diff --git a/app/build.gradle b/app/build.gradle
index 5b16f8e9..a5c0fe91 100644
--- a/app/build.gradle
+++ b/app/build.gradle
@@ -7,0 +8,2 @@ android {
+  buildToolsVersion "30.0.3"
+  ndkVersion "21.4.7075529"
@@ -16,2 +18,2 @@ android {
-    versionCode 152
-    versionName "1.0.8RC1"
+    versionCode $VERNUM
+    versionName "$VERSTR"
@@ -95 +97,3 @@ android {
-      applicationId "se.leap.riseupvpn"
+      //applicationId "se.leap.pahoehoe"
+      applicationId "com.lavabit.pahoehoe"
+
@@ -97 +101,2 @@ android {
-      appName = "Riseup VPN"
+      appName = "Lavabit Proxy"
+
@@ -99 +104 @@ android {
-      def customProviderUrl = '"https://riseup.net"'
+      def customProviderUrl = '"https://api.proxy.lavabit.com"'
@@ -101,2 +106,3 @@ android {
-      //static ip address of provider, using a commercially validated CA certificate to serve the provider.json
-      def customProviderIp = '"198.252.153.70"'
+
+      //Static ip address of provider, using a commercially validated CA certificate to serve the provider.json.
+      def customProviderIp = '"38.147.122.246"'
@@ -104,2 +110,3 @@ android {
-      //static ip address of the provider api, using a self signed certificate to serve provider.json, eip-service.json etc.
-      def customProviderApiIp = '"198.252.153.107"'
+
+      //Static ip address of the provider api, using a self signed certificate to serve provider.json, eip-service.json etc.
+      def customProviderApiIp = '"38.147.122.246"'
@@ -106,0 +114,3 @@ android {
+
+      //Where to look for location data.
+      //def geoipUrl = '"https://api.proxy.lavabit.com/geoip.json"'
@@ -109,4 +119,6 @@ android {
-      //Change the versionCode as needed
-      //versionCode 1
-      //Change the versionName as needed
-      //versionName "0.9.9RC1"
+
+      //Change the versionCode as needed.
+      versionCode $VERNUM
+
+      //Change the versionName as needed.
+      versionName "$VERSTR"
@@ -120 +132 @@ android {
-      def apkURL = '"https://dl.bitmask.net/RiseupVPN/android/RiseupVPN-Android-latest.apk"'
+      def apkURL = '"https://lavabit.com/files/android/Lavabit-Proxy-latest.apk"'
@@ -121,0 +134 @@ android {
+
@@ -123 +136 @@ android {
-      def signatureURL = '"https://dl.bitmask.net/RiseupVPN/android/RiseupVPN-Android-latest.apk.sig"'
+      def signatureURL = '"https://lavabit.com/files/android/Lavabit-Proxy-latest.apk.sig"'
@@ -124,0 +138 @@ android {
+
@@ -126 +140 @@ android {
-      buildConfigField "String", "version_file_url", '"https://dl.bitmask.net/client/android/versioncode.txt"' //'"https://dl.bitmask.net/RiseupVPN/android/versioncode.txt"'
+      buildConfigField "String", "version_file_url", '"https://lavabit.com/files/android/Lavabit-Proxy-latest.apk.version"'
@@ -131 +145 @@ android {
-      buildConfigField 'String', 'donation_url', '"https://riseup.net/vpn/donate"'
+      buildConfigField 'String', 'donation_url', '"https://lavabit.com/"'
@@ -133 +147 @@ android {
-      buildConfigField 'boolean', 'enable_donation', 'true'
+      buildConfigField 'boolean', 'enable_donation', 'false'
@@ -135 +149 @@ android {
-      buildConfigField 'boolean', 'enable_donation_reminder', 'true'
+      buildConfigField 'boolean', 'enable_donation_reminder', 'false'
@@ -137 +151 @@ android {
-      buildConfigField 'int', 'donation_reminder_duration', '30'
+      buildConfigField 'int', 'donation_reminder_duration', '900'
EOF

patch -p1 <<-EOF
diff --git a/ics-openvpn/main/build.gradle.kts b/ics-openvpn/main/build.gradle.kts
index d55317b0..6b7ef94d 100644
--- a/ics-openvpn/main/build.gradle.kts
+++ b/ice-openvpn/main/build.gradle.kts
@@ -22,0 +23 @@ android {
+        ndkVersion = "21.4.7075529"
@@ -37,0 +39 @@ android {
+            version = "3.10.2"
@@ -195,0 +198,10 @@ dependencies {
+
+/**
+afterEvaluate {
+  tasks.withType(JavaCompile::class) {
+    options.compilerArgs.add("-Xlint:unchecked")
+    options.compilerArgs.add("-Xlint:deprecation")
+  }
+}
+**/
+
EOF

patch -p1 <<-EOF
diff --git a/bitmask-web-core/build.gradle b/bitmask-web-core/build.gradle
index 650f38d6..899f41f7 100644
--- a/bitmask-web-core/build.gradle
+++ b/bitmask-web-core/build.gradle
@@ -4,2 +4,2 @@ android {
-    compileSdkVersion 28
-    buildToolsVersion "29.0.1"
+    compileSdkVersion 30
+    buildToolsVersion "30.0.3"
@@ -10 +10 @@ android {
-        targetSdkVersion 28
+        targetSdkVersion 30
EOF

patch -p1 <<-EOF
diff --git a/scripts/build_deps.sh b/scripts/build_deps.sh
index beb5e13e..ee862e3e 100755
--- a/scripts/build_deps.sh
+++ b/scripts/build_deps.sh
@@ -24 +24 @@ else
-    ./gradlew clean main:externalNativeBuildCleanSkeletonRelease main:externalNativeBuildSkeletonRelease --debug --stacktrace || quit "Build ics-openvpn native libraries failed"
+    ./gradlew clean main:externalNativeBuildCleanSkeletonRelease main:externalNativeBuildSkeletonRelease || quit "Build ics-openvpn native libraries failed"
@@ -33,0 +34 @@ else
+    export ANDROID_NDK_HOME=\$ANDROID_SDK_ROOT/ndk/21.4.7075529
EOF

patch -p1 <<-EOF
diff --git a/app/src/androidTest/legacy/TestEipFragment.java b/app/src/androidTest/legacy/TestEipFragment.java
index 4227f19a..dd4d5bb6 100644
--- a/app/src/androidTest/legacy/TestEipFragment.java
+++ b/app/src/androidTest/legacy/TestEipFragment.java
@@ -42,0 +43 @@ public class TestEipFragment extends BaseTestDashboardFragment {
+        checkProxyLavabitCom();
@@ -47,0 +49,4 @@ public class TestEipFragment extends BaseTestDashboardFragment {
+    private void checkProxyLavabitCom() {
+        checkProvider("proxy.lavabit.com");
+    }
+
diff --git a/app/src/normal/assets/proxy.lavabit.com.json b/app/src/normal/assets/proxy.lavabit.com.json
new file mode 100644
index 00000000..a03d6bbc
--- /dev/null
+++ b/app/src/normal/assets/proxy.lavabit.com.json
@@ -0,0 +1,37 @@
+{
+  "api_uri": "https://api.proxy.lavabit.com",
+  "api_version": "3",
+  "ca_cert_fingerprint": "SHA256: 984e88bb37ef7cc3f0b28a1ef8badf64c6b5c845ed72865f2ba42f26362d68bf",
+  "ca_cert_uri": "https://api.proxy.lavabit.com/ca.crt",
+  "default_language": "en",
+  "description": {
+    "en": "Lavabit Encrypted Proxy"
+  },
+  "domain": "lavabit.com",
+  "enrollment_policy": "open",
+  "languages": [
+    "en"
+  ],
+  "name": {
+    "en": "Lavabit Encrypted Proxy"
+  },
+  "service": {
+    "allow_anonymous": true,
+    "allow_free": true,
+    "allow_limited_bandwidth": false,
+    "allow_paid": false,
+    "allow_registration": false,
+    "allow_unlimited_bandwidth": true,
+    "bandwidth_limit": 102400,
+    "default_service_level": 1,
+    "levels": {
+      "1": {
+        "description": "Please enjoy.",
+        "name": "free"
+      }
+    }
+  },
+  "services": [
+    "openvpn"
+  ]
+}
diff --git a/app/src/normal/assets/urls/proxy.lavabit.com.url b/app/src/normal/assets/urls/proxy.lavabit.com.url
new file mode 100644
index 00000000..c14ef40c
--- /dev/null
+++ b/app/src/normal/assets/urls/proxy.lavabit.com.url
@@ -0,0 +1,5 @@
+{
+       "main_url" : "https://lavabit.com",
+       "provider_ip" : "38.147.122.242",
+       "provider_api_ip" : "38.147.122.246"
+}
diff --git a/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java b/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
index c48f520e..995ee92f 100644
--- a/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
+++ b/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
@@ -66,2 +66,2 @@ public class ProviderManagerTest {
-                    String[] preconfiguredUrls = new String[3];
-                    preconfiguredUrls[0] = "calyx.net.url";
+                    String[] preconfiguredUrls = new String[4];
+                    preconfiguredUrls[0] = "proxy.lavabit.com.url";
@@ -69,0 +70 @@ public class ProviderManagerTest {
+                    preconfiguredUrls[3] = "calyx.net.url";
@@ -104 +105 @@ public class ProviderManagerTest {
-        assertEquals("3 preconfigured, 1 custom provider, 1 dummy provider", 5, providerManager.size());
+        assertEquals("4 preconfigured, 1 custom provider, 1 dummy provider", 6, providerManager.size());
@@ -112 +113 @@ public class ProviderManagerTest {
-        assertEquals("3 preconfigured, 2 custom providers, 1 dummy provider", 6, providerManager.providers().size());
+        assertEquals("4 preconfigured, 2 custom providers, 1 dummy provider", 7, providerManager.providers().size());
@@ -120 +121 @@ public class ProviderManagerTest {
-        assertEquals("3 preconfigured, 1 custom provider, 1 dummy provider", 5, providerManager.providers().size());
+        assertEquals("4 preconfigured, 1 custom provider, 1 dummy provider", 6, providerManager.providers().size());
@@ -128 +129 @@ public class ProviderManagerTest {
-        assertEquals("3 preconfigured, 1 custom provider, 1 dummy provider", 5, providerManager.providers().size());
+        assertEquals("4 preconfigured, 1 custom provider, 1 dummy provider", 6, providerManager.providers().size());
@@ -136 +137 @@ public class ProviderManagerTest {
-        assertEquals("3 preconfigured, 1 custom provider, 1 dummy provider", 5, providerManager.providers().size());
+        assertEquals("4 preconfigured, 1 custom provider, 1 dummy provider", 6, providerManager.providers().size());
@@ -144 +145 @@ public class ProviderManagerTest {
-        assertEquals("3 preconfigured, 0 custom providers, 1 dummy provider", 4, providerManager.providers().size());
+        assertEquals("4 preconfigured, 0 custom providers, 1 dummy provider", 5, providerManager.providers().size());
@@ -152 +153 @@ public class ProviderManagerTest {
-        assertEquals("3 preconfigured, 1 custom providers, 1 dummy provider", 5, providerManager.providers().size());
+        assertEquals("4 preconfigured, 1 custom providers, 1 dummy provider", 6, providerManager.providers().size());
diff --git a/app/src/test/resources/preconfigured/proxy.lavabit.com.json b/app/src/test/resources/preconfigured/proxy.lavabit.com.json
new file mode 100644
index 00000000..a03d6bbc
--- /dev/null
+++ b/app/src/test/resources/preconfigured/proxy.lavabit.com.json
@@ -0,0 +1,37 @@
+{
+  "api_uri": "https://api.proxy.lavabit.com",
+  "api_version": "3",
+  "ca_cert_fingerprint": "SHA256: 984e88bb37ef7cc3f0b28a1ef8badf64c6b5c845ed72865f2ba42f26362d68bf",
+  "ca_cert_uri": "https://api.proxy.lavabit.com/ca.crt",
+  "default_language": "en",
+  "description": {
+    "en": "Lavabit Encrypted Proxy"
+  },
+  "domain": "lavabit.com",
+  "enrollment_policy": "open",
+  "languages": [
+    "en"
+  ],
+  "name": {
+    "en": "Lavabit Encrypted Proxy"
+  },
+  "service": {
+    "allow_anonymous": true,
+    "allow_free": true,
+    "allow_limited_bandwidth": false,
+    "allow_paid": false,
+    "allow_registration": false,
+    "allow_unlimited_bandwidth": true,
+    "bandwidth_limit": 102400,
+    "default_service_level": 1,
+    "levels": {
+      "1": {
+        "description": "Please enjoy.",
+        "name": "free"
+      }
+    }
+  },
+  "services": [
+    "openvpn"
+  ]
+}
diff --git a/app/src/test/resources/preconfigured/urls/proxy.lavabit.com.url b/app/src/test/resources/preconfigured/urls/proxy.lavabit.com.url
new file mode 100644
index 00000000..4986169b
--- /dev/null
+++ b/app/src/test/resources/preconfigured/urls/proxy.lavabit.com.url
@@ -0,0 +1,3 @@
+{
+       "main_url" : "https://api.proxy.lavabit.com"
+}
EOF

cat <<-EOF > $HOME/bitmask_android_leap/app/src/test/resources/preconfigured/proxy.lavabit.com.pem
-----BEGIN CERTIFICATE-----
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
-----END CERTIFICATE-----
EOF

cat <<-EOF > $HOME/bitmask_android_leap/app/src/normal/assets/proxy.lavabit.com.pem
-----BEGIN CERTIFICATE-----
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
-----END CERTIFICATE-----
EOF

patch -p1 <<-EOF
diff --git a/app/src/main/java/se/leap/bitmaskclient/providersetup/connectivity/OkHttpClientGenerator.java b/app/src/main/java/se/leap/bitmaskclient/providersetup/connectivity/OkHttpClientGenerator.java
index 2077a8b9..2b04ee73 100644
--- a/app/src/main/java/se/leap/bitmaskclient/providersetup/connectivity/OkHttpClientGenerator.java
+++ b/app/src/main/java/se/leap/bitmaskclient/providersetup/connectivity/OkHttpClientGenerator.java
@@ -38,0 +39 @@ import java.util.List;
+import java.util.concurrent.TimeUnit;
@@ -123 +124,5 @@ public class OkHttpClientGenerator {
-        OkHttpClient.Builder clientBuilder = new OkHttpClient.Builder();
+        OkHttpClient.Builder clientBuilder = new OkHttpClient.Builder()
+          .connectTimeout(120, TimeUnit.SECONDS)
+          .writeTimeout(120, TimeUnit.SECONDS)
+          .readTimeout(120, TimeUnit.SECONDS)
+          .callTimeout(120, TimeUnit.SECONDS);
EOF

# Remove the RiseUP VPN provider/certificate files bundled with the custom
# build variant.
rm --force $HOME/bitmask_android_leap/app/src/custom/assets/riseup.net.pem
rm --force $HOME/bitmask_android_leap/app/src/custom/assets/riseup.net.json

# Save the Lavabit provider config, and certificate inside the custom build
# directory. Note that the filename must match the "customProviderUrl" supplied
# by the build.gradle file.
curl --silent --insecure --output $HOME/bitmask_android_leap/app/src/custom/assets/api.proxy.lavabit.com.pem https://api.proxy.lavabit.com/ca.crt
curl --silent --cacert $HOME/bitmask_android_leap/app/src/custom/assets/api.proxy.lavabit.com.pem --output $HOME/bitmask_android_leap/app/src/custom/assets/api.proxy.lavabit.com.json https://api.proxy.lavabit.com/provider.json

# Update the Development Fingerprint
ping -c1 -w 1 api.centos.local &>/dev/null && curl --silent --insecure --output /dev/null https://api.centos.local/provider.json
if [ "$?" == "0" ]; then
patch -p1 <<-EOF
diff --git a/app/src/androidTest/legacy/TestEipFragment.java b/app/src/androidTest/legacy/TestEipFragment.java
index dd4d5bb6..8c413735 100644
--- a/app/src/androidTest/legacy/TestEipFragment.java
+++ b/app/src/androidTest/legacy/TestEipFragment.java
@@ -46,0 +47 @@ public class TestEipFragment extends BaseTestDashboardFragment {
+        checkProxyLocal();
@@ -64,0 +66,4 @@ public class TestEipFragment extends BaseTestDashboardFragment {
+    private void checkProxyLocal() {
+        checkProvider("centos.local");
+    }
+
diff --git a/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java b/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
index 995ee92f..9989d0e6 100644
--- a/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
+++ b/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
@@ -66 +66 @@ public class ProviderManagerTest {
-                    String[] preconfiguredUrls = new String[4];
+                    String[] preconfiguredUrls = new String[5];
@@ -70,0 +71 @@ public class ProviderManagerTest {
+                    preconfiguredUrls[4] = "centos.local.url";
EOF
cat <<-EOF > $HOME/bitmask_android_leap/app/src/test/resources/preconfigured/urls/centos.local.url
{
  "main_url" : "https://centos.local",
  "provider_ip" : "192.168.221.242",
  "provider_api_ip" : "192.168.221.246"
}
EOF

curl --silent --insecure https://api.centos.local/provider.json > $HOME/bitmask_android_leap/app/src/test/resources/preconfigured/centos.local.json
curl --silent --insecure https://api.centos.local/ca.crt > $HOME/bitmask_android_leap/app/src/test/resources/preconfigured/centos.local.pem

# Uncomment if you want the centos.local VPN provider to ship with non-debug builds.
# cat <<-EOF > $HOME/bitmask_android_leap/app/src/normal/assets/urls/centos.local.url
# {
#   "main_url" : "https://centos.local",
#   "provider_ip" : "192.168.221.242",
#   "provider_api_ip" : "192.168.221.246"
# }
# EOF
# curl --silent --insecure https://api.centos.local/provider.json > $HOME/bitmask_android_leap/app/src/normal/assets/centos.local.json
# curl --silent --insecure https://api.centos.local/ca.crt > $HOME/bitmask_android_leap/app/src/normal/assets/centos.local.pem
fi

ping -c1 -w 1 api.debian.local &>/dev/null && curl --silent --insecure --output /dev/null https://api.debian.local/provider.json
if [ "$?" == "0" ]; then
patch -p1 <<-EOF
diff --git a/app/src/androidTest/legacy/TestEipFragment.java b/app/src/androidTest/legacy/TestEipFragment.java
index 8c413735..9c34cc0e 100644
--- a/app/src/androidTest/legacy/TestEipFragment.java
+++ b/app/src/androidTest/legacy/TestEipFragment.java
@@ -47,0 +48 @@ public class TestEipFragment extends BaseTestDashboardFragment {
+        checkdebianLocal();
@@ -69,0 +71,4 @@ public class TestEipFragment extends BaseTestDashboardFragment {
+    private void checkdebianLocal() {
+       checkProvider("debian.local");
+    }
+
diff --git a/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java b/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
index d5396704..c5f10db7 100644
--- a/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
+++ b/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
@@ -66 +66 @@ public class ProviderManagerTest {
-                    String[] preconfiguredUrls = new String[5];
+                    String[] preconfiguredUrls = new String[6];
@@ -71,0 +72 @@ public class ProviderManagerTest {
+                    preconfiguredUrls[5] = "debian.local.url";
EOF
cat <<-EOF > $HOME/bitmask_android_leap/app/src/test/resources/preconfigured/urls/debian.local.url
{
  "main_url" : "https://debian.local",
  "provider_ip" : "192.168.221.142",
  "provider_api_ip" : "192.168.221.146"
}
EOF

curl --silent --insecure https://api.debian.local/provider.json > $HOME/bitmask_android_leap/app/src/test/resources/preconfigured/debian.local.json
curl --silent --insecure https://api.debian.local/ca.crt > $HOME/bitmask_android_leap/app/src/test/resources/preconfigured/debian.local.pem

# Uncomment if you want the debian.local VPN provider to ship with non-debug builds.
# cat <<-EOF > $HOME/bitmask_android_leap/app/src/normal/assets/urls/debian.local.url
# {
#   "main_url" : "https://debian.local",
#   "provider_ip" : "192.168.221.142",
#   "provider_api_ip" : "192.168.221.146"
# }
# EOF
# curl --silent --insecure https://api.debian.local/provider.json > $HOME/bitmask_android_leap/app/src/normal/assets/debian.local.json
# curl --silent --insecure https://api.debian.local/ca.crt > $HOME/bitmask_android_leap/app/src/normal/assets/debian.local.pem
fi

patch -p1 <<-EOF
diff --git a/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java b/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
index 8937f907..07b7f90c 100644
--- a/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
+++ b/app/src/test/java/se/leap/bitmaskclient/providersetup/ProviderManagerTest.java
@@ -107 +107 @@ public class ProviderManagerTest {
-        assertEquals("4 preconfigured, 1 custom provider, 1 dummy provider", 6, providerManager.size());
+        assertEquals("6 preconfigured, 1 custom provider, 1 dummy provider", 8, providerManager.size());
@@ -115 +115 @@ public class ProviderManagerTest {
-        assertEquals("4 preconfigured, 2 custom providers, 1 dummy provider", 7, providerManager.providers().size());
+        assertEquals("6 preconfigured, 2 custom providers, 1 dummy provider", 9, providerManager.providers().size());
@@ -123 +123 @@ public class ProviderManagerTest {
-        assertEquals("4 preconfigured, 1 custom provider, 1 dummy provider", 6, providerManager.providers().size());
+        assertEquals("6 preconfigured, 1 custom provider, 1 dummy provider", 8, providerManager.providers().size());
@@ -131 +131 @@ public class ProviderManagerTest {
-        assertEquals("4 preconfigured, 1 custom provider, 1 dummy provider", 6, providerManager.providers().size());
+        assertEquals("6 preconfigured, 1 custom provider, 1 dummy provider", 8, providerManager.providers().size());
@@ -139 +139 @@ public class ProviderManagerTest {
-        assertEquals("4 preconfigured, 1 custom provider, 1 dummy provider", 6, providerManager.providers().size());
+        assertEquals("6 preconfigured, 1 custom provider, 1 dummy provider", 8, providerManager.providers().size());
@@ -147 +147 @@ public class ProviderManagerTest {
-        assertEquals("4 preconfigured, 0 custom providers, 1 dummy provider", 5, providerManager.providers().size());
+        assertEquals("6 preconfigured, 0 custom providers, 1 dummy provider", 7, providerManager.providers().size());
@@ -155 +155 @@ public class ProviderManagerTest {
-        assertEquals("4 preconfigured, 1 custom providers, 1 dummy provider", 6, providerManager.providers().size());
+        assertEquals("6 preconfigured, 1 custom providers, 1 dummy provider", 8, providerManager.providers().size());
EOF

#./build_deps.sh && ./gradlew assemblenormalInsecureFat ; exit

./scripts/build_deps.sh || $( echo build_deps.sh failed ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; exit 1 )
./gradlew --warning-mode none build || $( echo gradlew build failed ;  printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; exit 1 )
./gradlew --warning-mode none assembleDebug || $( echo gradlew assembleDebug failed ;  printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; exit 1 )
./gradlew --warning-mode none assembleRelease || $( echo gradlew assembleRelease failed ;  printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; sleep 1 ; printf "\a" ; exit 1 )

echo "All finished."
sudo fstrim --all
( for i in {1..10}; do printf "\a" ; sleep 1; done ) &

# ./cleanProject.sh
# git clean -f -d -x ; git clean -f -d -x go/src/golang.org/x/mobile/;  git clean -f -d -x  go/src/golang.org/x/mod/; git clean -f -d -x  go/src/golang.org/x/xerrors/

# Broken.
# ./gradlew connectedCheck --warning-mode none
