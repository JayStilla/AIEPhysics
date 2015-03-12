#pragma once

#include "Application.h"
#include <glm/glm.hpp>

#include <PxPhysicsAPI.h>

using namespace physx;

// #PVD_AVAILABLE // uncomment this if we ever get PVD binaries in /MDd

// derived application class that wraps up all globals neatly
class PhysXTutorial : public Application
{
public:

	PhysXTutorial();
	virtual ~PhysXTutorial();

protected:

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
	void PhysXTutorial::addWidget(PxShape * pShape, PxRigidActor * actor);

	void addBox(PxShape* pShape, PxRigidActor* actor);
	void addSphere(PxShape * pShape, PxRigidActor * actor);
	void addCapsule(PxShape* pShape, PxRigidActor * actor);

	void useBallGun();

#ifdef PVD_AVAILABLE
	void setUpVisualDebugger();
	physx::PxVisualDebuggerConnection * m_PVDConnection;
#endif
};