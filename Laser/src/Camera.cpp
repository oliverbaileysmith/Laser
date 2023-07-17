#include "Camera.h"

#include "Util.h"

Camera::Camera(cl_float verticalFOV, cl_float aspectRatio)
{
    m_Props.VerticalFOV = verticalFOV;
    m_Props.AspectRatio = aspectRatio;

    cl_float theta = m_Props.VerticalFOV * PI / 180.0f;
    cl_float h = tan(theta / 2.0f);
    m_Props.ViewportHeight = 2.0f * h;
    m_Props.ViewportWidth = m_Props.AspectRatio * m_Props.ViewportHeight;

    m_Props.FocalLength = 1.0f;

    m_Props.Position = { 0.0f, 0.0f, 1.0f };
    m_Props.Target = { 0.0f, 0.0f, 0.0f };

    m_Props.UpperLeftCorner = m_Props.Position;
    m_Props.UpperLeftCorner.x -= m_Props.ViewportWidth / 2.0f;
    m_Props.UpperLeftCorner.y += m_Props.ViewportHeight / 2.0f;
    m_Props.UpperLeftCorner.z -= m_Props.FocalLength;
}

Camera::Props Camera::GetProps() const
{
    return m_Props;
}