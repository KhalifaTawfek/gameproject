#version 400

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoord;

out vec2 textureCoord;
out vec3 norm;
out vec3 fragPos;

uniform mat4 MVP;
uniform mat4 model;
uniform float time; 

void main()
{
    textureCoord = texCoord;


    vec3 displacedPos = pos;
    float sway = sin(time + pos.y * 0.5) * 0.1; // Sway effect based on time and height
    displacedPos.x += sway; // Displace horizontally (X-axis)
    displacedPos.z += sway; // Displace horizontally (Z-axis)

    fragPos = vec3(model * vec4(displacedPos, 1.0f));
    norm = mat3(transpose(inverse(model))) * normals;
    gl_Position = MVP * vec4(displacedPos, 1.0f);
}
