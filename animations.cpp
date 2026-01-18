      float timeValue = (float)glfwGetTime();
      glUniform1f(glGetUniformLocation(shader.getId(), "time"), timeValue);
      auto DrawAnimatedMesh = [&](Mesh& m, glm::vec3 pos, glm::vec3 scale, bool animated) {
          glm::mat4 model = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), scale);
          glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &(ProjectionMatrix * ViewMatrix * model)[0][0]);
          glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &model[0][0]);
          glUniform1i(glGetUniformLocation(shader.getId(), "isTree"), animated ? 1 : 0);
          m.draw(shader);
          };

      for (const auto& pos : treePositions) {
          DrawAnimatedMesh(treeModel, pos, glm::vec3(3.0f), true);
      }

-------------------------
vertex shader update
      #version 400

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoord;

out vec2 TexCoords; 
out vec3 Normal; 
out vec3 FragPos; 

uniform mat4 MVP;
uniform mat4 model;
uniform float time;
uniform bool isTree;

void main()
{
    TexCoords = texCoord;
    vec3 animatedPos = pos;

    if (isTree) {
        float height = max(0.0, pos.y + 4.0); 
        float mask = pow(height, 2.0) * 0.01; 

        float swaySpeed = 1.2;
        float swayStrength = 0.2;

        animatedPos.x += sin(time * swaySpeed + (pos.y * 0.5)) * mask * swayStrength;
        animatedPos.z += cos(time * swaySpeed * 0.8 + (pos.y * 0.5)) * mask * (swayStrength * 0.5);
    }
    
    FragPos = vec3(model * vec4(animatedPos, 1.0f));
    Normal = mat3(transpose(inverse(model))) * normals;
    gl_Position = MVP * vec4(animatedPos, 1.0f);
}



