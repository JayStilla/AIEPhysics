#pragma once

#include "Application.h"
#include <glm/glm.hpp>

#include <PxPhysicsAPI.h>
#include <cloth\PxCloth.h>
#include <cloth\PxClothCollisionData.h>
#include <cloth\PxClothFabric.h>
#include <cloth\PxClothTypes.h>

#include "TextureData.h"

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

	// Cloth
	unsigned int m_shader;
	TextureData m_texture;

	unsigned int m_clothIndexCount;
	unsigned int m_clothVertexCount;
	glm::vec3 * m_clothPositions;		// populated each frame

	unsigned int m_clothVAO, m_clothVBO, m_clothTextureVBO, m_clothIBO;

	PxCloth *		m_cloth;

	PxCloth*		createCloth(const glm::vec3& a_position,
		unsigned int& a_vertexCount, unsigned int& a_indexCount,
		const glm::vec3* a_vertices,
		unsigned int* a_indices);

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