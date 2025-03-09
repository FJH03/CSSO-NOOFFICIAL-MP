//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basecsgrenade_projectile.h"

#ifdef CLIENT_DLL

	#include "c_cs_player.h"

#else

	#include "bot_manager.h"
	#include "cs_player.h"
	#include "soundent.h"
	#include "te_effect_dispatch.h"
	#include "KeyValues.h"
	#include "cs_simple_hostage.h"

	BEGIN_DATADESC( CBaseCSGrenadeProjectile )
		DEFINE_THINKFUNC( DangerSoundThink ),
	END_DATADESC()

	#define GRENADE_FAILSAFE_MAX_BOUNCES 20

#endif


IMPLEMENT_NETWORKCLASS_ALIASED( BaseCSGrenadeProjectile, DT_BaseCSGrenadeProjectile )

BEGIN_NETWORK_TABLE( CBaseCSGrenadeProjectile, DT_BaseCSGrenadeProjectile )
	#ifdef CLIENT_DLL
		RecvPropVector( RECVINFO( m_vInitialVelocity ) ),
		RecvPropInt( RECVINFO( m_nBounces ) )
	#else
		SendPropVector( SENDINFO( m_vInitialVelocity ), 
			20,		// nbits
			0,		// flags
			-3000,	// low value
			3000	// high value
			),
		SendPropInt( SENDINFO( m_nBounces ) )
	#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL


	void CBaseCSGrenadeProjectile::PostDataUpdate( DataUpdateType_t type )
	{
		BaseClass::PostDataUpdate( type );

		if ( type == DATA_UPDATE_CREATED )
		{
			// Now stick our initial velocity into the interpolation history 
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
			
			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

			// Add a sample 1 second back.
			Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
			interpolator.AddToHead( changeTime - 1.0, &vCurOrigin, false );

			// Add the current sample.
			vCurOrigin = GetLocalOrigin();
			interpolator.AddToHead( changeTime, &vCurOrigin, false );
		}
	}

	int CBaseCSGrenadeProjectile::DrawModel( int flags )
	{
		// During the first half-second of our life, don't draw ourselves if he's
		// still playing his throw animation.
		// (better yet, we could draw ourselves in his hand).
		if ( GetThrower() != C_BasePlayer::GetLocalPlayer() )
		{
			if ( gpGlobals->curtime - m_flSpawnTime < 0.5 )
			{
				C_CSPlayer *pPlayer = dynamic_cast<C_CSPlayer*>( GetThrower() );
				if ( pPlayer && pPlayer->m_PlayerAnimState->IsThrowingGrenade() )
				{
					return 0;
				}
			}
		}

		return BaseClass::DrawModel( flags );
	}

	void CBaseCSGrenadeProjectile::Spawn()
	{
		m_flSpawnTime = gpGlobals->curtime;
		BaseClass::Spawn();
	}

#else

	void CBaseCSGrenadeProjectile::PostConstructor( const char *className )
	{
		BaseClass::PostConstructor( className );
		TheBots->AddGrenade( this );
	}

	CBaseCSGrenadeProjectile::~CBaseCSGrenadeProjectile()
	{
		TheBots->RemoveGrenade( this );
	}

	void CBaseCSGrenadeProjectile::Spawn( void )
	{
		BaseClass::Spawn();

		SetSolidFlags( FSOLID_NOT_STANDABLE );
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
		SetSolid( SOLID_BBOX );	// So it will collide with physics props!
		AddFlag( FL_GRENADE );

		// smaller, cube bounding box so we rest on the ground
		Vector min = Vector( -2, -2, -2 );
		Vector max = Vector( 2, 2, 2 );

		SetSize( min, max );
 		if ( CollisionProp( ) )
 			CollisionProp( )->SetCollisionBounds( min, max );

		m_nBounces = 0;
	}

	int	CBaseCSGrenadeProjectile::UpdateTransmitState()
	{
		// always call ShouldTransmit() for grenades
		return SetTransmitState( FL_EDICT_FULLCHECK );
	}

	int CBaseCSGrenadeProjectile::ShouldTransmit( const CCheckTransmitInfo *pInfo )
	{
		CBaseEntity *pRecipientEntity = CBaseEntity::Instance( pInfo->m_pClientEnt );
		if ( pRecipientEntity->IsPlayer() )
		{
			CBasePlayer *pRecipientPlayer = static_cast<CBasePlayer*>( pRecipientEntity );

			// always transmit to the thrower of the grenade
			if ( pRecipientPlayer && ( (GetThrower() && pRecipientPlayer == GetThrower()) ||
				pRecipientPlayer->GetTeamNumber() == TEAM_SPECTATOR) )
			{
				return FL_EDICT_ALWAYS;
			}
		}

		return FL_EDICT_PVSCHECK;
	}

	void CBaseCSGrenadeProjectile::DangerSoundThink( void )
	{
		if (!IsInWorld())
		{
			Remove( );
			return;
		}

		if( gpGlobals->curtime > m_flDetonateTime )
		{
			Detonate();
			return;
		}

		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, GetAbsVelocity().Length( ), 0.2 );

		SetNextThink( gpGlobals->curtime + 0.2 );

		if (GetWaterLevel() != WL_NotInWater)
		{
			SetAbsVelocity( GetAbsVelocity() * 0.5 );
		}
	}

	//Sets the time at which the grenade will explode
	void CBaseCSGrenadeProjectile::SetDetonateTimerLength( float timer )
	{
		m_flDetonateTime = gpGlobals->curtime + timer;
	}

	void CBaseCSGrenadeProjectile::SetThrownBodygroup( void )
	{
		int iBodygroup = FindBodygroupByName("pin");
		if ( iBodygroup )
			SetBodygroup( iBodygroup, 1 );

		iBodygroup = FindBodygroupByName( "spoon" );
		if ( iBodygroup )
			SetBodygroup( iBodygroup, 1 );
	}

	void CBaseCSGrenadeProjectile::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
	{
		const float kSleepVelocity = 20.0f;
		const float kSleepVelocitySquared = kSleepVelocity * kSleepVelocity;

		// Verify that we have an entity.
		CBaseEntity *pEntity = trace.m_pEnt;
		Assert( pEntity );

		// if its breakable glass and we kill it, don't bounce.
		// give some damage to the glass, and if it breaks, pass 
		// through it.
		bool breakthrough = false;

		if( pEntity && FClassnameIs( pEntity, "func_breakable" ) )
		{
			breakthrough = true;
		}

		if( pEntity && FClassnameIs( pEntity, "func_breakable_surf" ) )
		{
			breakthrough = true;
		}

		if( pEntity && FClassnameIs( pEntity, "prop_physics_multiplayer" ) && pEntity->GetMaxHealth() > 0 && pEntity->m_takedamage == DAMAGE_YES )
		{
			breakthrough = true;
		}

		// this one is tricky because BounceTouch hits breakable propers before we hit this function and the damage is already applied there (CBaseGrenade::BounceTouch( CBaseEntity *pOther ))
		// by the time we hit this, the prop hasn't been removed yet, but it broke, is set to not take anymore damage and is marked for deletion - we have to cover this case here
		if( pEntity && FClassnameIs( pEntity, "prop_dynamic" ) && pEntity->GetMaxHealth() > 0 && (pEntity->m_takedamage == DAMAGE_YES || (pEntity->m_takedamage == DAMAGE_NO && pEntity->IsEFlagSet( EFL_KILLME ))) )
		{
			breakthrough = true;
		}

		if ( breakthrough )
		{
			CTakeDamageInfo info( this, this, 10, DMG_CLUB );
			pEntity->DispatchTraceAttack( info, GetAbsVelocity().Normalized(), &trace );

			ApplyMultiDamage();

			if( pEntity->m_iHealth <= 0 )
			{
				// slow our flight a little bit
				Vector vel = GetAbsVelocity();

				vel *= 0.4;

				SetAbsVelocity( vel );
				return;
			}
		}


		//Assume all surfaces have the same elasticity
		float flSurfaceElasticity = 1.0;

		//Don't bounce off of players with perfect elasticity
		if ( pEntity && pEntity->IsPlayer() )
		{
			flSurfaceElasticity = 0.3f;

			// and do slight damage to players on the opposite team
			if ( GetTeamNumber() != pEntity->GetTeamNumber() )
			{
				CTakeDamageInfo info( this, GetThrower(), 2, DMG_GENERIC );

				pEntity->TakeDamage( info );
			}
		}

		//Don't bounce twice on a selection of problematic entities
		bool bIsProjectile = dynamic_cast< CBaseCSGrenadeProjectile* >( pEntity ) != NULL;
		if ( pEntity && !pEntity->IsWorld() && m_lastHitPlayer.Get() == pEntity )
		{
			bool bIsHostage = dynamic_cast< CHostage* >( pEntity ) != NULL;
			if (  pEntity->IsPlayer() || bIsHostage || bIsProjectile )
			{
				//DevMsg( "Setting %s to DEBRIS, it is in group %i, it hit %s in group %i\n", this->GetClassname(), this->GetCollisionGroup(), pEntity->GetClassname(), pEntity->GetCollisionGroup() );
				SetCollisionGroup( COLLISION_GROUP_DEBRIS );
				if ( bIsProjectile )
				{
					//DevMsg( "Setting %s to DEBRIS, it is in group %i.\n", pEntity->GetClassname(), pEntity->GetCollisionGroup() );
					pEntity->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
				}
				return;
			}
		}
		if ( pEntity )
		{
			m_lastHitPlayer = pEntity;
		}

		float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
		flTotalElasticity = clamp( flTotalElasticity, 0.0f, 0.9f );

		// NOTE: A backoff of 2.0f is a reflection
		Vector vecAbsVelocity;
		PhysicsClipVelocity( GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f );
		vecAbsVelocity *= flTotalElasticity;

		// Get the total velocity (player + conveyors, etc.)
		VectorAdd( vecAbsVelocity, GetBaseVelocity(), vecVelocity );
		float flSpeedSqr = DotProduct( vecVelocity, vecVelocity );

		bool bIsWeapon = dynamic_cast< CBaseCombatWeapon* >( pEntity ) != NULL;
		
		// Stop if on ground or if we bounce and our velocity is really low (keeps it from bouncing infinitely)
		if ( pEntity &&
			( ( trace.plane.normal.z > 0.7f ) || (trace.plane.normal.z > 0.1f && flSpeedSqr < kSleepVelocitySquared) ) &&
			( pEntity->IsStandable() || bIsProjectile || bIsWeapon || pEntity->IsWorld() ) 
			)
		{
			// clip it again to emulate old behavior and keep it from bouncing up like crazy when you throw it at the ground on the first toss
			if ( flSpeedSqr > 96000 )
			{
				float alongDist = DotProduct( vecAbsVelocity.Normalized(), trace.plane.normal );
				if ( alongDist > 0.5f )
				{
					float flBouncePadding = (1.0f - alongDist) + 0.5f;
					vecAbsVelocity *= flBouncePadding;
				}
			}

			SetAbsVelocity( vecAbsVelocity );

			if ( flSpeedSqr < kSleepVelocitySquared )
			{
				SetGroundEntity( pEntity );

				// Reset velocities.
				SetAbsVelocity( vec3_origin );
				SetLocalAngularVelocity( vec3_angle );

				//align to the ground so we're not standing on end
				QAngle angle;
				VectorAngles( trace.plane.normal, angle );

				// rotate randomly in yaw
				angle[1] = random->RandomFloat( 0, 360 );

				// TODO: rotate around trace.plane.normal
				
				SetAbsAngles( angle );			
			}
			else
			{
				Vector vecBaseDir = GetBaseVelocity();
				if ( !vecBaseDir.IsZero() )
				{
					VectorNormalize( vecBaseDir );
					Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
					float flScale = vecDelta.Dot( vecBaseDir );
					vecAbsVelocity += GetBaseVelocity() * flScale;
				}
					
				VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
				
				PhysicsPushEntity( vecVelocity, &trace );
			}
		}
		else
		{
			SetAbsVelocity( vecAbsVelocity );
			VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
			PhysicsPushEntity( vecVelocity, &trace );
		}
		
		BounceSound();

		// tell the bots a grenade has bounced
		CCSPlayer *player = ToCSPlayer(GetThrower());
		if ( player )
		{
			IGameEvent * event = gameeventmanager->CreateEvent( "grenade_bounce" );
			if ( event )
			{
				event->SetInt( "userid", player->GetUserID() );
				event->SetFloat( "x", GetAbsOrigin().x );
				event->SetFloat( "y", GetAbsOrigin().y );
				event->SetFloat( "z", GetAbsOrigin().z );
				gameeventmanager->FireEvent( event );
			}
		}

		OnBounced();

		if (m_nBounces > GRENADE_FAILSAFE_MAX_BOUNCES )
		{
			//failsafe detonate after 20 bounces
			SetAbsVelocity( vec3_origin );
			DetonateOnNextThink();
			SetNextThink( gpGlobals->curtime );
			SetMoveType( MOVETYPE_NONE );
		}
		else
		{
			m_nBounces++;
		}
	}

	void CBaseCSGrenadeProjectile::SetupInitialTransmittedGrenadeVelocity( const Vector &velocity )
	{
		m_vInitialVelocity = velocity;
	}

	#define	MAX_WATER_SURFACE_DISTANCE	512

	void CBaseCSGrenadeProjectile::Splash()
	{
		Vector centerPoint = GetAbsOrigin();
		Vector normal( 0, 0, 1 );

		// Find our water surface by tracing up till we're out of the water
		trace_t tr;
		Vector vecTrace( 0, 0, MAX_WATER_SURFACE_DISTANCE );
		UTIL_TraceLine( centerPoint, centerPoint + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr );

		// If we didn't start in water, we're above it
		if ( tr.startsolid == false )
		{
			// Look downward to find the surface
			vecTrace.Init( 0, 0, -MAX_WATER_SURFACE_DISTANCE );
			UTIL_TraceLine( centerPoint, centerPoint + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr );

			// If we hit it, setup the explosion
			if ( tr.fraction < 1.0f )
			{
				centerPoint = tr.endpos;
			}
			else
			{
				//NOTENOTE: We somehow got into a splash without being near water?
				Assert( 0 );
			}
		}
		else if ( tr.fractionleftsolid )
		{
			// Otherwise we came out of the water at this point
			centerPoint = centerPoint + (vecTrace * tr.fractionleftsolid);
		}
		else
		{
			// Use default values, we're really deep
		}

		CEffectData	data;
 		data.m_vOrigin = centerPoint;
		data.m_vNormal = normal;
		data.m_flScale = random->RandomFloat( 1.0f, 2.0f );

		if ( GetWaterType() & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect( "gunshotsplash", data );
	}

#endif // !CLIENT_DLL