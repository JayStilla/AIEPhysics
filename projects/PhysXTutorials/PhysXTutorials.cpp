#include "PhysXTutorials.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>


using namespace physx; 

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

PxFoundation* g_PhysicsFoundation = nullptr;
PxPhysics* g_Physics = nullptr;
PxScene* g_PhysicsScene = nullptr;

PxDefaultErrorCallback gDefaultErrorCallback;
PxDefaultAllocator gDefaultAllocatorCallback;
PxSimulationFilterShader gDefaultFilterShader = PxDefaultSimulationFilterShader;
PxMaterial* g_PhysicsMaterial = nullptr;
PxCooking* g_PhysicsCooker = nullptr;
std::vector<PxRigidDynamic*> g_PhysXActors;

PhysXTutorials::PhysXTutorials()
{

}

PhysXTutorials::~PhysXTutorials()
{

}

bool PhysXTutorials::onCreate(int a_argc, char* a_argv[]) 
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse( glm::lookAt(glm::vec3(10,10,10),glm::vec3(0,0,0), glm::vec3(0,1,0)) );

	// get window dimensions to calculate aspect ratio
	//int width = 0, height = 0;
	//glfwGetWindowSize(m_window, &width, &height);

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, DEFAULT_SCREENWIDTH / (float)DEFAULT_SCREENHEIGHT, 0.1f, 1000.0f);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f,0.25f,0.25f,1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	PxSimulationEventCallback* mycollisionCallBack = new MycollisionCallBack();  //instantiate our class to overload call backs
//	g_PhysicsScene->setSimulationEventCallback(mycollisionCallBack); //tell the scene to use our call back class

	PxCapsuleGeometry capsule(1, 2);
	//set it's initial transform and set it vertical.
	startingPlayerPos = PxTransform(PxVec3(0, 10, 0), PxQuat(11 / 7.0f, PxVec3(0, 0, 1)));
	playerActor = PxCreateDynamic(*g_Physics, startingPlayerPos, capsule, *g_PhysicsMaterial, 1);
	//add it to the physX scene
	g_PhysicsScene->addActor(*playerActor);
	g_PhysXActors.push_back(playerActor);
	playerActor->setName("Player");

	setUpPhysXTutorial();
	tutorial_1();

	return true;
}

void PhysXTutorials::onUpdate(float a_deltaTime) 
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement( m_cameraMatrix, a_deltaTime, 10 );

	// clear all gizmos from last frame
	Gizmos::clear();
	
	// add an identity matrix gizmo
	Gizmos::addTransform( glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1) );

	// add a 20x20 grid on the XZ-plane
	for ( int i = 0 ; i < 21 ; ++i )
	{
		Gizmos::addLine( glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
		
		Gizmos::addLine( glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i), 
						 i == 10 ? glm::vec4(1,1,1,1) : glm::vec4(0,0,0,1) );
	}

	upDatePhysx();

	// quit our application when escape is pressed
	if (glfwGetKey(m_window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void PhysXTutorials::onDraw() 
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse( m_cameraMatrix );
	
	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// get window dimensions for 2D orthographic projection
	//int width = 0, height = 0;
	//glfwGetWindowSize(m_window, &width, &height);
	//Gizmos::draw2D(glm::ortho<float>(0, width, 0, height, -1.0f, 1.0f));
}

void PhysXTutorials::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();

	cleanUpPhsyx();
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new PhysXTutorials();
	
	if (app->create("AIE - PhysXTutorials",DEFAULT_SCREENWIDTH,DEFAULT_SCREENHEIGHT,argc,argv) == true)
		app->run();
	
		
	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void PhysXTutorials::setUpPhysXTutorial()
{
	PxAllocatorCallback *myCallback = new myAllocator();
	g_PhysicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, gDefaultErrorCallback);
	g_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_PhysicsFoundation, PxTolerancesScale());
	g_PhysicsCooker = PxCreateCooking(PX_PHYSICS_VERSION, *g_PhysicsFoundation, PxCookingParams(PxTolerancesScale()));
	PxInitExtensions(*g_Physics);
	//create physics material
	g_PhysicsMaterial = g_Physics->createMaterial(0.5f, 0.5f, 0.6f);
	PxSceneDesc sceneDesc(g_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -1.0f, 0);
	sceneDesc.filterShader = gDefaultFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	g_PhysicsScene = g_Physics->createScene(sceneDesc);



	if (g_PhysicsScene)
	{
		std::cout << "start physx scene2";
	}
}

void PhysXTutorials::cleanUpPhsyx()
{
	g_PhysicsCooker->release();
	g_PhysicsScene->release();
	g_Physics->release();
	g_PhysicsFoundation->release();
}

void PhysXTutorials::upDatePhysx()
{
	g_PhysicsScene->simulate(1 / 60.f);
	while (g_PhysicsScene->fetchResults() == false)
	{
		// don’t need to do anything here yet but we still need to do the fetch
	}

	for (auto actor : g_PhysXActors)
	{
		PxU32 nShapes = actor->getNbShapes();
		PxShape** shapes = new PxShape*[nShapes];
		actor->getShapes(shapes, nShapes);
		// Render all the shapes in the physx actor (for early tutorials there is just one)
		while (nShapes--)
		{
			addWidget(shapes[nShapes], actor);
		}
		delete[] shapes;
	}
}

void PhysXTutorials::controlPlayer(float a_deltaTime)
{
	PxTransform pose = playerActor->getGlobalPose(); //get the pose from PhysX
	pose.q = PxQuat(11 / 7.0f, PxVec3(0, 0, 1));  //force the actor rotation to vertical
	playerActor->setGlobalPose(pose); //reset the actor pose
	//set linear damping on our actor so it slows down when we stop pressing the ke

	if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		playerActor->addForce(PxVec3(100, 0, 0));
	}

	if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		playerActor->addForce(PxVec3(-100, 0, 0));
	}

	if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		playerActor->addforce(PxVec3(0, 100, 0));
	}

	if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		playerActor->addforce(PxVec3(0, -100, 0));
	}

	//rest of code to move player about..
}


void PhysXTutorials::addBox(PxShape* pShape, PxRigidDynamic* actor)
{
	//get the geometry for this PhysX collision volume
	PxBoxGeometry geometry;
	float width = 1, height = 1, length = 1;
	bool status = pShape->getBoxGeometry(geometry);
	if (status)
	{
		width = geometry.halfExtents.x;
		height = geometry.halfExtents.y;
		length = geometry.halfExtents.z;
	}
	//get the transform for this PhysX collision volume
	PxMat44 m(PxShapeExt::getGlobalPose(*pShape));
	glm::mat4 M(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
		m.column1.x, m.column1.y, m.column1.z, m.column1.w,
		m.column2.x, m.column2.y, m.column2.z, m.column2.w,
		m.column3.x, m.column3.y, m.column3.z, m.column3.w);
	glm::vec3 position;
	//get the position out of the transform
	position.x = m.getPosition().x;
	position.y = m.getPosition().y;
	position.z = m.getPosition().z;
	glm::vec3 extents = glm::vec3(width, height, length);
	glm::vec4 colour = glm::vec4(1, 0, 0, 1);
	//create our box gizmo
	Gizmos::addAABBFilled(position, extents, colour, &M);
}

void PhysXTutorials::addSphere(PxShape* pShape, PxRigidActor* actor)
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
	PxMat44 m(PxShapeExt::getGlobalPose(*pShape)); 
	glm::mat4 M(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
		m.column1.x, m.column1.y, m.column1.z, m.column1.w,
		m.column2.x, m.column2.y, m.column2.z, m.column2.w,
		m.column3.x, m.column3.y, m.column3.z, m.column3.w);
	glm::vec3 position; 

	position.x = m.getPosition().x;
	position.y = m.getPosition().y;
	position.z = m.getPosition().z;
	//create a widget to represent it
	Gizmos::addSphere(position, 10, 10, radius, glm::vec4(1, 0, 1, 1));
}

//helper function to convert PhysX matrix to Opengl
glm::mat4 Px2Glm(PxMat44 m)
{
	glm::mat4 M(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
		m.column1.x, m.column1.y, m.column1.z, m.column1.w,
		m.column2.x, m.column2.y, m.column2.z, m.column2.w,
		m.column3.x, m.column3.y, m.column3.z, m.column3.w);
	return M; 
}

glm::vec3 Px2GlV3(PxVec3 v1)
{
	glm::vec3 v2; 
	v2.x = v1.x; 
	v2.y = v1.y; 
	v2.z = v1.z; 
	return v2; 
}

void PhysXTutorials::addWidget(PxShape* shape, PxRigidDynamic* actor)
{
	PxGeometryType::Enum type = shape->getGeometryType();
	switch (type)
	{
	case PxGeometryType::eBOX:
		addBox(shape, actor);
		break;
	case PxGeometryType::eSPHERE:
		addSphere(shape,actor);
		break;
	case PxGeometryType::eCAPSULE:
		addCapsule(shape, actor); 
		break; 
	}
}

void PhysXTutorials::addCapsule(PxShape* pShape, PxRigidActor* actor)
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
	PxTransform transform = PxShapeExt::getGlobalPose(*pShape);
	//use it to create a matrix
	PxMat44 m(transform);
	//convert it to an open gl matrix for adding our gizmos
	glm::mat4 M = Px2Glm(m);
	//get the world position from the PhysX tranform
	glm::vec3 position = Px2GlV3(transform.p);
	glm::vec4 axis(halfHeight, 0, 0, 0);	//axis for the capsule
	axis = M * axis; //rotate axis to correct orientation
	//add our 2 end cap spheres...
	Gizmos::addSphere(position + axis.xyz, 10, 10, radius, colour);
	Gizmos::addSphere(position - axis.xyz, 10, 10, radius, colour);
	//the cylinder gizmo is oriented 90 degrees to what we want so we need to change the rotation matrix...
	glm::mat4 m2 = glm::rotate(M, 11 / 7.0f, glm::vec3(0.0f, 0.0f, 1.0f)); //adds an additional rotation onto the matrix
	//now we can use this matrix and the other data to create the cylinder...
	Gizmos::addCylinderFilled(position, radius, halfHeight, 10, colour, &m2);
}

void PhysXTutorials::addPlatforms()
{
	for (auto staticObjectPtr : gStaticObject)
	{
		PxBoxGeometry box(staticObjectPtr->extents); 
		PxTransform pose = PxTransform(staticObjectPtr->centre); 
		PxRigidStatic*staticObject = PxCreateStatic(*g_Physics, pose, box, *g_PhysicsMaterial);
		//add it to the PhysX scene
		g_PhysicsScene->addActor(*staticObject); 

		//g_PhysXActors.push_back(staticObject); 
		staticObject->setName(staticObjectPtr->name); 
	}
}

void PhysXTutorials::tutorial_1()
{
	//add a plane
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi*0.95f, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*g_Physics, pose, PxPlaneGeometry(), *g_PhysicsMaterial);
	//add it to the physX scene
	g_PhysicsScene->addActor(*plane);

	//add a box
	float density = 10;
	PxBoxGeometry box(2, 2, 2);
	PxTransform transform(PxVec3(0, 20, 0));
	PxRigidDynamic* dynamicActor = PxCreateDynamic(*g_Physics, transform, box, *g_PhysicsMaterial, density);
	//add it to the physX scene
	g_PhysicsScene->addActor(*dynamicActor);
	//add it to our copy of the scene
	g_PhysXActors.push_back(dynamicActor);
}

void PhysXTutorials::tutorial_1_ballGun()
{
	//cout << "ball gun"; 
	float muzzleSpeed = -100; 
	GLFWwindow* window = glfwGetCurrentContext(); 
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !GLFWMouseButton1Down)
	{
		GLFWMouseButton1Down = true; 
		PxSphereGeometry sphere(1); 
		glm::vec3 position(m_cameraMatrix[3]); 
		glm::vec3 direction(m_cameraMatrix[2]); 
		physx::PxVec3 velocity = physx::PxVec3(direction.x, direction.y, direction.z); 
		float density = 500; 
		PxTransform transform(PxVec3(position.x, position.y, position.z), PxQuat::createIdentity()); 
		PxRigidDynamic *dynamicActor = PxCreateDynamic(*g_Physics, transform, sphere, *g_PhysicsMaterial, density);
		dynamicActor->setLinearVelocity(velocity, true); 
		g_PhysicsScene->addActor(*dynamicActor);
		g_PhysXActors.push_back(dynamicActor); 
		//bullets.push_back(new BulletStruct(dynamicActor)); 

	}
}