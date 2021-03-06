#pragma once

#include "Application.h"
#include <glm/glm.hpp>

#include <PxPhysicsAPI.h>

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


	// PhysX

	void setUpPhysXTutorial();

	void updatePhysX();

	void cleanUpPhysX();	// @AIE - there is a typo in the tutorial (Phsyx vs PhysX)

	void addWidget(physx::PxShape * pShape, physx::PxRigidDynamic * actor);
	void addBox(physx::PxShape * pShape, physx::PxRigidDynamic * actor);

#ifdef PVD_AVAILABLE
	void setUpVisualDebugger();
	physx::PxVisualDebuggerConnection * m_PVDConnection;
#endif
};