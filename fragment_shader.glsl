#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool useTexture;
uniform vec3 objectColor;

// Flashlight
uniform int flashOn;
uniform vec3 flashPos;
uniform vec3 flashDir;
uniform float flashCutOff;

void main()
{
    // 1. Ambient (Brighter Night)
    vec3 ambient = 0.5 * lightColor; // Increased from 0.3

    // 2. Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. Flashlight (Spotlight)
    vec3 spotLight = vec3(0.0, 0.0, 0.0);

    if (flashOn == 1)
    {
        vec3 flashLightDir = normalize(flashPos - FragPos);
        float theta = dot(flashLightDir, normalize(-flashDir));
        
        if (theta > flashCutOff)
        {
            float distance = length(flashPos - FragPos);
            float attenuation = 1.0 / (1.0 + 0.02 * distance + 0.005 * distance * distance); // Stronger beam (lower attenuation)
            
            float spotDiff = max(dot(norm, flashLightDir), 0.0);
            
            // YELLOW COLOR (1.0, 1.0, 0.5) and Brighter (3.0 intensity)
            spotLight = vec3(1.0, 1.0, 0.5) * spotDiff * attenuation * 3.0; 
        }
    }

    vec3 baseColor;
    if (useTexture)
    {
        baseColor = texture(texture_diffuse1, TexCoords).rgb;
    }
    else
    {
        baseColor = objectColor;
    }

    vec3 result = (ambient + diffuse + spotLight) * baseColor;
    FragColor = vec4(result, 1.0);
}