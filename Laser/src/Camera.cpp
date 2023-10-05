#include "Camera.h"

#include <glm/glm.hpp>

#include "Util.h"

Camera::Camera(cl_float3 position, cl_float3 target, cl_float verticalFOV,
	cl_float aspectRatio, cl_float aperture, cl_float focusDistance)
{
	m_Props.VerticalFOV = verticalFOV;
	m_Props.AspectRatio = aspectRatio;
	m_Props.Position = position;
	m_Props.Target = target;
	m_Props.LensRadius = aperture / 2.0f;

	// Use FOV to calculate viewport dimensions
	cl_float theta = m_Props.VerticalFOV * PI / 180.0f;
	cl_float h = tan(theta / 2.0f);
	cl_float viewportHeight = 2.0f * h;
	cl_float viewportWidth = m_Props.AspectRatio * viewportHeight;

	// Calculate basis for camera orientation
	glm::vec3 worldUp = {0.0f, 1.0f, 0.0f};
	glm::vec3 glmPosition = {position.x, position.y, position.z};
	glm::vec3 glmTarget = {target.x, target.y, target.z};

	glm::vec3 w = glm::normalize(glmPosition - glmTarget);
	glm::vec3 u = glm::normalize(glm::cross(worldUp, w));
	glm::vec3 v = glm::cross(w, u);

	// Upper left corner of viewport for generating rays
	glm::vec3 horizontal = focusDistance * viewportWidth * u;
	glm::vec3 vertical = focusDistance * viewportHeight * v;
	glm::vec3 upperLeftCorner =
		glmPosition - horizontal / 2.0f + vertical / 2.0f - focusDistance * w;
	m_Props.UpperLeftCorner = {upperLeftCorner.x, upperLeftCorner.y,
		upperLeftCorner.z};
	m_Props.ViewportHorizontal = {horizontal.x, horizontal.y, horizontal.z};
	m_Props.ViewportVertical = {vertical.x, vertical.y, vertical.z};
	m_Props.u = {u.x, u.y, u.z};
	m_Props.v = {v.x, v.y, v.z};
	m_Props.w = {w.x, w.y, w.z};
}

Camera::Props Camera::GetProps() const
{
	return m_Props;
}
