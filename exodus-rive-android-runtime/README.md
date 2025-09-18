# @exodus/rive-android-runtime

This package bundles the hardened **rive-android Kotlin runtime** (`.aar`) for use via **npm**.  
It lives inside the [`rive-android`](https://github.com/ExodusMovement/rive-android) fork and replaces the upstream Maven artifact (`app.rive:rive-android`) when used in **@exodus/rive-react-native**.

---

## 📦 Installation

```sh
yarn add @exodus/rive-android-runtime
```

android/settings.gradle

```gradle
include ':rive-android-runtime'
project(':rive-android-runtime').projectDir = new File(rootProject.projectDir, '../node_modules/@exodus/rive-android-runtime/android')
```

android/app/build.gradle

```gradle
dependencies {
    implementation project(':rive-android-runtime')
}
```
