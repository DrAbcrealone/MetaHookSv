#pragma once

#include <vector>
#include <memory>

#define BULLET_DEFAULT_DEBUG_DRAW_LEVEL 1
#define BULLET_WORLD_DEBUG_DRAW_LEVEL 10
#define BULLET_DEFAULT_SOFTNESS 1.0f
#define BULLET_DEFAULT_BIAS_FACTOR 0.3f
#define BULLET_DEFAULT_RELAXTION_FACTOR 1.0f
#define BULLET_DEFAULT_LINEAR_ERP 0.3f
#define BULLET_DEFAULT_ANGULAR_ERP 0.3f
#define BULLET_DEFAULT_LINEAR_CFM 0.01f
#define BULLET_DEFAULT_ANGULAR_CFM 0.01f
#define BULLET_DEFAULT_LINEAR_STOP_ERP 0.3f
#define BULLET_DEFAULT_ANGULAR_STOP_ERP 0.3f
#define BULLET_DEFAULT_LINEAR_STOP_CFM 0.01f
#define BULLET_DEFAULT_ANGULAR_STOP_CFM 0.01f
#define BULLET_DEFAULT_CCD_THRESHOLD 0.001f
#define BULLET_DEFAULT_LINEAR_FRICTION 1.0f
#define BULLET_DEFAULT_ANGULAR_FRICTION 0.2f
#define BULLET_DEFAULT_RESTITUTION 0.0f
#define BULLET_DEFAULT_MASS 1.0f
#define BULLET_DEFAULT_DENSENTY 1.0f
#define BULLET_DEFAULT_LINEAR_SLEEPING_THRESHOLD 5.0f
#define BULLET_DEFAULT_ANGULAR_SLEEPING_THRESHOLD 3.0f
#define BULLET_DEFAULT_MAX_TOLERANT_LINEAR_ERROR 30.0f

#define INCHES_PER_METER 1//39.3700787402f

const float B2GScale = INCHES_PER_METER;
const float G2BScale = 1.0f / B2GScale;

enum PhysicConfigType
{
	PhysicConfigType_None,
	PhysicConfigType_PhysicObject,
	PhysicConfigType_RigidBody,
	PhysicConfigType_Constraint,
	PhysicConfigType_CollisionShape,
	PhysicConfigType_Action,
	PhysicConfigType_AnimControl,
};

enum PhysicConfigState
{
	PhysicConfigState_NotLoaded = 0,
	PhysicConfigState_Loaded,
	PhysicConfigState_LoadedWithError
};

enum PhysicObjectType
{
	PhysicObjectType_None,
	PhysicObjectType_StaticObject,
	PhysicObjectType_DynamicObject,
	PhysicObjectType_RagdollObject,
	PhysicObjectType_Maximum
};

//Flags are always runtime

const int PhysicObjectFlag_Barnacle = 0x1;
const int PhysicObjectFlag_Gargantua = 0x2;
const int PhysicObjectFlag_StaticObject = 0x1000;
const int PhysicObjectFlag_DynamicObject = 0x2000;
const int PhysicObjectFlag_RagdollObject = 0x4000;
const int PhysicObjectFlag_FromBSP = 0x8000;
const int PhysicObjectFlag_FromConfig = 0x10000;
const int PhysicObjectFlag_AnyObject = (PhysicObjectFlag_StaticObject | PhysicObjectFlag_RagdollObject | PhysicObjectFlag_DynamicObject);

const int PhysicRigidBodyFlag_None = 0;
const int PhysicRigidBodyFlag_AlwaysDynamic = 0x1;
const int PhysicRigidBodyFlag_AlwaysKinematic = 0x2;
const int PhysicRigidBodyFlag_AlwaysStatic = 0x4;
const int PhysicRigidBodyFlag_NoCollisionToWorld = 0x8;
const int PhysicRigidBodyFlag_NoCollisionToStaticObject = 0x10;
const int PhysicRigidBodyFlag_NoCollisionToDynamicObject = 0x20;
const int PhysicRigidBodyFlag_NoCollisionToRagdollObject = 0x40;

const int PhysicRigidBodyFlag_AllowedOnStaticObject = (
	PhysicRigidBodyFlag_AlwaysStatic | PhysicRigidBodyFlag_AlwaysKinematic | 
	PhysicRigidBodyFlag_NoCollisionToWorld | PhysicRigidBodyFlag_NoCollisionToStaticObject | PhysicRigidBodyFlag_NoCollisionToDynamicObject | PhysicRigidBodyFlag_NoCollisionToRagdollObject);

const int PhysicRigidBodyFlag_AllowedOnDynamicObject = (
	PhysicRigidBodyFlag_AlwaysDynamic | PhysicRigidBodyFlag_AlwaysKinematic |
	PhysicRigidBodyFlag_NoCollisionToWorld | PhysicRigidBodyFlag_NoCollisionToStaticObject | PhysicRigidBodyFlag_NoCollisionToDynamicObject | PhysicRigidBodyFlag_NoCollisionToRagdollObject);

const int PhysicRigidBodyFlag_AllowedOnRagdollObject = (
	PhysicRigidBodyFlag_AlwaysDynamic | PhysicRigidBodyFlag_AlwaysKinematic |
	PhysicRigidBodyFlag_NoCollisionToWorld | PhysicRigidBodyFlag_NoCollisionToStaticObject | PhysicRigidBodyFlag_NoCollisionToDynamicObject | PhysicRigidBodyFlag_NoCollisionToRagdollObject);

const int PhysicRigidBodyFactorIdx_Maximum = 32;

enum PhysicConstraint
{
	PhysicConstraint_None = 0,
	PhysicConstraint_ConeTwist,
	PhysicConstraint_Hinge,
	PhysicConstraint_Point,
	PhysicConstraint_Slider,
	PhysicConstraint_Dof6,
	PhysicConstraint_Dof6Spring,
	PhysicConstraint_Fixed,
	PhysicConstraint_Maximum
};

const int PhysicRotOrder_XYZ = 0;
const int PhysicRotOrder_XZY = 1;
const int PhysicRotOrder_YXZ = 2;
const int PhysicRotOrder_YZX = 3;
const int PhysicRotOrder_ZXY = 4;
const int PhysicRotOrder_ZYX = 5;
const int PhysicRotOrder_Maximum = 6;

const int PhysicConstraintFlag_Barnacle = 0x1;
const int PhysicConstraintFlag_Gargantua = 0x2;
const int PhysicConstraintFlag_DeactiveOnNormalActivity = 0x4;
const int PhysicConstraintFlag_DeactiveOnDeathActivity = 0x8;
const int PhysicConstraintFlag_DeactiveOnCaughtByBarnacleActivity = 0x10;
const int PhysicConstraintFlag_DeactiveOnBarnacleCatchingActivity = 0x20;
const int PhysicConstraintFlag_DontResetPoseOnErrorCorrection = 0x40;
const int PhysicConstraintFlag_NonNative = (PhysicConstraintFlag_Barnacle | PhysicConstraintFlag_Gargantua);

const int PhysicConstraintFactorIdx_ConeTwistSwingSpanLimit1 = 0;
const int PhysicConstraintFactorIdx_ConeTwistSwingSpanLimit2 = 1;
const int PhysicConstraintFactorIdx_ConeTwistTwistSpanLimit = 2;
const int PhysicConstraintFactorIdx_ConeTwistSoftness = 3;
const int PhysicConstraintFactorIdx_ConeTwistBiasFactor = 4;
const int PhysicConstraintFactorIdx_ConeTwistRelaxationFactor = 5;

const int PhysicConstraintFactorIdx_HingeLowLimit = 0;
const int PhysicConstraintFactorIdx_HingeHighLimit = 1;
const int PhysicConstraintFactorIdx_HingeSoftness = 3;
const int PhysicConstraintFactorIdx_HingeBiasFactor = 4;
const int PhysicConstraintFactorIdx_HingeRelaxationFactor = 5;

const int PhysicConstraintFactorIdx_SliderLowerLinearLimit = 0;
const int PhysicConstraintFactorIdx_SliderUpperLinearLimit = 1;
const int PhysicConstraintFactorIdx_SliderLowerAngularLimit = 2;
const int PhysicConstraintFactorIdx_SliderUpperAngularLimit = 3;

const int PhysicConstraintFactorIdx_Dof6LowerLinearLimitX = 0;
const int PhysicConstraintFactorIdx_Dof6LowerLinearLimitY = 1;
const int PhysicConstraintFactorIdx_Dof6LowerLinearLimitZ = 2;
const int PhysicConstraintFactorIdx_Dof6UpperLinearLimitX = 3;
const int PhysicConstraintFactorIdx_Dof6UpperLinearLimitY = 4;
const int PhysicConstraintFactorIdx_Dof6UpperLinearLimitZ = 5;

const int PhysicConstraintFactorIdx_Dof6LowerAngularLimitX = 6;
const int PhysicConstraintFactorIdx_Dof6LowerAngularLimitY = 7;
const int PhysicConstraintFactorIdx_Dof6LowerAngularLimitZ = 8;
const int PhysicConstraintFactorIdx_Dof6UpperAngularLimitX = 9;
const int PhysicConstraintFactorIdx_Dof6UpperAngularLimitY = 10;
const int PhysicConstraintFactorIdx_Dof6UpperAngularLimitZ = 11;

const int PhysicConstraintFactorIdx_Dof6SpringEnableLinearSpringX = 12;
const int PhysicConstraintFactorIdx_Dof6SpringEnableLinearSpringY = 13;
const int PhysicConstraintFactorIdx_Dof6SpringEnableLinearSpringZ = 14;

const int PhysicConstraintFactorIdx_Dof6SpringEnableAngularSpringX = 15;
const int PhysicConstraintFactorIdx_Dof6SpringEnableAngularSpringY = 16;
const int PhysicConstraintFactorIdx_Dof6SpringEnableAngularSpringZ = 17;

const int PhysicConstraintFactorIdx_Dof6SpringLinearStiffnessX = 18;
const int PhysicConstraintFactorIdx_Dof6SpringLinearStiffnessY = 19;
const int PhysicConstraintFactorIdx_Dof6SpringLinearStiffnessZ = 20;

const int PhysicConstraintFactorIdx_Dof6SpringAngularStiffnessX = 21;
const int PhysicConstraintFactorIdx_Dof6SpringAngularStiffnessY = 22;
const int PhysicConstraintFactorIdx_Dof6SpringAngularStiffnessZ = 23;

const int PhysicConstraintFactorIdx_Dof6SpringLinearDampingX = 24;
const int PhysicConstraintFactorIdx_Dof6SpringLinearDampingY = 25;
const int PhysicConstraintFactorIdx_Dof6SpringLinearDampingZ = 26;

const int PhysicConstraintFactorIdx_Dof6SpringAngularDampingX = 27;
const int PhysicConstraintFactorIdx_Dof6SpringAngularDampingY = 28;
const int PhysicConstraintFactorIdx_Dof6SpringAngularDampingZ = 29;

const int PhysicConstraintFactorIdx_LinearERP = 32;
const int PhysicConstraintFactorIdx_LinearCFM = 33;
const int PhysicConstraintFactorIdx_LinearStopERP = 34;
const int PhysicConstraintFactorIdx_LinearStopCFM = 35;
const int PhysicConstraintFactorIdx_AngularERP = 36;
const int PhysicConstraintFactorIdx_AngularCFM = 37;
const int PhysicConstraintFactorIdx_AngularStopERP = 38;
const int PhysicConstraintFactorIdx_AngularStopCFM = 39;
const int PhysicConstraintFactorIdx_RigidBodyLinearDistanceOffset = 40;

const int PhysicConstraintFactorIdx_Maximum = 64;

const float PhysicConstraintFactorDefaultValue_ConeTwistSoftness = BULLET_DEFAULT_SOFTNESS;
const float PhysicConstraintFactorDefaultValue_ConeTwistBiasFactor = BULLET_DEFAULT_BIAS_FACTOR;
const float PhysicConstraintFactorDefaultValue_ConeTwistRelaxationFactor = BULLET_DEFAULT_RELAXTION_FACTOR;

const float PhysicConstraintFactorDefaultValue_HingeSoftness = BULLET_DEFAULT_SOFTNESS;
const float PhysicConstraintFactorDefaultValue_HingeBiasFactor = BULLET_DEFAULT_BIAS_FACTOR;
const float PhysicConstraintFactorDefaultValue_HingeRelaxationFactor = BULLET_DEFAULT_RELAXTION_FACTOR;

const float PhysicConstraintFactorDefaultValue_Dof6SpringEnableLinearSpringX = 0;
const float PhysicConstraintFactorDefaultValue_Dof6SpringEnableLinearSpringY = 0;
const float PhysicConstraintFactorDefaultValue_Dof6SpringEnableLinearSpringZ = 0;

const float PhysicConstraintFactorDefaultValue_Dof6SpringEnableAngularSpringX = 0;
const float PhysicConstraintFactorDefaultValue_Dof6SpringEnableAngularSpringY = 0;
const float PhysicConstraintFactorDefaultValue_Dof6SpringEnableAngularSpringZ = 0;

const float PhysicConstraintFactorDefaultValue_LinearERP = BULLET_DEFAULT_LINEAR_ERP;
const float PhysicConstraintFactorDefaultValue_LinearCFM = BULLET_DEFAULT_LINEAR_CFM;
const float PhysicConstraintFactorDefaultValue_LinearStopERP = BULLET_DEFAULT_LINEAR_STOP_ERP;
const float PhysicConstraintFactorDefaultValue_LinearStopCFM = BULLET_DEFAULT_LINEAR_STOP_CFM;
const float PhysicConstraintFactorDefaultValue_AngularERP = BULLET_DEFAULT_ANGULAR_ERP;
const float PhysicConstraintFactorDefaultValue_AngularCFM = BULLET_DEFAULT_ANGULAR_CFM;
const float PhysicConstraintFactorDefaultValue_AngularStopERP = BULLET_DEFAULT_ANGULAR_STOP_ERP;
const float PhysicConstraintFactorDefaultValue_AngularStopCFM = BULLET_DEFAULT_ANGULAR_STOP_CFM;
const float PhysicConstraintFactorDefaultValue_RigidBodyLinearDistanceOffset = 0;

enum PhysicShape
{
	PhysicShape_None,
	PhysicShape_Box,
	PhysicShape_Sphere,
	PhysicShape_Capsule,
	PhysicShape_Cylinder,
	PhysicShape_MultiSphere,
	PhysicShape_TriangleMesh,
	PhysicShape_Compound,
	PhysicShape_Maximum
};

enum PhysicAction
{
	PhysicAction_None = 0,
	PhysicAction_BarnacleDragForce,
	PhysicAction_BarnacleChewForce,
	PhysicAction_BarnacleConstraintLimitAdjustment,
	PhysicAction_SimpleBuoyancy,
	PhysicAction_Maximum
};

const int PhysicActionFactorIdx_BarnacleDragForceMagnitude = 0;
const int PhysicActionFactorIdx_BarnacleDragForceExtraHeight = 1;

const int PhysicActionFactorIdx_BarnacleChewForceMagnitude = 0;
const int PhysicActionFactorIdx_BarnacleChewForceInterval = 1;

const int PhysicActionFactorIdx_BarnacleConstraintLimitAdjustmentExtraHeight = 1;
const int PhysicActionFactorIdx_BarnacleConstraintLimitAdjustmentInterval = 2;
const int PhysicActionFactorIdx_BarnacleConstraintLimitAdjustmentAxis = 3;

const int PhysicActionFactorIdx_SimpleBuoyancyMagnitude = 0;
const int PhysicActionFactorIdx_SimpleBuoyancyLinearDrag = 1;
const int PhysicActionFactorIdx_SimpleBuoyancyAngularDrag = 2;

const int PhysicActionFactorIdx_Maximum = 16;

const int PhysicActionFlag_Barnacle = 0x1;
const int PhysicActionFlag_Gargantua = 0x2;
//const int PhysicActionFlag_AffectsRigidBody = 0x4;
//const int PhysicActionFlag_AffectsConstraint = 0x8;
const int PhysicActionFlag_NonNative = (PhysicActionFlag_Barnacle | PhysicActionFlag_Gargantua);

enum PhysicShapeDirection
{
	PhysicShapeDirection_X = 0,
	PhysicShapeDirection_Y,
	PhysicShapeDirection_Z,
	PhysicShapeDirection_Maximum
};

enum StudioAnimActivityType
{
	StudioAnimActivityType_Idle,
	StudioAnimActivityType_Death,
	StudioAnimActivityType_CaughtByBarnacle,
	StudioAnimActivityType_BarnacleCatching,
	StudioAnimActivityType_Debug,
	StudioAnimActivityType_Maximum,
};

const int PhysicIndexArrayFlag_FromBSP = 0x1;
const int PhysicIndexArrayFlag_LoadFailed = 0x2;
const int PhysicIndexArrayFlag_FromOBJ = 0x4;
const int PhysicIndexArrayFlag_FromExternal = (PhysicIndexArrayFlag_FromOBJ);

class CPhysicBrushVertex
{
public:
	vec3_t	pos{ 0 };
};

class CPhysicBrushFace
{
public:
	int start_vertex{};
	int num_vertexes{};
};

class CPhysicVertexArray
{
public:
	std::vector<CPhysicBrushVertex> vVertexBuffer;
	std::vector<CPhysicBrushFace> vFaceBuffer;
};

class CPhysicIndexArray
{
public:
	std::shared_ptr<CPhysicVertexArray> pVertexArray;
	std::vector<int> vIndexBuffer;
	int flags{};
};