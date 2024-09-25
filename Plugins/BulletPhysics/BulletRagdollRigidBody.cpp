#include "BulletRagdollRigidBody.h"
#include "privatehook.h"
#include "plugins.h"

CBulletRagdollRigidBody::CBulletRagdollRigidBody(
	int id,
	int entindex,
	IPhysicObject* pPhysicObject,
	const CClientRigidBodyConfig* pRigidConfig,
	const btRigidBody::btRigidBodyConstructionInfo& constructionInfo,
	int group,
	int mask) :

	CBulletPhysicRigidBody(
		id,
		entindex,
		pPhysicObject,
		pRigidConfig,
		constructionInfo,
		group,
		mask)
{

}

bool CBulletRagdollRigidBody::ResetPose(studiohdr_t* studiohdr, entity_state_t* curstate)
{
	if (!m_pInternalRigidBody)
		return false;

	auto pMotionState = (CBulletBaseMotionState*)m_pInternalRigidBody->getMotionState();

	if (pMotionState->IsBoneBased())
	{
		if (!(m_boneindex >= 0 && m_boneindex < studiohdr->numbones))
		{
			Sys_Error("CBulletRagdollRigidBody::ResetPose invalid m_boneindex!");
			return false;
		}

		auto pBoneMotionState = (CBulletBoneMotionState*)pMotionState;

		btTransform bonematrix;

		Matrix3x4ToTransform((*pbonetransform)[m_boneindex], bonematrix);

		TransformGoldSrcToBullet(bonematrix);

		auto newWorldTrans = bonematrix * pBoneMotionState->m_offsetmatrix;

		m_pInternalRigidBody->setWorldTransform(newWorldTrans);
		m_pInternalRigidBody->setInterpolationWorldTransform(newWorldTrans);
		m_pInternalRigidBody->getMotionState()->setWorldTransform(newWorldTrans);
	}

	return true;
}

bool CBulletRagdollRigidBody::SetupBones(studiohdr_t* studiohdr)
{
	if (!m_pInternalRigidBody)
		return false;

	auto pMotionState = (CBulletBaseMotionState*)m_pInternalRigidBody->getMotionState();

	if (pMotionState->IsBoneBased())
	{
		if (!(m_boneindex >= 0 && m_boneindex < studiohdr->numbones))
		{
			Sys_Error("CBulletRagdollRigidBody::SetupBones invalid m_boneindex!");
			return false;
		}

		auto pBoneMotionState = (CBulletBoneMotionState*)pMotionState;

		btTransform bonematrix = pBoneMotionState->m_bonematrix;

		TransformBulletToGoldSrc(bonematrix);

		float bonematrix_3x4[3][4];
		TransformToMatrix3x4(bonematrix, bonematrix_3x4);

		memcpy((*pbonetransform)[m_boneindex], bonematrix_3x4, sizeof(bonematrix_3x4));
		memcpy((*plighttransform)[m_boneindex], bonematrix_3x4, sizeof(bonematrix_3x4));

		return true;
	}

	return false;
}

bool CBulletRagdollRigidBody::SetupJiggleBones(studiohdr_t* studiohdr)
{
	if (!m_pInternalRigidBody)
		return false;

	auto pMotionState = (CBulletBaseMotionState*)m_pInternalRigidBody->getMotionState();

	if (pMotionState->IsBoneBased())
	{
		if (!(m_boneindex >= 0 && m_boneindex < studiohdr->numbones))
		{
			Sys_Error("CBulletRagdollRigidBody::SetupJiggleBones invalid m_boneindex!");
			return false;
		}

		auto pBoneMotionState = (CBulletBoneMotionState*)pMotionState;

		if (!m_pInternalRigidBody->isKinematicObject())
		{
			btTransform bonematrix = pBoneMotionState->m_bonematrix;

			TransformBulletToGoldSrc(bonematrix);

			float bonematrix_3x4[3][4];
			TransformToMatrix3x4(bonematrix, bonematrix_3x4);

			memcpy((*pbonetransform)[m_boneindex], bonematrix_3x4, sizeof(bonematrix_3x4));
			memcpy((*plighttransform)[m_boneindex], bonematrix_3x4, sizeof(bonematrix_3x4));
		}
		else
		{
			auto& bonematrix = pBoneMotionState->m_bonematrix;

			Matrix3x4ToTransform((*pbonetransform)[m_boneindex], bonematrix);

			TransformGoldSrcToBullet(bonematrix);
		}

		return true;
	}

	return false;
}

void CBulletRagdollRigidBody::Update(CPhysicComponentUpdateContext* ComponentUpdateContext)
{
	if (!m_pInternalRigidBody)
		return;

	auto pPhysicObject = GetOwnerPhysicObject();

	if (!pPhysicObject->IsRagdollObject())
		return;

	auto ent = pPhysicObject->GetClientEntity();

	auto pRagdollObject = (IRagdollObject*)pPhysicObject;

	bool bKinematic = false;

	bool bKinematicStateChanged = false;

	do
	{
		if (ComponentUpdateContext->m_bForceKinematic)
		{
			bKinematic = true;
			break;
		}

		if (ComponentUpdateContext->m_bForceDynamic)
		{
			bKinematic = false;
			break;
		}

		if (m_flags & PhysicRigidBodyFlag_AlwaysKinematic)
		{
			bKinematic = true;
			break;
		}

		if (m_flags & PhysicRigidBodyFlag_AlwaysDynamic)
		{
			bKinematic = false;
			break;
		}

		if (pRagdollObject->GetActivityType() > StudioAnimActivityType_Idle)
		{
			bKinematic = false;
			break;
		}
		else
		{
			bKinematic = true;
			break;
		}

	} while (0);

	if (bKinematic)
	{
		int iCollisionFlags = m_pInternalRigidBody->getCollisionFlags();

		if (!(iCollisionFlags & btCollisionObject::CF_KINEMATIC_OBJECT))
		{
			iCollisionFlags |= btCollisionObject::CF_KINEMATIC_OBJECT;

			m_pInternalRigidBody->setCollisionFlags(iCollisionFlags);
			m_pInternalRigidBody->setActivationState(DISABLE_DEACTIVATION);
			m_pInternalRigidBody->setGravity(btVector3(0, 0, 0));

			bKinematicStateChanged = true;
		}
	}
	else
	{
		int iCollisionFlags = m_pInternalRigidBody->getCollisionFlags();

		if (iCollisionFlags & btCollisionObject::CF_KINEMATIC_OBJECT)
		{
			iCollisionFlags &= ~btCollisionObject::CF_KINEMATIC_OBJECT;

			m_pInternalRigidBody->setCollisionFlags(iCollisionFlags);
			m_pInternalRigidBody->forceActivationState(ACTIVE_TAG);
			m_pInternalRigidBody->setMassProps(m_mass, m_inertia);

			bKinematicStateChanged = true;
		}
	}

	if (bKinematicStateChanged)
	{
		ComponentUpdateContext->m_pObjectUpdateContext->m_bRigidbodyKinematicChanged = true;
	}
}