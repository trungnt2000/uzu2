#version 330 core
out vec4 FragColor;
in vec3 color;
in vec2 texCoords;
uniform sampler2D tex0;

void main()
{
  vec4 texColor = texture(tex0, texCoords);
  if(texColor.r > 0.0)
    FragColor = vec4(color, 1.0f);
  else
    discard;
}
