//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "predicted_viewmodel.h"

#ifdef CLIENT_DLL 
 #include "prediction.h" 
 #ifdef CSTRIKE_DLL
 #include "c_cs_player.h" 
 #endif 
 #endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( predicted_viewmodel, CPredictedViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( PredictedViewModel, DT_PredictedViewModel )

BEGIN_NETWORK_TABLE( CPredictedViewModel, DT_PredictedViewModel )
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
CPredictedViewModel::CPredictedViewModel() : m_LagAnglesHistory("CPredictedViewModel::m_LagAnglesHistory")
{
	m_vLagAngles.Init();
	m_LagAnglesHistory.Setup( &m_vLagAngles, 0 );
}
#else
CPredictedViewModel::CPredictedViewModel()
{
}
#endif


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPredictedViewModel::~CPredictedViewModel()
{
}

#ifdef CLIENT_DLL
ConVar cl_wpn_sway_interp( "cl_wpn_sway_interp", "0.1", FCVAR_CLIENTDLL );
ConVar cl_wpn_sway_scale( "cl_wpn_sway_scale", "1.0", FCVAR_CLIENTDLL|FCVAR_CHEAT );

#ifdef CSTRIKE_DLL
 extern ConVar        cmod_new_bobbing; 
#endif //cstrike_dll 
#endif //client_dll

//----------------------------------------------------------------------------- 
 // Purpose:  Adds head bob for off hand models 
 //----------------------------------------------------------------------------- 
 void CPredictedViewModel::AddViewModelBob( CBasePlayer *owner, Vector& eyePosition, QAngle& eyeAngles ) 
 { 
 #ifdef CSTRIKE_DLL
 #ifdef CLIENT_DLL 
         if ( cmod_new_bobbing.GetBool() == false ) 
                 return; 

         // if we are an off hand view model (index 1) and we have a model, add head bob. 
         // (Head bob for main hand model added by the weapon itself.) 
         if ( ViewModelIndex() == 1 && m_bShouldIgnoreOffsetAndAccuracy ) 
         { 
                CalcNewViewModelBobbing( owner, &m_BobState, 1 ); 
                AddNewViewModelBobbing( eyePosition, eyeAngles, &m_BobState ); 
         } 
 #endif 
 #endif 
 }

void CPredictedViewModel::CalcViewModelLag( Vector& origin, QAngle& angles, QAngle& original_angles )
{
	#ifdef CLIENT_DLL
		// Calculate our drift
		Vector	forward, right, up;
		AngleVectors( angles, &forward, &right, &up );
		
		// Add an entry to the history.
		m_vLagAngles = angles;
		m_LagAnglesHistory.NoteChanged( gpGlobals->curtime, cl_wpn_sway_interp.GetFloat(), false );
		
		// Interpolate back 100ms.
		m_LagAnglesHistory.Interpolate( gpGlobals->curtime, cl_wpn_sway_interp.GetFloat() );
		
		// Now take the 100ms angle difference and figure out how far the forward vector moved in local space.
		Vector vLaggedForward;
		QAngle angleDiff = m_vLagAngles - angles;
		AngleVectors( -angleDiff, &vLaggedForward, 0, 0 );
		Vector vForwardDiff = Vector(1,0,0) - vLaggedForward;

		// Now offset the origin using that.
		vForwardDiff *= cl_wpn_sway_scale.GetFloat();
		origin += forward*vForwardDiff.x + right*-vForwardDiff.y + up*vForwardDiff.z;
	#endif
}