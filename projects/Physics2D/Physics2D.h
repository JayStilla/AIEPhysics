#pragma once

#include "Application.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include <vector>

// derived application class that wraps up all globals neatly
class Physics2D : public Application
{
public:

	Physics2D();
	virtual ~Physics2D();

protected:

	struct DataPoint
	{
		glm::vec3 position;
		glm::vec4 color;

		DataPoint(const glm::vec3& a_position, const glm::vec4& a_color)
			: position(a_position), color(a_color) {}
	};

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	void ClearBalls();
	void Setup();

	void LaunchProjectile(float a_angle, float a_speed, const glm::vec4& a_color);
	void DrawGuide(float a_angle, float a_speed, const glm::vec4& a_color, unsigned int a_segments = 42, float a_segmentTime = 0.1f);

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	static const unsigned int BALL_COUNT = 15;

	Scene* m_scene;
	Actor* m_cueBall;
	Actor* m_balls[BALL_COUNT];
	bool m_aiming;
	bool m_cued;
	float m_threshold;

	std::vector<DataPoint> m_points;
};