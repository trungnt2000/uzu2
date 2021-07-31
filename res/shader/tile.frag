#version 330 core
in vec2 texCoord;
uniform sampler2D tex;
void main() {
    vec4 colour =  texture(tex, texCoord); 
    if (colour.a < 0.1)
        discard;
    gl_FragColor = colour; 
}


