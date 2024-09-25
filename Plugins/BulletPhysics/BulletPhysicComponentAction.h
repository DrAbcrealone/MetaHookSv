#pragma once

#include "BasePhysicComponentAction.h"
#include "BulletPhysicManager.h"

class CBulletPhysicComponentAction : public CBasePhysicComponentAction
{
public:
	CBulletPhysicComponentAction(int id, int entindex, IPhysicObject* pPhysicObject, const CClientPhysicActionConfig* pActionConfig, int attachedPhysicComponentId);
	~CBulletPhysicComponentAction();

	bool AddToPhysicWorld(void* world) override;
	bool RemoveFromPhysicWorld(void* world) override;
	bool IsAddedToPhysicWorld(void* world) const override;

protected:
	btRigidBody* CreateInternalRigidBody(IPhysicObject* pPhysicObject, const CClientPhysicActionConfig* pActionConfig, int attachedPhysicComponentId);
	void FreeInternalRigidBody(btRigidBody* pRigidBody);

public:
	bool m_addedToPhysicWorld{};

	btTransform m_offsetmatrix{};

	//For rayTest only
	btRigidBody* m_pInternalRigidBody{};
};


