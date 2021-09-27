#version 330 core
in vec4 vertColor;
in vec2 texCoord;
uniform sampler2D tex;
out vec4 fragColor;
void main() {
    fragColor = texture(tex, texCoord) * vertColor;
    if (fragColor.a < 0.1f)
        discard;
}

