#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include <vector>
#include <PxPhysicsAPI.h>
#include <iostream>

using namespace physx; 

// derived application class that wraps up all globals neatly
class PhysXTutorials : public Application
{
public:

	PhysXTutorials();
	virtual ~PhysXTutorials();

	void setUpPhysXTutorial();
	void cleanUpPhsyx();
	void upDatePhysx();
	void addBox(PxShape* pShape, PxRigidDynamic* actor);
	void addSphere(PxShape* pShape, PxRigidActor* actor);
	void addCapsule(PxShape* pShape, PxRigidActor* actor); 
	void addPlatforms(); 
	void addWidget(PxShape* shape, PxRigidDynamic* actor);
	void controlPlayer(float a_deltaTime);


	void tutorial_1();
	void tutorial_1_ballGun(); 

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	bool GLFWMouseButton1Down; 
	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;
};

PxFilterFlags myFliterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}
	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	// trigger the contact callback for pairs (A,B) where
	// the filtermask of A contains the ID of B and vice versa.
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

	return PxFilterFlag::eDEFAULT;
}

struct FilterGroup
{
	enum Enum
	{
		ePLAYER = (1 << 0),
		ePLATFORM = (1 << 1),
		eGROUND = (1 << 2)
	};
};

//helper function to set up filtering
void setupFiltering(PxRigidActor* actor, PxU32 filterGroup, PxU32 filterMask)
{
	PxFilterData filterData;
	filterData.word0 = filterGroup; // word0 = own ID
	filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a contact callback;
	const PxU32 numShapes = actor->getNbShapes();
	PxShape** shapes = (PxShape**)_aligned_malloc(sizeof(PxShape*)*numShapes, 16);
	actor->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; i++)
	{
		PxShape* shape = shapes[i];
		shape->setSimulationFilterData(filterData);
	}
	_aligned_free(shapes);
}

struct StaticObject
{
	PxVec3 centre;
	PxVec3 extents;
	bool trigger;
	char* name;
public:
	StaticObject(float x, float y, float z, float length, float height, float width, bool trigger, char* name)
	{
		centre.x = x;
		centre.y = y;
		centre.z = z;
		extents.x = width;
		extents.y = height;
		extents.z = length;
		this->trigger = trigger;
		this->name = name;
	}
};

StaticObject* gStaticObject[] = {
	new StaticObject(0, 1, 0, 2, 1.0f, 2, 0, "Platform"),
	new StaticObject(7, 1, 0, 2, 1.0f, 2, 0, "Platform"),
	new StaticObject(14, 2, 0, 2, 2.0f, 2, 0, "Platform"),
	new StaticObject(22, 4, 8, 10, 4.0f, 2, 0, "Platform"),
	new StaticObject(22, 10, 17, 0.5f, 2, 0.5f, 1, "Pickup1"),
};

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

class MycollisionCallBack : public PxSimulationEventCallback
{
	virtual void MycollisionCallBack::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
	{
		for (PxU32 i = 0; i < nbPairs; i++)
		{
			const PxContactPair& cp = pairs[i];
			if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				std::cout << "collision: " << pairHeader.actors[0]->getName() << " : " << pairHeader.actors[1]->getName() << std::endl;
			}
		}
	}
	//we have to create versions of the following functions even though we don't do anything with them...
	virtual void    onTrigger(PxTriggerPair* pairs, PxU32 nbPairs){};
	virtual void	onConstraintBreak(PxConstraintInfo*, PxU32){};
	virtual void	onWake(PxActor**, PxU32){};
	virtual void	onSleep(PxActor**, PxU32){};
};