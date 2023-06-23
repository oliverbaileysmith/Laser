#include "Transform.h"

glm::mat4 Transform::Generate(const glm::vec3& translate, float rotateAngle,
    const glm::vec3& rotateAxis, const glm::vec3& scale)
{
    glm::mat4 m(1.0f);
    m = glm::scale(m, scale);
    m = glm::rotate(m, glm::radians(rotateAngle), rotateAxis);
    m = glm::translate(m, translate);
    return m;
}
