#version 330 core
uniform sampler2D tex;
in vec2 texCoords;
in vec2 vertPos;

in LightData
{
    vec4 color;
    vec2 position;
    float intensity;
    float ambient;
    float radius;
} light;

out vec4 fragColor;

void main() 
{
    float dist = distance(light.position, vertPos);

    if (dist < light.radius)
        discard;

    float intensity = (dist / light.radius) * intensity; 

    vec4 color = texture(tex, texCoord) * light.color * intensity;
    if (fragColor.a < 0.1f)
        discard;
}

