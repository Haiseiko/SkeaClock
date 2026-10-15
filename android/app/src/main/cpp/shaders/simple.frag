#version 450
layout(binding = 0) uniform sampler2D uTexture;
layout(location = 0) in vec2 outUV;
layout(location = 0) out vec4 outColor;
void main() {
    vec4 c = texture(uTexture, outUV);
    // Expect premultiplied alpha in textures; output directly
    outColor = c;
}
