#include "texture_loader.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

// stb_image is expected to be added to the native build (stb_image.h / STB_IMAGE_IMPLEMENTATION)
// If you add stb_image, ensure it's visible to CMake and compiled into the native-lib target.

#ifdef __cplusplus
extern "C" {
#endif

// If STB is available, use it. Otherwise provide a stub that fails gracefully.
#ifdef HAS_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

bool LoadTextureFromAsset(AAssetManager* mgr, const char* assetPath,
                           int* outWidth, int* outHeight, std::vector<unsigned char>& outPixels) {
    if (!mgr || !assetPath) return false;
    AAsset* asset = AAssetManager_open(mgr, assetPath, AASSET_MODE_STREAMING);
    if (!asset) return false;
    off_t size = AAsset_getLength(asset);
    std::vector<unsigned char> buffer(static_cast<size_t>(size));
    int read = AAsset_read(asset, buffer.data(), static_cast<size_t>(size));
    AAsset_close(asset);
    if (read <= 0) return false;

#ifdef HAS_STB_IMAGE
    int channels = 0;
    unsigned char* pixels = stbi_load_from_memory(buffer.data(), static_cast<int>(buffer.size()), outWidth, outHeight, &channels, 4);
    if (!pixels) return false;
    const size_t total = static_cast<size_t>((*outWidth) * (*outHeight) * 4);
    outPixels.assign(pixels, pixels + total);
    stbi_image_free(pixels);
    return true;
#else
    // STB not available: caller must add stb_image.h and define HAS_STB_IMAGE in CMake.
    __android_log_print(ANDROID_LOG_WARN, "texture_loader", "stb_image not present; add it to native build for real loading.");
    return false;
#endif
}

#ifdef __cplusplus
}
#endif
