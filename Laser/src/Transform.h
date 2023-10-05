#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
public:
	glm::mat4 Generate(const glm::vec3 &translate = glm::vec3(0.0f),
		float rotateAngle = 0.0f,
		const glm::vec3 &rotateAxis = glm::vec3(0.0f, 1.0f, 0.0f),
		const glm::vec3 &scale = glm::vec3(1.0f));
};
