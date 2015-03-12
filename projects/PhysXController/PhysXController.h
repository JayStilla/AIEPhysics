#pragma once

#include "Application.h"
#include <glm/glm.hpp>

#include <PxPhysicsAPI.h>

// #PVD_AVAILABLE // uncomment this if we ever get PVD binaries in /MDd or /MD

using namespace physx;

struct HeightMap
{
	PxHeightFieldSample* samples;
	int numRows;
	int numCols;
	float heightScale;
	float rowScale;
	float colScale;
	glm::vec3 center;
	bool enabled;

public:
	HeightMap()
	{
		enabled = false;
	}
	PxRigidStatic* actor;
};

// derived application class that wraps up all globals neatly
class PhysXTutorial : public Application
{
public:

	PhysXTutorial();
	virtual ~PhysXTutorial();

protected:
	friend class MyControllerHitReport;

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	// # PhysX
	void setUpPhysXTutorial();
	void updatePhysX(float a_deltaTime);
	void cleanUpPhysX();
	void addWidget(PxShape * pShape, PxRigidActor * actor);

	void addBox(PxShape* pShape, PxRigidActor* actor);
	void addSphere(PxShape * pShape, PxRigidActor * actor);
	void addCapsule(PxShape* pShape, PxRigidActor * actor);

	// # Character Controller Tutorial
	void controlPlayer(float a_deltaTime);

	void addTestEnvironment();
	void addHeightMap();
	void createCollisionGroundPlane();

	PxRigidStatic* addStaticHeightMapCollision(PxTransform transform);

	float gravity;
	PxVec3 playerContactNormal; //set to a value if the player has contacted something;
	PxRigidDynamic* playerActor;

	float characterRotation;
	float characterYVelocity;

	HeightMap heightMap;

#ifdef PVD_AVAILABLE
	void setUpVisualDebugger();
	physx::PxVisualDebuggerConnection * m_PVDConnection;
#endif
};