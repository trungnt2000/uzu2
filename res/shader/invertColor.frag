
#version 330 core
in vec4 vertColor;
in vec2 texCoord;
uniform sampler2D tex;
out vec4 fragColor;
void main() {
    vec4 color = texture(tex, texCoord);
    fragColor = vec4(1.f - color.r, 1.f - color.g, 1.f - color.b, color.a);
    if (fragColor.a < 0.1f)
        discard;
}

