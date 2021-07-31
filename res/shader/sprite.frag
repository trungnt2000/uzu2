#version 330 core
in vec4 vertColor;
in vec2 texCoord;
uniform sampler2D tex;
void main() {
    gl_FragColor = vertColor * texture(tex, texCoord);
}

