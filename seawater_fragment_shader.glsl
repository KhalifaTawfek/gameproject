#version 400

in vec2 textureCoord; 
in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{

    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;


    vec3 normal = normalize(norm);
    vec3 lightDir = normalize(lightPos - fragPos); 
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.7;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 lighting = ambient + diffuse + specular;


    vec4 texColor = texture(texture1, textureCoord);

    vec3 blueTint = vec3(0.2, 0.4, 0.8); 
    float alpha = 0.8;


    vec3 finalColor = mix(texColor.rgb, blueTint, 0.5) * lighting;

    fragColor = vec4(finalColor, alpha); 
}
