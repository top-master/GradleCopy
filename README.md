# GradleCopy
Is a `Qt` framework based tool to copy Gradle caches to Android local "m2repository" directory which saves me from redownloading them (again and again).

![Preview](todo/preview.png?raw=true "Windows preview")

## Usage Example
1. First, ensure you have `ANDROID_HOME` environment-variable, pointing to your Android-SDK directory.
2. Create the `init.gradle` file in your `%UserProfile%/.gradle` directory, and open it (in your prefered text-editor).
3. Then, simply copy below and paste in `init.gradle` file (replacing any previous content):

```groovy
ext {
    offlineRepoDir = "${System.getenv("ANDROID_HOME")}\\extras\\m2repository"
}
rootProject {
    buildscript {
        ext {
            offlineRepoDir = offlineRepoDir
        }
    }
    allprojects {
        buildscript {
            repositories {
                maven { url uri(offlineRepoDir) }
            }
        }
        repositories {
            maven { url uri(offlineRepoDir) }
        }
    }
}

settingsEvaluated { settings ->
    settings.pluginManagement {
        buildscript {
            repositories {
                maven { url uri(offlineRepoDir) }
            }
        }
        repositories {
            maven { url uri(offlineRepoDir) }
        }
    }
}

```
4. Download GradleCopy if you didn't already and launch it.
5. Uncheck the `Test Run` option (to skip listing what will be done).
6. At last, click the `Copy` button and done!
7. But sometimes Gradle only downloads `.pom` files, and fails to fetch related `.jar` files:
    - To fix that: Gradle will not even try to download related `.jar` files
      (as long as your offline repo contains related `.pom` file).
    - First, click the "`Generate Missing Jar List`" option from the `Copy` button's dropdown menu.
    - In the resulted window, click "`Disable incomplete lib`" option from the `Download` button's dropdown menu
      (which renames any `.pom` file with no `.jar` file beside it, inside your offline `m2repository` directory only).
    - Previous step (the rename) can be undone by clicking "`Restore incomplete lib`" option (from the same menu).
    - For now, the `Download` button itself is not functional
      (which in future, downloads what is missing, instead of relaying solely on Gradle).

**Platform:** broken `Windows` but should also work for `Mac` and `Linux` as well (once compiled).

**Note:**
All codes are based and meant to be `Apache 2.0` License but it does use the `Qt4` framework under the `LGPL` or `GPL` license, so if you rewrite this using another framework it is pure `Apache 2.0` License
