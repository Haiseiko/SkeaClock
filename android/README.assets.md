Assets preparation and texture loader

This folder contains helper scripts and a minimal native texture loader skeleton to
support copying the original SkeaClock artwork into the Android app and uploading
it as Vulkan textures.

Steps to prepare original assets (recommended):

1. Create a zip of the original repository's `source` directory (the one used by
   prepare-assets.ps1 in the original project). Name it e.g. `source.zip`.

2. Run the Python preparation script (requires Pillow):

   python3 android/tools/prepare-assets.py --source-zip /path/to/source.zip --out-dir android/app/src/main/assets/processed

   This will extract GIF frames into `android/app/src/main/assets/processed/<gifname>/frame_XXXX.png`
   and copy JSON files under the processed/ tree.

3. Add stb_image to the native build so the texture loader can decode PNGs.
   - Download `stb_image.h` and add it under `android/app/src/main/cpp/`.
   - In `CMakeLists.txt`, add `stb_image.h` (or compile an implementation file) and define HAS_STB_IMAGE.

4. Build the project in Android Studio (or via Gradle). The native code will use
   AAssetManager to access these assets and upload textures to Vulkan.

Notes:
- We opted to include extracted frames (PNG) in the APK for runtime performance and
  pixel-exact reproduction.
- The texture loader here is a minimal skeleton; it intentionally requires `stb_image`.
- After assets are prepared and textures load correctly, we'll implement the Vulkan
  texture upload path and the composition shaders that reproduce the original GDI+
  blend behavior.
