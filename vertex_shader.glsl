#version 400

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoord;

// OUTPUTS MUST MATCH FRAGMENT SHADER INPUTS EXACTLY
out vec2 TexCoords; 
out vec3 Normal;   // Renamed from 'norm'
out vec3 FragPos;  // Renamed from 'fragPos' (Capital P)

uniform mat4 MVP;
uniform mat4 model;

void main()
{
    TexCoords = texCoord;
    
    // Assign to the corrected output names
    FragPos = vec3(model * vec4(pos, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normals;
    
    gl_Position = MVP * vec4(pos, 1.0f);
}