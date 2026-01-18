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
