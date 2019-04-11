# GradleCopy
Is a `Qt` framework based tool to copy Gradle caches to Android local "m2repository" directory which saves me from redownloading them (again and again).

![Preview](todo/preview.png?raw=true "Windows preview")

## Usage Example
Below is an almost copy of the `react-native` framework's `build.gradle` file;

But note the lines with "`offlineRepoDir`" in them, since that's where the magic happens:
```groovy
// Top-level build file where you can add configuration options common to all sub-projects/modules.

buildscript {
    ext {
        buildToolsVersion = "28.0.3"
        minSdkVersion = 16
        compileSdkVersion = 28
        targetSdkVersion = 28
        supportLibVersion = "28.0.0"
        // Finds our custom maven2-repository path
        offlineRepoDir = System.getenv("ANDROID_HOME")
        offlineRepoDir = "$offlineRepoDir/extras/m2repository"
    }
    repositories {
        try { maven { url uri(offlineRepoDir) } } catch (Throwable e) {}
        google()
        jcenter()
        mavenCentral()
    }
    dependencies {
        classpath 'com.android.tools.build:gradle:3.3.1'

        // NOTE: Do not place your application dependencies here; they belong
        // in the individual module build.gradle files
    }
}

allprojects {
    repositories {
        try { maven { url uri(offlineRepoDir) } } catch (Throwable e) {}
        maven {
            // All of React Native (JS, Obj-C sources, Android binaries) is installed from npm
            url "$rootDir/../node_modules/react-native/android"
        }
        mavenLocal()
        google()
        jcenter()
    }
    buildDir = "${rootProject.rootDir}/.build/${project.name}"
}
```

**Platform:** broken `Windows` but should also work for `Mac` and `Linux`

**Note:**
All codes are based and meant to be `Apache 2.0` License but it does use the `Qt4` framework under the `LGPL` or `GPL` license, so if you rewrite this using another framework it is pure `Apache 2.0` License
