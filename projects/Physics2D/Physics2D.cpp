#include "Physics2D.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Physics2D::Physics2D()
{

}

Physics2D::~Physics2D()
{

}

bool Physics2D::onCreate(int a_argc, char* a_argv[]) 
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(20,20,20),glm::vec3(0,0,0), glm::vec3(0,1,0)) );

	// get window dimensions to calculate aspect ratio
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, width / (float)height, 0.1f, 1000.0f);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	m_scene = new Scene();

	// set up pool table
	Actor::Material felt(1.0f, 0.5f, 2.0f, 2.0f);
	Actor::Material wood(1.0f, 0.9f, 0.9f, 0.9f);
	/*m_scene->AddActor(new Actor(Geometry::Plane(40, 1.0f, glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1)),
								glm::vec4(0, 1, 0, 0), false, felt));/**/
	m_scene->AddActor(new Actor(Geometry::Box(glm::vec3(10, 1, 19.5), glm::vec3(0, -1, 0)),
								glm::vec4(0, 0.625f, 0.125f, 1), false, felt));
	m_scene->AddActor(new Actor(Geometry::Box(glm::vec3(1, 1, 8), glm::vec3(11, 1, 9.5)),
								glm::vec4(0, 0.625f, 0.125f, 1), false, felt));
	m_scene->AddActor(new Actor(Geometry::Box(glm::vec3(1, 1, 8), glm::vec3(11, 1, -9.5)),
								glm::vec4(0, 0.625f, 0.125f, 1), false, felt));
	m_scene->AddActor(new Actor(Geometry::Box(glm::vec3(1, 1, 8), glm::vec3(-11, 1, 9.5)),
								glm::vec4(0, 0.625f, 0.125f, 1), false, felt));
	m_scene->AddActor(new Actor(Geometry::Box(glm::vec3(1, 1, 8), glm::vec3(-11, 1, -9.5)),
								glm::vec4(0, 0.625f, 0.125f, 1), false, felt));
	m_scene->AddActor(new Actor(Geometry::Box(glm::vec3(8, 1, 1), glm::vec3(0, 1, 20.5)),
								glm::vec4(0, 0.625f, 0.125f, 1), false, felt));
	m_scene->AddActor(new Actor(Geometry::Box(glm::vec3(8, 1, 1), glm::vec3(0, 1, -20.5)),
								glm::vec4(0, 0.625f, 0.125f, 1), false, felt));

	// add balls
	m_cueBall = nullptr;
	for (unsigned int i = 0; i < BALL_COUNT; ++i)
		m_balls[i] = nullptr;
	Setup();
	m_threshold = -1.5f;
	
	return true;
}

void Physics2D::ClearBalls()
{
	if (nullptr != m_cueBall)
	{
		m_scene->DestroyActor(m_cueBall);
		m_cueBall = nullptr;
	}
	for (unsigned int i = 0; i < BALL_COUNT; ++i)
	{
		if (nullptr != m_balls[i])
		{
			m_scene->DestroyActor(m_balls[i]);
			m_balls[i] = nullptr;
		}
	}
}
void Physics2D::Setup()
{
	// remove existing balls
	ClearBalls();

	// create balls
	Actor::Material ivory;
	m_cueBall = new Actor(Geometry::Sphere(1, glm::vec3(0, 1, 10)), glm::vec4(1), ivory);
	m_balls[0] = new Actor(Geometry::Sphere(1, glm::vec3(0, 1, -10)), glm::vec4(1, 1, 0, 1), ivory);
	m_balls[1] = new Actor(Geometry::Sphere(1, glm::vec3(1.01, 1, -11.75)), glm::vec4(1, 0, 0, 0.5), ivory);
	m_balls[2] = new Actor(Geometry::Sphere(1, glm::vec3(-1.01, 1, -11.75)), glm::vec4(0, 0, 1, 0.5), ivory);
	m_balls[3] = new Actor(Geometry::Sphere(1, glm::vec3(2.02, 1, -13.5)), glm::vec4(1, 1, 0, 0.5), ivory);
	m_balls[4] = new Actor(Geometry::Sphere(1, glm::vec3(0, 1, -13.5)), glm::vec4(0, 0, 0, 1), ivory);
	m_balls[5] = new Actor(Geometry::Sphere(1, glm::vec3(-2.02, 1, -13.5)), glm::vec4(0, 1, 0, 1), ivory);
	m_balls[6] = new Actor(Geometry::Sphere(1, glm::vec3(3.03, 1, -15.25)), glm::vec4(1, 0.5, 0, 1), ivory);
	m_balls[7] = new Actor(Geometry::Sphere(1, glm::vec3(1.01, 1, -15.25)), glm::vec4(0.5, 0, 1, 0.5), ivory);
	m_balls[8] = new Actor(Geometry::Sphere(1, glm::vec3(-1.01, 1, -15.25)), glm::vec4(1, 0, 0.5, 1), ivory);
	m_balls[9] = new Actor(Geometry::Sphere(1, glm::vec3(-3.03, 1, -15.25)), glm::vec4(1, 0.5, 0, 0.5), ivory);
	m_balls[10] = new Actor(Geometry::Sphere(1, glm::vec3(4.04, 1, -17)), glm::vec4(1, 0, 0.5, 0.5), ivory);
	m_balls[11] = new Actor(Geometry::Sphere(1, glm::vec3(2.02, 1, -17)), glm::vec4(0, 0, 1, 1), ivory);
	m_balls[12] = new Actor(Geometry::Sphere(1, glm::vec3(0, 1, -17)), glm::vec4(0, 1, 0, 0.5), ivory);
	m_balls[13] = new Actor(Geometry::Sphere(1, glm::vec3(-2.02, 1, -17)), glm::vec4(1, 0, 0, 1), ivory);
	m_balls[14] = new Actor(Geometry::Sphere(1, glm::vec3(-4.04, 1, -17)), glm::vec4(0.5, 0, 1, 1), ivory);

	// add balls to scene
	m_scene->AddActor(m_cueBall);
	for (auto ball : m_balls)
		m_scene->AddActor(ball);
	m_aiming = m_cued = false;
}

void Physics2D::LaunchProjectile(float a_angle, float a_speed, const glm::vec4& a_color)
{
	m_scene->AddActor(new Actor(Geometry::Sphere(0.5f, glm::vec3(-20,0,0)), a_color, Actor::Material(),
								glm::vec3(glm::cos(a_angle), glm::sin(a_angle), 0) * a_speed));
}

void Physics2D::DrawGuide(float a_angle, float a_speed, const glm::vec4& a_color, unsigned int a_segments, float a_segmentTime)
{
	for (unsigned int i = 0; i < a_segments; ++i)
	{
		float t1 = a_segmentTime * i;
		float t2 = a_segmentTime * (i+1);
		Gizmos::addLine(glm::vec3(-20.0f + a_speed*t1*glm::cos(a_angle),
								  a_speed*t1*glm::sin(a_angle) - 9.81f*t1*t1/2,
								  0),
						glm::vec3(-20.0f + a_speed*t2*glm::cos(a_angle),
								  a_speed*t2*glm::sin(a_angle) - 9.81f*t2*t2 / 2,
								  0),
						a_color);
	}
}

void Physics2D::onUpdate(float a_deltaTime) 
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement( m_cameraMatrix, a_deltaTime, 10 );

	// clear all gizmos from last frame
	Gizmos::clear();
	
	// add an identity matrix gizmo
	Gizmos::addTransform( glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1) );

	/*DrawGuide(glm::quarter_pi<float>(), 20, glm::vec4(1, 0, 0, 1));
	DrawGuide(glm::pi<float>() / 3, 20, glm::vec4(0, 1, 0, 1));
	DrawGuide(glm::half_pi<float>(), 20, glm::vec4(0, 0, 1, 1));
	DrawGuide(0, 20, glm::vec4(1, 1, 0, 1));

	for (auto point : m_points)
	{
		Gizmos::addSphere(point.position, 0.25f, 2, 4, point.color);
	}

	for (auto actor : m_scene->GetActors())
	{
		m_points.push_back(DataPoint(actor->GetPosition(), actor->GetColor()));
	}/**/
	m_scene->Update();
	m_scene->Render();
	if (!m_cued)
	{
		GLFWwindow* window = glfwGetCurrentContext();
		// get window dimensions to calculate aspect ratio
		int width = 0, height = 0;
		glfwGetWindowSize(m_window, &width, &height);
		double mouseX = 0, mouseY = 0;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		glm::vec3 screenCoord(mouseX, (float)height - mouseY, 0);
		glm::vec4 viewPort = glm::vec4(0.f, 0.f, width, height);
		glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
		glm::vec3 worldPos = glm::unProject(screenCoord, viewMatrix, m_projectionMatrix, viewPort);
		glm::vec3 rayOrigin = m_cameraMatrix[3].xyz();
		glm::vec3 rayDirection = glm::normalize(worldPos - m_cameraMatrix[3].xyz());
		glm::vec3 up(0, 1, 0);
		float d = glm::dot(rayDirection, up);
		if (0 != d)
		{
			glm::vec3 cue = rayOrigin + rayDirection * glm::dot(up - rayOrigin, up) / d;
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
			{
				m_aiming = true;
				Gizmos::addLine(cue, m_cueBall->GetPosition(), glm::vec4(1));
			}
			else if (m_aiming)
			{
				m_aiming = false;
				m_cued = true;
				glm::vec3 aim = m_cueBall->GetPosition() - cue;
				m_cueBall->ApplyImpulse(aim * 2.0f * m_cueBall->GetMass(),
										m_cueBall->GetGeometry().ClosestSurfacePointTo(cue));
				m_cueBall->SetColor(glm::vec4(0.95f, 0.95f, 0.9f, 1));
			}
		}
	}
	else
	{
		if (m_cueBall->GetPosition().y < m_threshold)
			Setup();
		bool still = glm::vec3(0) == m_cueBall->GetVelocity();
		unsigned int remainingBalls = 0;
		for (unsigned int i = 0; i < BALL_COUNT; ++i)
		{
			if (nullptr == m_balls[i])
				continue;
			++remainingBalls;
			if (m_balls[i]->GetPosition().y < m_threshold)
			{
				m_scene->DestroyActor(m_balls[i]);
				m_balls[i] = nullptr;
				--remainingBalls;
			}
			else if (still)
			{
				still = glm::vec3(0) == m_balls[i]->GetVelocity();
			}
		}
		if (0 == remainingBalls)
		{
			Setup();
		}
		else if (still)
		{
			m_aiming = m_cued = false;
			m_cueBall->SetColor(glm::vec4(1));
		}
	}

	// add a 20x20 grid on the XZ-plane
	/*for ( int i = 0 ; i < 21 ; ++i )
	{
		Gizmos::addLine( glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
		
		Gizmos::addLine( glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
	}/**/

	// quit our application when escape is pressed
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void Physics2D::onDraw() 
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// get window dimensions for 2D orthographic projection
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);
	Gizmos::draw2D(glm::ortho<float>(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
}

void Physics2D::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();

	if (nullptr != m_scene)
	{
		delete m_scene;
		m_scene = nullptr;
	}

	m_points.clear();
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Physics2D();
	
	if (app->create("AIE - Physics2D",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}