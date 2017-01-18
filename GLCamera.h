#pragma once

#include <angel\Angel.h>
#include <iostream>

class GLCamera
{

public:
	GLCamera();
	GLCamera(const vec3 pos, const vec3 target, const vec3 up, GLuint shader1, GLuint shader2, GLuint shader3);

	mat4 getModelViewMatrix();
	vec3 getCamPosition();

	void setModelViewMatrix();
	void resetModelViewMatrix();

	void slide(float du, float dv, float dn);
	void roll(float angle);
	void yaw(float angle);
	void pitch(float angle);

	float getDist();

private:
	vec3 m_pos;
	vec3 m_target;
	vec3 m_up;
	vec3 u,v,n;

	mat4 model_view;

	GLuint model_shader1;
	GLuint model_shader2;
	GLuint model_shader3;

	vec3 m_pos_bak;
	vec3 m_target_bak;
	vec3 m_up_bak;
	vec3 u_bak, v_bak, n_bak;
};