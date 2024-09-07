#if !defined CS_PLAYER_SHARED_H
#define CS_PLAYER_SHARED_H

// 
// Configuration for using high priority entities by CS players
//
class CConfigurationForHighPriorityUseEntity_t
{
public:
	enum EPriority_t
	{	// Priority of use entities, higher number is higher priority for use
		k_EPriority_Default,
		k_EPriority_Hostage,
		k_EPriority_Bomb
	};
	enum EPlayerUseType_t
	{
		k_EPlayerUseType_Start,		// Player wants to initiate the use
		k_EPlayerUseType_Progress	// Player wants to make progress using the entity
	};
	enum EDistanceCheckType_t
	{
		k_EDistanceCheckType_3D,
		k_EDistanceCheckType_2D
	};

	CBaseEntity *m_pEntity;
	EPriority_t m_ePriority;
	EDistanceCheckType_t m_eDistanceCheckType;
	Vector m_pos;
	float m_flMaxUseDistance;
	float m_flLosCheckDistance;
	float m_flDotCheckAngle;
	float m_flDotCheckAngleMax;

public:
	// Check if this high priority use entity is better for use than the other one
	bool IsBetterForUseThan( CConfigurationForHighPriorityUseEntity_t const &other ) const;

	// Check if this entity can be used by the given player according to its use rules
	bool UseByPlayerNow( CCSPlayer *pPlayer, EPlayerUseType_t ePlayerUseType );
};

#endif // CS_PLAYER_SHARED_H
