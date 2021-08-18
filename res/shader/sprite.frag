#version 330 core
in vec4 vertColor;
in vec2 texCoord;
uniform sampler2D tex;
void main() {
    gl_FragColor = texture(tex, texCoord) * vertColor;
    if (gl_FragColor.a < 0.1f)
        discard;
}

