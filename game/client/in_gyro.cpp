//========= Gyroscope Input Support ============//

#include "cbase.h"
#include "in_gyro.h"
#include "inputsystem/iinputsystem.h"
#include "cstrike/c_cs_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// up / down
#define PITCH 0
// left / right
#define YAW   1

// ---------------------------------------------------------------------------
// CVars
// ---------------------------------------------------------------------------

static ConVar gyro_enable(
    "gyro_enable", "0", FCVAR_ARCHIVE,
    "Enable gyroscope aiming (requires a device with a gyroscope sensor)." );

static ConVar gyro_pitch_sensitivity(
    "gyro_pitch_sensitivity", "1.0", FCVAR_ARCHIVE,
    "Gyroscope pitch (up/down) sensitivity multiplier.",
    true, 0.0f, false, 2.0f );

static ConVar gyro_yaw_sensitivity(
    "gyro_yaw_sensitivity", "1.0", FCVAR_ARCHIVE,
    "Gyroscope yaw (left/right) sensitivity multiplier.",
    true, 0.0f, false, 2.0f );

static ConVar gyro_reverse_pitch(
    "gyro_reverse_pitch", "0", FCVAR_ARCHIVE,
    "Invert the gyroscope pitch axis (1 = inverted)." );

static ConVar gyro_reverse_yaw(
    "gyro_reverse_yaw", "0", FCVAR_ARCHIVE,
    "Invert the gyroscope yaw axis (1 = inverted)." );

static ConVar gyro_deadzone(
    "gyro_deadzone", "1.0", FCVAR_ARCHIVE,
    "Gyroscope deadzone in degrees/second. "
    "Rotation rates below this threshold are ignored.",
    true, 0.0f, false, 3.0f );

static ConVar gyro_scoped_sensitivity(
    "gyro_scoped_sensitivity", "0.5", FCVAR_ARCHIVE,
    "Gyroscope sensitivity multiplier applied while the player is scoped. "
    "Scales both pitch and yaw on top of their base sensitivities. "
    "0.5 = half speed while scoped, 1.0 = no change.",
    true, 0.0f, true, 2.0f );

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void Gyro_Init( void )
{
    // CVars are registered automatically as static ConVar objects above.
    // Nothing else needed here; kept as a named entry point so callers
    // can be self-documenting.
}

void Gyro_ApplyMove( float frametime )
{
    if ( !gyro_enable.GetBool() )
        return;

    if ( !inputsystem )
        return;

    float pitch_dps = 0.0f;
    float yaw_dps   = 0.0f;

    if ( !inputsystem->GetGyroAccumulators( pitch_dps, yaw_dps ) )
        return;  // gyro hardware not available

    // Apply deadzone: if the rotation rate is below the threshold, discard it.
    const float dz = gyro_deadzone.GetFloat();
    if ( fabsf( pitch_dps ) < dz )
        pitch_dps = 0.0f;
    if ( fabsf( yaw_dps ) < dz )
        yaw_dps = 0.0f;

    if ( pitch_dps == 0.0f && yaw_dps == 0.0f )
        return;

    // Inversion
    if ( gyro_reverse_pitch.GetBool() )
        pitch_dps = -pitch_dps;
    if ( gyro_reverse_yaw.GetBool() )
        yaw_dps = -yaw_dps;

    // When the local player is scoped
    float scopedMult = 1.0f;
    C_CSPlayer *pPlayer = C_CSPlayer::GetLocalCSPlayer();
    if ( pPlayer && pPlayer->m_bIsScoped )
        scopedMult = gyro_scoped_sensitivity.GetFloat();

    // Integrate over frametime to get the angular delta in degrees.
    float dPitch = gyro_pitch_sensitivity.GetFloat() * scopedMult * pitch_dps * frametime;
    float dYaw   = gyro_yaw_sensitivity.GetFloat()   * scopedMult * yaw_dps   * frametime;

    // Fetch current view angles from the engine and apply the gyro delta.
    QAngle viewangles;
    engine->GetViewAngles( viewangles );

    viewangles[PITCH] += dPitch;
    viewangles[YAW]   -= dYaw;   // Negative: device rotates right -> yaw increases right

    // Clamp pitch to sane limits
    extern ConVar cl_pitchup;
    extern ConVar cl_pitchdown;
    viewangles[PITCH] = clamp( viewangles[PITCH],
                                -cl_pitchup.GetFloat(),
                                 cl_pitchdown.GetFloat() );

    engine->SetViewAngles( viewangles );
}
