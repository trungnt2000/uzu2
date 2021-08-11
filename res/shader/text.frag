#version 330 core
out vec4 FragColor;
in vec4 color;
in vec2 texCoords;
uniform sampler2D tex0;

void main()
{
  vec4 texColor = texture(tex0, texCoords);
  FragColor = color;
  if(texColor.r == 0.0)
    FragColor.a = 0.0;
}
