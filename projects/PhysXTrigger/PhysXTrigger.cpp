#include "PhysXTrigger.h"
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

bool GLFWMouseButton1Down = false;

PxFoundation * g_PhysicsFoundation = nullptr;
PxPhysics * g_Physics = nullptr;
PxScene * g_PhysicsScene = nullptr;

PxDefaultErrorCallback gDefaultErrorCallback;
PxDefaultAllocator gDefaultAllocatorCallback;
PxSimulationFilterShader gDefaultFilterShader = PxDefaultSimulationFilterShader;
PxMaterial * g_PhysicsMaterial = nullptr;
PxCooking * g_PhysicsCooker = nullptr;

PxActor * volume = nullptr;
PxActor * volume2 = nullptr; 

std::vector<PxRigidActor*> g_PhysXActors;

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

//helper function to convert PhysX matrix to Opengl 
glm::mat4 Px2Glm(PxMat44 m)
{
	glm::mat4 M(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
		m.column1.x, m.column1.y, m.column1.z, m.column1.w,
		m.column2.x, m.column2.y, m.column2.z, m.column2.w,
		m.column3.x, m.column3.y, m.column3.z, m.column3.w);
	return M;
}
//helper function to convert PhysX vector to Opengl 
glm::vec3 Px2GlV3(PxVec3 v1)
{
	glm::vec3 v2;
	v2.x = v1.x;
	v2.y = v1.y;
	v2.z = v1.z;
	return v2;
}
//convert from extended physx vector to opengl vector3
glm::vec3 Px2GLM(PxExtendedVec3 in)
{
	return glm::vec3(in.x, in.y, in.z);
}



PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(filterData0);
	PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	// all initial and persisting reports for everything, with per-point data
	pairFlags = PxPairFlag::eRESOLVE_CONTACTS
		| PxPairFlag::eNOTIFY_TOUCH_FOUND
		| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
		| PxPairFlag::eNOTIFY_CONTACT_POINTS;
	return PxFilterFlag::eDEFAULT;
}

class ContactReportCallback : public PxSimulationEventCallback
{
	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)	{ PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(PxActor** actors, PxU32 count)							{ PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(PxActor** actors, PxU32 count)							{ PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(PxTriggerPair* pairs, PxU32 count)					{ PX_UNUSED(pairs); PX_UNUSED(count); }
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
	{
		for (PxU32 i = 0; i<nbPairs; i++)
		{
			const PxContactPair& cp = pairs[i];

			if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				if ((pairHeader.actors[0] == volume) || (pairHeader.actors[1] == volume))
				{
					printf("Something hit our volume!\n");
				}
			}
		}
	}
};

ContactReportCallback gContactReportCallback;

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

	//add a plane
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi*0.95f, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*g_Physics, pose, PxPlaneGeometry(), *g_PhysicsMaterial);
	//add it to the physX scene
	g_PhysicsScene->addActor(*plane);

	//add a box
	float density = 10;
	PxBoxGeometry box(1, 1, 1);
	PxTransform transform(PxVec3(0, 2, 0));
	PxRigidDynamic* dynamicActor = PxCreateDynamic(*g_Physics, transform, box, *g_PhysicsMaterial, density); 
	//add it to the physX scene
	g_PhysicsScene->addActor(*dynamicActor);
	//add it to our copy of the scene
	g_PhysXActors.push_back(dynamicActor);

	// record which volume will trigger a callback
	volume = dynamicActor;

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

	useBallGun();
	updatePhysX(a_deltaTime);

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

	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// get window dimensions for 2D orthographic projection
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);
	Gizmos::draw2D(glm::ortho<float>(0, width, 0, height, -1.0f, 1.0f));
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

// # PhysX

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
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	sceneDesc.simulationEventCallback = &gContactReportCallback;
	g_PhysicsScene = g_Physics->createScene(sceneDesc);
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
void PhysXTutorial::updatePhysX(float a_deltaTime)
{
	g_PhysicsScene->simulate(a_deltaTime);
	g_PhysicsScene->fetchResults(true);

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

void PhysXTutorial::addWidget(PxShape * pShape, PxRigidActor * actor)
{
	PxGeometryType::Enum type = pShape->getGeometryType();
	switch (type)
	{
	case PxGeometryType::eBOX:
		addBox(pShape, actor);
		break;
	case PxGeometryType::eSPHERE:
		addSphere(pShape, actor);
		break;
	case PxGeometryType::eCAPSULE:
		addCapsule(pShape, actor);
		break;
	}
}
void PhysXTutorial::addBox(PxShape * pShape, PxRigidActor * actor)
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
void PhysXTutorial::addSphere(PxShape * pShape, PxRigidActor * actor)
{
	PxSphereGeometry geometry;
	float radius = 1;
	//get the geometry for this PhysX collision volume
	bool status = pShape->getSphereGeometry(geometry);
	if (status)
	{
		radius = geometry.radius;
	}
	//get the transform for this PhysX collision volume
	PxMat44 m(PxShapeExt::getGlobalPose(*pShape, *actor));
	glm::mat4 M(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
		m.column1.x, m.column1.y, m.column1.z, m.column1.w,
		m.column2.x, m.column2.y, m.column2.z, m.column2.w,
		m.column3.x, m.column3.y, m.column3.z, m.column3.w);
	glm::vec3 position;

	position.x = m.getPosition().x;
	position.y = m.getPosition().y;
	position.z = m.getPosition().z;
	//create a widget to represent it
	Gizmos::addSphere(position, radius, 10, 10, glm::vec4(1, 0, 1, 1));
}
void PhysXTutorial::addCapsule(PxShape* pShape, PxRigidActor * actor)
{
	//creates a gizmo representation of a capsule using 2 spheres and a cylinder
	glm::vec4 colour(0, 0, 1, 1);  //make our capsule blue
	PxCapsuleGeometry capsuleGeometry;
	float radius = 1; //temporary values whilst we try and get the real value from PhysX
	float halfHeight = 1;;
	//get the geometry for this PhysX collision volume
	bool status = pShape->getCapsuleGeometry(capsuleGeometry);
	if (status)
	{
		//this should always happen but just to be safe we check the status flag
		radius = capsuleGeometry.radius; //copy out capsule radius
		halfHeight = capsuleGeometry.halfHeight; //copy out capsule half length
	}
	//get the world transform for the centre of this PhysX collision volume
	PxTransform transform = PxShapeExt::getGlobalPose(*pShape, *actor);
	//use it to create a matrix
	PxMat44 m(transform);
	//convert it to an open gl matrix for adding our gizmos
	glm::mat4 M = Px2Glm(m);
	//get the world position from the PhysX tranform
	glm::vec3 position = Px2GlV3(transform.p);
	glm::vec4 axis(halfHeight, 0, 0, 0);	//axis for the capsule
	axis = M * axis; //rotate axis to correct orientation
	//add our 2 end cap spheres...
	Gizmos::addSphere(position + axis.xyz, radius, 10, 10, colour);
	Gizmos::addSphere(position - axis.xyz, radius, 10, 10, colour);
	//the cylinder gizmo is oriented 90 degrees to what we want so we need to change the rotation matrix...
	glm::mat4 m2 = glm::rotate(M, 11 / 7.0f, glm::vec3(0.0f, 0.0f, 1.0f)); //adds an additional rotation onto the matrix
	//now we can use this matrix and the other data to create the cylinder...
	Gizmos::addCylinderFilled(position, radius, halfHeight, 10, colour, &m2);
}

void PhysXTutorial::useBallGun()
{
	float muzzleSpeed = -50;
	GLFWwindow* window = glfwGetCurrentContext();
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !GLFWMouseButton1Down)
	{
		GLFWMouseButton1Down = true;
		PxSphereGeometry sphere(.5);
		//get the camera position from the camera matrix
		glm::vec3 position(m_cameraMatrix[3]);
		//get the camera rotationfrom the camera matrix
		glm::vec3 direction(m_cameraMatrix[2]);
		physx::PxVec3 velocity = physx::PxVec3(direction.x, direction.y, direction.z)* muzzleSpeed;
		float density = 5;
		PxTransform transform(PxVec3(position.x, position.y, position.z), PxQuat::createIdentity());
		PxRigidDynamic *dynamicActor = PxCreateDynamic(*g_Physics, transform, sphere, *g_PhysicsMaterial, density);
		dynamicActor->setLinearVelocity(velocity, true);
		dynamicActor->setName("the Player's Bullet");
		g_PhysicsScene->addActor(*dynamicActor);
		g_PhysXActors.push_back(dynamicActor);
	}
	if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
	{
		GLFWMouseButton1Down = false;
	}
	int index = 0;
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