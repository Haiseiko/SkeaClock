// texture_loader.h
// Minimal AAssetManager-based PNG loader skeleton. Uses stb_image if available.

#pragma once

#include <android/asset_manager.h>
#include <vector>

// Loads a PNG/JPEG from the APK assets using AAssetManager* mgr and the asset path
// (e.g. "processed/Skea-Idle_1/frame_000.png"). On success returns true and fills
// outPixels (RGBA8), outWidth and outHeight.
// This function expects stb_image.h to be available in the build. If you don't have
// stb_image included yet, add the single-file implementation to your native sources.

bool LoadTextureFromAsset(AAssetManager* mgr, const char* assetPath,
                           int* outWidth, int* outHeight, std::vector<unsigned char>& outPixels);
