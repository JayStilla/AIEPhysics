#pragma once

#include "Application.h"
#include <glm/glm.hpp>

#include <PxPhysicsAPI.h>
#include <PxQueryFiltering.h>
#include <FBXFile.h>

// #PVD_AVAILABLE // uncomment this if we ever get PVD binaries in /MDd

using namespace physx;

struct RagdollNode
{
	PxQuat globalRotation;  //rotation of this link in model space - we could have done this relative to the parent node but it's harder to visualize when setting up the data by hand
	PxVec3 scaledGobalPos; //Position of the link centre in world space which is calculated when we process the node.  It's easiest if we store it here so we have it when we transform the child
	int parentNodeIdx;	//Index of the parent node
	float halfLength; //half length of the capsule for this node
	float radius; //radius of capsule for thisndoe
	float parentLinkPos; //relative position of link centre in parent to this node.  0 is the centre of hte node, -1 is left end of capsule and 1 is right end of capsule relative to x 
	float childLinkPos;  //relative position of link centre in child
	char* name;	//name of link
	PxArticulationLink* linkPtr; //pointer to link if we are using articulation
	PxRigidDynamic* actorPtr;  //Pointer the PhysX actor which is linked to this node if we are using seperate actors

	//constructor
	RagdollNode(PxQuat _globalRotation, int _parentNodeIdx, float _halfLength, float _radius, float _parentLinkPos, float _childLinkPos, char* _name){ globalRotation = _globalRotation, parentNodeIdx = _parentNodeIdx; halfLength = _halfLength; radius = _radius; parentLinkPos = _parentLinkPos; childLinkPos = _childLinkPos; name = _name; };
};

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
	void addWidget(PxShape * pShape, PxRigidActor * actor);

	void addBox(PxShape* pShape, PxRigidActor* actor);
	void addSphere(PxShape * pShape, PxRigidActor * actor);
	void addCapsule(PxShape* pShape, PxRigidActor * actor);

	void useBallGun();

	// # Ragdoll
	void addSteps(PxTransform transform, int numberSteps);
	PxArticulation* makeRagdoll(RagdollNode** nodes, PxTransform worldPos, float scale);
	void makeRagdollUsingActors(RagdollNode** nodes, PxTransform worldPos, float scale);	// structure assigned to an FBXMeshNode's m_userData

	struct GLData
	{
		unsigned int vbo, ibo, vao;
	};
	FBXFile*	m_file;
	unsigned int m_program, m_vs, m_fs;

#ifdef PVD_AVAILABLE
	void setUpVisualDebugger();
	physx::PxVisualDebuggerConnection * m_PVDConnection;
#endif
};