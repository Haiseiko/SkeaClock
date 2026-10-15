# Android native prototype for SkeaClock

This branch contains an initial Android native scaffold using JNI and a small C++ native library.

What's included:
- Minimal Android Studio Gradle project under android/app
- Native C++ library (native-lib) built with CMake
- MainActivity + SurfaceView drawing a basic clock using native timing/angle calculations

How to build:
1. Open android/ in Android Studio
2. Build & Run on device or emulator (minSdk 21)

Notes & next steps:
- This is a prototype to validate native integration and basic rendering.
- Next: integrate existing C++ logic for alarm/timers/layout, replace simple drawing with asset-based artwork rendering, and add audio playback.
