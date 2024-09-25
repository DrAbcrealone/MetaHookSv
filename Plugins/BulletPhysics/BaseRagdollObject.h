#pragma once

#include "exportfuncs.h"
#include "ClientEntityManager.h"
#include "BasePhysicManager.h"

class CBaseRagdollObject : public IRagdollObject
{
public:
	CBaseRagdollObject(const CPhysicObjectCreationParameter& CreationParam);
	~CBaseRagdollObject();

	int GetEntityIndex() const override;
	cl_entity_t* GetClientEntity() const override;
	entity_state_t* GetClientEntityState() const override;
	model_t* GetModel() const override;
	float GetModelScaling() const override;
	uint64 GetPhysicObjectId() const override;
	int GetPlayerIndex() const override;
	int GetObjectFlags() const override;
	int GetPhysicConfigId() const override;
	bool IsClientEntityNonSolid() const override;
	bool ShouldDrawOnDebugDraw(const CPhysicDebugDrawContext* ctx) const override;
	bool EnumPhysicComponents(const fnEnumPhysicComponentCallback& callback) override;
	bool Build(const CPhysicObjectCreationParameter& CreationParam) override;
	bool Rebuild(const CPhysicObjectCreationParameter& CreationParam) override;
	void Update(CPhysicObjectUpdateContext* ObjectUpdateContext) override;
	bool GetGoldSrcOriginAngles(float* origin, float* angles) override;
	bool CalcRefDef(struct ref_params_s* pparams, bool bIsThirdPerson, void(*callback)(struct ref_params_s* pparams)) override;
	bool SyncThirdPersonView(struct ref_params_s* pparams, void(*callback)(struct ref_params_s* pparams)) override;
	bool SyncFirstPersonView(struct ref_params_s* pparams, void(*callback)(struct ref_params_s* pparams)) override;
	void UpdatePose(entity_state_t* curstate) override;
	bool SetupBones(studiohdr_t* studiohdr) override;
	bool SetupJiggleBones(studiohdr_t* studiohdr) override;
	bool ResetPose(entity_state_t* curstate) override;
	void ApplyBarnacle(IPhysicObject* pBarnacleObject) override;
	void ReleaseFromBarnacle() override;
	void ApplyGargantua(IPhysicObject* pGargantuaObject) override;
	void ReleaseFromGargantua() override;
	StudioAnimActivityType GetActivityType() const override;
	void CalculateOverrideActivityType(const entity_state_t* entstate, StudioAnimActivityType& ActivityType) const override;
	int GetBarnacleIndex() const override;
	int GetGargantuaIndex() const override;
	bool IsDebugAnimEnabled() const override;
	void SetDebugAnimEnabled(bool bEnabled) override;
	void AddPhysicComponentsToPhysicWorld(void* world, const CPhysicComponentFilters& filters) override;
	void RemovePhysicComponentsFromPhysicWorld(void* world, const CPhysicComponentFilters& filters) override;
	void FreePhysicComponentsWithFilters(const CPhysicComponentFilters& filters) override;
	void TransferOwnership(int entindex) override;
	IPhysicComponent* GetPhysicComponentByName(const std::string& name) override;
	IPhysicComponent* GetPhysicComponentByComponentId(int id) override;
	IPhysicRigidBody* GetRigidBodyByName(const std::string& name) override;
	IPhysicRigidBody* GetRigidBodyByComponentId(int id) override;
	IPhysicConstraint* GetConstraintByName(const std::string& name) override;
	IPhysicConstraint* GetConstraintByComponentId(int id) override;
	IPhysicAction* GetPhysicActionByName(const std::string& name) override;
	IPhysicAction* GetPhysicActionByComponentId(int id) override;

	virtual IPhysicRigidBody* FindRigidBodyByName(const std::string& name, bool allowNonNativeRigidBody);
	virtual void SetupNonKeyBones(const CPhysicObjectCreationParameter& CreationParam);
	virtual void InitCameraControl(const CClientCameraControlConfig* pCameraControlConfig, CPhysicCameraControl& CameraControl);
	virtual void SaveBoneRelativeTransform(const CPhysicObjectCreationParameter& CreationParam);
	
	virtual IPhysicRigidBody* CreateRigidBody(const CPhysicObjectCreationParameter& CreationParam, CClientRigidBodyConfig* pRigidConfig, int physicComponentId) = 0;
	virtual IPhysicConstraint* CreateConstraint(const CPhysicObjectCreationParameter& CreationParam, CClientConstraintConfig* pConstraintConfig, int physicComponentId) = 0;
	virtual IPhysicAction* CreateAction(const CPhysicObjectCreationParameter& CreationParam, CClientPhysicActionConfig* pActionConfig, int physicComponentId) = 0;

	virtual void AddRigidBody(const CPhysicObjectCreationParameter& CreationParam, CClientRigidBodyConfig* pRigidBodyConfig, IPhysicRigidBody* pRigidBody);
	virtual void AddConstraint(const CPhysicObjectCreationParameter& CreationParam, CClientConstraintConfig* pConstraintConfig, IPhysicConstraint* pConstraint);
	virtual void AddAction(const CPhysicObjectCreationParameter& CreationParam, CClientPhysicActionConfig* pPhysicActionConfig, IPhysicAction* pPhysicAction);

protected:

	bool UpdateActivity(StudioAnimActivityType iOldActivityType, StudioAnimActivityType iNewActivityType, entity_state_t* curstate);

public:
	int m_entindex{};
	int m_playerindex{};
	cl_entity_t* m_entity{};
	model_t* m_model{};
	float m_model_scaling{ 1 };
	int m_flags{ PhysicObjectFlag_RagdollObject };
	int m_debugDrawLevel{ BULLET_DEFAULT_DEBUG_DRAW_LEVEL };
	int m_configId{};

	std::vector<std::shared_ptr<CClientRigidBodyConfig>> m_RigidBodyConfigs;
	std::vector<std::shared_ptr<CClientConstraintConfig>> m_ConstraintConfigs;
	std::vector<std::shared_ptr<CClientPhysicActionConfig>> m_ActionConfigs;
	std::vector<std::shared_ptr<CClientAnimControlConfig>> m_AnimControlConfigs;

	std::shared_ptr<CClientAnimControlConfig> m_IdleAnimConfig;
	std::shared_ptr<CClientAnimControlConfig> m_DebugAnimConfig;
	bool m_bDebugAnimEnabled{};

	CPhysicCameraControl m_FirstPersonViewCameraControl;
	CPhysicCameraControl m_ThirdPersonViewCameraControl;

	StudioAnimActivityType m_iActivityType{ StudioAnimActivityType_Idle };
	int m_iBarnacleIndex{ 0 };
	int m_iGargantuaIndex{ 0 };

	std::vector<int> m_keyBones;
	std::vector<int> m_nonKeyBones;

	std::vector<IPhysicComponent*> m_PhysicComponents;
};