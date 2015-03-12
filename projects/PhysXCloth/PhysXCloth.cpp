#include "PhysXCloth.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#include <iostream>
#include <vector>



using namespace physx;

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

PxFoundation * g_PhysicsFoundation = nullptr;
PxPhysics * g_Physics = nullptr;
PxScene * g_PhysicsScene = nullptr;

PxDefaultErrorCallback gDefaultErrorCallback;
PxDefaultAllocator gDefaultAllocatorCallback;
PxSimulationFilterShader gDefaultFilterShader = PxDefaultSimulationFilterShader;
PxMaterial * g_PhysicsMaterial = nullptr;
PxCooking * g_PhysicsCooker = nullptr;
PxClothFabricCooker * g_ClothCooker;

std::vector<PxRigidDynamic*> g_PhysXActors;

class myAllocator : public PxAllocatorCallback
{
public:
	virtual ~myAllocator() {}
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		void* pointer = _aligned_malloc(size, 16);
		return pointer;
	}
	virtual void deallocate(void* ptr)
	{
		_aligned_free(ptr);
	}
};

PhysXTutorial::PhysXTutorial()
{

}

PhysXTutorial::~PhysXTutorial()
{

}

bool PhysXTutorial::onCreate(int a_argc, char* a_argv[])
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

	// get window dimensions to calculate aspect ratio
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, width / (float)height, 0.1f, 1000.0f);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f, 0.25f, 0.25f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	setUpPhysXTutorial();

	// set cloth properties
	float springSize = 0.2f;
	unsigned int springRows = 40;
	unsigned int springCols = 40;

	// this position will represent the top middle vertex
	glm::vec3 clothPosition = glm::vec3(0, 12, 0);

	// shifting grid position for looks
	float halfWidth = springRows * springSize * 0.5f;

	// generate vertices for a grid with texture coordinates
	m_clothVertexCount = springRows * springCols;
	m_clothPositions = new glm::vec3[m_clothVertexCount];
	glm::vec2* clothTextureCoords = new glm::vec2[m_clothVertexCount];
	for (unsigned int r = 0; r < springRows; ++r)
	{
		for (unsigned int c = 0; c < springCols; ++c)
		{
			m_clothPositions[r * springCols + c].x = clothPosition.x + springSize * c;
			m_clothPositions[r * springCols + c].y = clothPosition.y;
			m_clothPositions[r * springCols + c].z = clothPosition.z + springSize * r - halfWidth;

			clothTextureCoords[r * springCols + c].x = 1.0f - r / (springRows - 1.0f);
			clothTextureCoords[r * springCols + c].y = 1.0f - c / (springCols - 1.0f);
		}
	}

	// set up indices for a grid
	m_clothIndexCount = (springRows - 1) * (springCols - 1) * 2 * 3;
	unsigned int* indices = new unsigned int[m_clothIndexCount];
	unsigned int* index = indices;
	for (unsigned int r = 0; r < (springRows - 1); ++r)
	{
		for (unsigned int c = 0; c < (springCols - 1); ++c)
		{
			// indices for the 4 quad corner vertices
			unsigned int i0 = r * springCols + c;
			unsigned int i1 = i0 + 1;
			unsigned int i2 = i0 + springCols;
			unsigned int i3 = i2 + 1;

			// every second quad changes the triangle order
			if ((c + r) % 2)
			{
				*index++ = i0; *index++ = i2; *index++ = i1;
				*index++ = i1; *index++ = i2; *index++ = i3;
			}
			else
			{
				*index++ = i0; *index++ = i2; *index++ = i3;
				*index++ = i0; *index++ = i3; *index++ = i1;
			}
		}
	}

	// set up opengl data for the grid, with the positions as dynamic
	glGenVertexArrays(1, &m_clothVAO);
	glBindVertexArray(m_clothVAO);

	glGenBuffers(1, &m_clothIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_clothIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_clothIndexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &m_clothVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_clothVBO);
	glBufferData(GL_ARRAY_BUFFER, m_clothVertexCount * (sizeof(glm::vec3)), 0, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (char*)0);

	glGenBuffers(1, &m_clothTextureVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_clothTextureVBO);
	glBufferData(GL_ARRAY_BUFFER, m_clothVertexCount * (sizeof(glm::vec2)), clothTextureCoords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1); // texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (char*)0);

	glBindVertexArray(0);

	unsigned int vs = Utility::loadShader("./resources/shaders/basic.vert", GL_VERTEX_SHADER);
	unsigned int fs = Utility::loadShader("./resources/shaders/basic.frag", GL_FRAGMENT_SHADER);
	m_shader = Utility::createProgram(vs, 0, 0, 0, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	m_texture = TextureData("./resources/textures/cloth.png");

	m_cloth = createCloth(clothPosition, m_clothVertexCount, m_clothIndexCount, m_clothPositions, indices);
	g_PhysicsScene->addActor(*m_cloth);

	// texture coords and indices no longer needed
	delete[] clothTextureCoords;
	delete[] indices;

	return true;
}

void PhysXTutorial::onUpdate(float a_deltaTime)
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement(m_cameraMatrix, a_deltaTime, 10);

	// clear all gizmos from last frame
	Gizmos::clear();

	// add an identity matrix gizmo
	Gizmos::addTransform(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));

	// add a 20x20 grid on the XZ-plane
	for (int i = 0; i < 21; ++i)
	{
		Gizmos::addLine(glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10),
			i == 10 ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 0, 1));

		Gizmos::addLine(glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i),
			i == 10 ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 0, 1));
	}

	Gizmos::addAABBFilled(glm::vec3(0, 0, 0), glm::vec3(10, 0, 10), glm::vec4(1, 1, 1, 1));

	updatePhysX();

	// quit our application when escape is pressed
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void PhysXTutorial::onDraw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);

	// bind shader and transforms, along with texture
	glUseProgram(m_shader);

	int location = glGetUniformLocation(m_shader, "projectionView");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix * viewMatrix));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture.textureID);

	// update vertex positions from the cloth
	glBindBuffer(GL_ARRAY_BUFFER, m_clothVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_clothVertexCount * sizeof(glm::vec3), m_clothPositions);

	// disable face culling so that we can draw it double-sided
	glDisable(GL_CULL_FACE);

	// bind and draw the cloth
	glBindVertexArray(m_clothVAO);
	glDrawElements(GL_TRIANGLES, m_clothIndexCount, GL_UNSIGNED_INT, 0);

	glEnable(GL_CULL_FACE);

	// draw the gizmos from this frame
	Gizmos::draw(viewMatrix, m_projectionMatrix);
}

void PhysXTutorial::onDestroy()
{
	cleanUpPhysX();

	// clean up anything we created
	Gizmos::destroy();
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new PhysXTutorial();

	if (app->create("AIE - PhysXTutorial", DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, argc, argv) == true)
		app->run();

	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void PhysXTutorial::setUpPhysXTutorial()
{

	PxAllocatorCallback *myCallback = new myAllocator();
	g_PhysicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, gDefaultErrorCallback);
	g_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_PhysicsFoundation, PxTolerancesScale());
	g_PhysicsCooker = PxCreateCooking(PX_PHYSICS_VERSION, *g_PhysicsFoundation, PxCookingParams(PxTolerancesScale()));

	PxInitExtensions(*g_Physics);
	//create physics material
	g_PhysicsMaterial = g_Physics->createMaterial(0.5f, 0.5f, 0.6f);
	PxSceneDesc sceneDesc(g_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -30.0f, 0);
	sceneDesc.filterShader = gDefaultFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	g_PhysicsScene = g_Physics->createScene(sceneDesc);

	if (g_PhysicsScene)
	{
		std::cout << "start physx scene2";
	}
}

void PhysXTutorial::cleanUpPhysX()
{
	g_PhysicsCooker->release();
	g_PhysicsScene->release();

#ifdef PVD_AVAILABLE
	if (m_PVDConnection)
	{
		m_PVDConnection->release();
	}
#endif
	g_Physics->release();
	g_PhysicsFoundation->release();
}

void PhysXTutorial::updatePhysX()
{
	g_PhysicsScene->simulate(Utility::getDeltaTime());
	while (g_PhysicsScene->fetchResults() == false)
	{
		// dont need to do anything here yet but we still need to do the fetch
	}

	PxClothParticleData* data = m_cloth->lockParticleData();
	for (unsigned int i = 0; i < m_clothVertexCount; ++i)
	{
		m_clothPositions[i] = glm::vec3(data->particles[i].pos.x,
			data->particles[i].pos.y,
			data->particles[i].pos.z);
	}
	data->unlock();

	// Add widgets to represent all physX actors
	for (auto actor : g_PhysXActors)
	{
		PxU32 nShapes = actor->getNbShapes();
		PxShape ** shapes = new PxShape*[nShapes];
		actor->getShapes(shapes, nShapes);

		// render all da shapes
		while (nShapes--)
		{
			addWidget(shapes[nShapes], actor);
		}
		delete[]shapes;
	}
}

void PhysXTutorial::addWidget(PxShape * pShape, PxRigidDynamic * actor)
{
	PxGeometryType::Enum type = pShape->getGeometryType();
	switch (type)
	{
	case PxGeometryType::eBOX:
	{
								 addBox(pShape, actor);
								 break;
	}
	case PxGeometryType::eSPHERE:
	{
									//addSphere(shape, actor);
									break;
	}
	}
}

void PhysXTutorial::addBox(PxShape * pShape, PxRigidDynamic * actor)
{
	// get geo for PHysX collision
	PxBoxGeometry geometry;
	float width = 1, height = 1, length = 1;
	bool status = pShape->getBoxGeometry(geometry);
	if (status)
	{
		width = geometry.halfExtents.x;
		height = geometry.halfExtents.y;
		length = geometry.halfExtents.z;
	}

	// get the transofrm
	PxMat44 m(PxShapeExt::getGlobalPose(*pShape, *actor));	// @AIE missing second func arg


	glm::mat4 M(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
		m.column1.x, m.column1.y, m.column1.z, m.column1.w,
		m.column2.x, m.column2.y, m.column2.z, m.column2.w,
		m.column3.x, m.column3.y, m.column3.z, m.column3.w);

	//get the position out of the transform
	glm::vec3 position;

	position.x = m.getPosition().x;
	position.y = m.getPosition().y;
	position.z = m.getPosition().z;

	glm::vec3 extents = glm::vec3(width, height, length);
	glm::vec4 colour = glm::vec4(1, 0, 0, 1);

	//create our box gizmo
	Gizmos::addAABBFilled(position, extents, colour, &M);
}

PxCloth* PhysXTutorial::createCloth(const glm::vec3& a_position,
	unsigned int& a_vertexCount, unsigned int& a_indexCount,
	const glm::vec3* a_vertices,
	unsigned int* a_indices)
{
	// set up the cloth description
	PxClothMeshDesc clothDesc;
	clothDesc.setToDefault();
	clothDesc.points.count = a_vertexCount;
	clothDesc.triangles.count = a_indexCount / 3;
	clothDesc.points.stride = sizeof(glm::vec3);
	clothDesc.triangles.stride = sizeof(unsigned int)* 3;
	clothDesc.points.data = a_vertices;
	clothDesc.triangles.data = a_indices;

	// cook the geometry into fabric
	PxDefaultMemoryOutputStream buf;
	g_ClothCooker = new PxClothFabricCooker(clothDesc, PxVec3(0, -9.8f, 0));
	g_ClothCooker->save(buf, false);

	PxDefaultMemoryInputData data(buf.getData(), buf.getSize());
	PxClothFabric* fabric = g_Physics->createClothFabric(data);

	// set up the particles for each vertex
	PxClothParticle* particles = new PxClothParticle[a_vertexCount];
	for (unsigned int i = 0; i < a_vertexCount; ++i)
	{
		particles[i].pos = PxVec3(a_vertices[i].x, a_vertices[i].y, a_vertices[i].z);

		// set weights (0 means static)
		if (a_vertices[i].x == a_position.x)
			particles[i].invWeight = 0;
		else
			particles[i].invWeight = 1.f;
	}

	// create the cloth then setup the spring properties
	PxCloth* cloth = g_Physics->createCloth(PxTransform(PxVec3(a_position.x, a_position.y, a_position.z)),
		*fabric, particles, PxClothFlag::eSWEPT_CONTACT);

	// we need to set some solver configurations
	if (cloth != nullptr)
	{
		PxClothStretchConfig bendCfg;
		//bendCfg.solverType = PxClothPhaseSolverConfig::eFAST;
		bendCfg.stiffness = 1;
		bendCfg.stretchLimit = 0.5;
		cloth->setStretchConfig(PxClothFabricPhaseType::eBENDING, bendCfg);
		//cloth->setStretchConfig(PxClothFabricPhaseType::eSTRETCHING, bendCfg);
		cloth->setStretchConfig(PxClothFabricPhaseType::eSHEARING, bendCfg);
		//cloth->setStretchConfig(PxClothFabricPhaseType::eSTRETCHING_HORIZONTAL, bendCfg);
		//cloth->setDampingCoefficient(0.125f);
		cloth->setDampingCoefficient(PxVec3(0.25f, 0.25f, 0.25f));
	}

	delete[] particles;

	return cloth;
}

#ifdef PVD_AVAILABLE
void PhysXTutorial::setUpVisualDebugger()
{
	//@terrehbyte: consider wrapping this in #_DEBUG prep defines

	// check if PvdConnection manager is available on this platform
	if (NULL == g_Physics->getPvdConnectionManager())
		return;

	// setup connection parameters
	const char*     pvd_host_ip = "127.0.0.1";  // IP of the PC which is running PVD
	int             port = 5425;         // TCP port to connect to, where PVD is listening
	unsigned int    timeout = 100;          // timeout in milliseconds to wait for PVD to respond,

	// consoles and remote PCs need a higher timeout.
	PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();

	// and now try to connect
	m_PVDConnection = PxVisualDebuggerExt::createConnection(g_Physics->getPvdConnectionManager(),
		pvd_host_ip, port, timeout, connectionFlags);

}
#endif