#pragma once
#include "Actor.h"
#include "Utilities.h"
#include <set>

class Scene
{
public:

	Scene(const glm::vec3& a_gravity = glm::vec3(0.0f, -9.81f, 0.0f),
		  float a_timeStep = 0.01f)
		: m_gravity(a_gravity), m_timeStep(a_timeStep),
		  m_lastUpdate(Utility::getTotalTime()) {}
	~Scene() { ClearActors(); }

	void AddActor(Actor* a_actor);
	void ClearActors();
	bool DestroyActor(Actor* a_actor);	// returns false if actor not in scene
	const std::set<Actor*>& GetActors() const { return m_actors; }
	bool HasActor(Actor* a_actor) const { return nullptr != a_actor && 0 != m_actors.count(a_actor); }

	void Update();
	void Render() const;


protected:

	glm::vec3 m_gravity;
	float m_timeStep;
	float m_lastUpdate;

	std::set<Actor*> m_actors;

};
