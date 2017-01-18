//-------------------------------------------------------------------------------------------------
// Free Camera Class
// Adapted by Raymond.X
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "GLCamera.h"

GLCamera::GLCamera()
{

}

GLCamera::GLCamera(const vec3 pos, const vec3 target, const vec3 up, GLuint shader1, GLuint shader2, GLuint shader3)
{
	m_pos = pos;
	m_target = target;
	m_up = up;
	n = vec3(pos.x - target.x, pos.y - target.y, pos.z - target.z);
	u = vec3(cross(up ,n).x, cross(up, n).y, cross(up, n).z);
	v = vec3(cross(n, u).x, cross(n, u).y, cross(n, u).z);

	n = normalize(n);
	u = normalize(u);
	v = normalize(v);

	// Link current object shaders
	model_shader1 = shader1;
	model_shader2 = shader2;
	model_shader3 = shader3;

	setModelViewMatrix();

	// Backup parameters
	m_pos_bak = m_pos;
	m_target_bak = m_target;
	m_up_bak = m_up;
	n_bak = n;
	u_bak = u;
	v_bak = v;
}

// Get current view matrix
mat4 GLCamera::getModelViewMatrix()
{
	return model_view;
}

// Get current camera position
vec3 GLCamera::getCamPosition()
{
	return m_pos;
}

// Update view matrix
void GLCamera::setModelViewMatrix()
{
	model_view = {
	u.x, u.y, u.z, -dot(m_pos, u),
	v.x, v.y, v.z, -dot(m_pos, v),
	n.x, n.y, n.z, -dot(m_pos, n),
	0, 0, 0, 1.0 };

	glUseProgram(model_shader1);
	glUniformMatrix4fv(glGetUniformLocation(model_shader1, "ModelView"), 1, GL_TRUE, model_view);
	glUseProgram(model_shader2);
	glUniformMatrix4fv(glGetUniformLocation(model_shader2, "ModelView"), 1, GL_TRUE, model_view);
	glUseProgram(model_shader3);
	glUniformMatrix4fv(glGetUniformLocation(model_shader3, "ModelView"), 1, GL_TRUE, model_view);
	glUseProgram(0);
}

// Reset camera position
void GLCamera::resetModelViewMatrix()
{
	m_pos = m_pos_bak;
	m_target = m_target_bak;
	m_up = m_up_bak;
	n = n_bak;
	u = u_bak;
	v = v_bak;

	setModelViewMatrix();
}

// Slide camera
void GLCamera::slide(float du, float dv, float dn)
{
	m_pos[0] = m_pos[0] + du*u.x + dv*v.x + dn*n.x;
	m_pos[1] = m_pos[1] + du*u.y + dv*v.y + dn*n.y;
	m_pos[2] = m_pos[2] + du*u.z + dv*v.z + dn*n.z;
	m_target[0] = m_target[0] + du*u.x + dv*v.x + dn*n.x;
	m_target[1] = m_target[0] + du*u.y + dv*v.y + dn*n.y;
	m_target[2] = m_target[0] + du*u.z + dv*v.z + dn*n.z;

	setModelViewMatrix();
}

// Roll, Pitch, Yaw Rotation

void GLCamera::roll(float angle)
{
	float cs = cos(angle*DegreesToRadians * 2);
	float sn = sin(angle*DegreesToRadians * 2);
	vec3 t(u);
	vec3 s(v);
	u.x = cs*t.x - sn*s.x;
	u.y = cs*t.y - sn*s.y;
	u.z = cs*t.z - sn*s.z;

	v.x = sn*t.x + cs*s.x;
	v.y = sn*t.y + cs*s.y;
	v.z = sn*t.z + cs*s.z;

	setModelViewMatrix();
}

void GLCamera::pitch(float angle)
{
	float cs = cos(angle*DegreesToRadians * 2);
	float sn = sin(angle*DegreesToRadians * 2);
	vec3 t(v);
	vec3 s(n);

	v.x = cs*t.x - sn*s.x;
	v.y = cs*t.y - sn*s.y;
	v.z = cs*t.z - sn*s.z;

	n.x = sn*t.x + cs*s.x;
	n.y = sn*t.y + cs*s.y;
	n.z = sn*t.z + cs*s.z;

	setModelViewMatrix();
}

void GLCamera::yaw(float angle)
{
	float cs = cos(angle*DegreesToRadians * 2);
	float sn = sin(angle*DegreesToRadians * 2);
	vec3 t(n);
	vec3 s(u);

	n.x = cs*t.x - sn*s.x;
	n.y = cs*t.y - sn*s.y;
	n.z = cs*t.z - sn*s.z;

	u.x = sn*t.x + cs*s.x;
	u.y = sn*t.y + cs*s.y;
	u.z = sn*t.z + cs*s.z;

	setModelViewMatrix();
}

// Get current distance from the origin
float GLCamera::getDist()
{
	float dist = pow(m_pos.x, 2) + pow(m_pos.y, 2) + pow(m_pos.z, 2);

	return pow(dist, 0.5);
}