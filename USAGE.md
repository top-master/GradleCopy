
**Note** that although below works great for a single project,
for a global solution see [Usage-Example section of README](README.md#Usage-Example) file.

## Usage Example
Below is an almost copy of the `react-native` framework's root `build.gradle` file (not `app/build.gradle`);

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
        kotlin_version = '1.3.21'
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

        // Below "kotlin" is required in root "build.gradle" else offline-repo will not get searched.
        classpath "org.jetbrains.kotlin:kotlin-gradle-plugin:$kotlin_version"

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

// Something like below needs to be before your Gradle "apply" statements.
//
subprojects {
    buildscript {
        repositories {
            try { maven { url uri(offlineRepoDir) } } catch (Throwable e) {}
        }
    }
    allprojects {
        repositories {
            try { maven { url uri(offlineRepoDir) } } catch (Throwable e) {}
        }
    }
    // Force all subprojects to use the same build tools
    afterEvaluate {project ->
        if (project.hasProperty("android")) {
            android {
                compileSdkVersion rootProject.ext.compileSdkVersion
                buildToolsVersion rootProject.ext.buildToolsVersion
            }
        }
    }
}
```
