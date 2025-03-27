//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules object
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef CS_GAMERULES_H
#define CS_GAMERULES_H

#ifdef _WIN32
#pragma once
#endif


#include "teamplay_gamerules.h"
#include "convar.h"
#include "cs_shareddefs.h"
#include "gamevars_shared.h"

#ifdef CLIENT_DLL
	#include "c_cs_player.h"
	#include "networkstringtable_clientdll.h"
#else
	#include "cs_player.h"
	#include "funfactmgr_cs.h"
#endif

#define	WINNER_NONE		0
#define WINNER_DRAW		1
#define WINNER_TER		TEAM_TERRORIST
#define WINNER_CT		TEAM_CT

#define CUSTOM_BOT_DIFFICULTY_NOBOTS	0
#define CUSTOM_BOT_DIFFICULTY_DUMB		1
#define CUSTOM_BOT_DIFFICULTY_EASY		2
#define CUSTOM_BOT_DIFFICULTY_MEDIUM	3
#define CUSTOM_BOT_DIFFICULTY_HARD		4
#define CUSTOM_BOT_DIFFICULTY_EXPERT	5

#define CUSTOM_BOT_MIN_DIFFICULTY_FOR_AWARDS_PROGRESS	CUSTOM_BOT_DIFFICULTY_EASY

#define MAX_WEAPON_NAME_POPUP_RANGE 128.0

class CCSBot;
class CHostage;

extern ConVar mp_startmoney;
extern ConVar mp_maxmoney;
extern ConVar mp_tkpunish;
extern ConVar mp_c4timer;
extern ConVar mp_buytime;
extern ConVar mp_freezetime;
extern ConVar mp_playerid;
extern ConVar mp_defuser_allocation;
extern ConVar mp_death_drop_gun;
extern ConVar mp_death_drop_grenade;
extern ConVar mp_death_drop_defuser;
extern ConVar ammo_grenade_limit_total;

#ifndef CLIENT_DLL
	extern ConVar mp_autoteambalance;
#endif // !CLIENT_DLL

//--------------------------------------------------------------------------------------------------------------
struct GGWeaponAliasName
{
	CSWeaponID id;
	const char *aliasName;
};

#define GGLIST_PISTOLS_TOTAL		9
#define GGLIST_RIFLES_TOTAL			7
#define GGLIST_MGS_TOTAL			2
#define GGLIST_SGS_TOTAL			4
#define GGLIST_SMGS_TOTAL			6
#define GGLIST_SNIPERS_TOTAL		3

#define GGLIST_PISTOLS_START	0
#define GGLIST_PISTOLS_LAST		(GGLIST_PISTOLS_START+GGLIST_PISTOLS_TOTAL-1)
#define GGLIST_RIFLES_START		GGLIST_PISTOLS_LAST+1
#define GGLIST_RIFLES_LAST		(GGLIST_RIFLES_START+GGLIST_RIFLES_TOTAL-1)
#define GGLIST_MGS_START		GGLIST_RIFLES_LAST+1
#define GGLIST_MGS_LAST			(GGLIST_MGS_START+GGLIST_MGS_TOTAL-1)
#define GGLIST_SGS_START		GGLIST_MGS_LAST+1
#define GGLIST_SGS_LAST			(GGLIST_SGS_START+GGLIST_SGS_TOTAL-1)
#define GGLIST_SMGS_START		GGLIST_SGS_LAST+1
#define GGLIST_SMGS_LAST			(GGLIST_SMGS_START+GGLIST_SMGS_TOTAL-1)
#define GGLIST_SNIPERS_START		GGLIST_SMGS_LAST+1
#define GGLIST_SNIPERS_LAST			(GGLIST_SNIPERS_START+GGLIST_SNIPERS_TOTAL-1)

//--------------------------------------------------------------------------------------------------------------
// NOTE: Array must be NULL-terminated
static GGWeaponAliasName ggWeaponAliasNameList[] =
{
	//pistols
	{ WEAPON_DEAGLE, "deagle" },
	{ WEAPON_REVOLVER, "revolver" },
	{ WEAPON_ELITE, "elite" },
	{ WEAPON_FIVESEVEN, "fiveseven" },
	{ WEAPON_GLOCK, "glock" },
	{ WEAPON_TEC9, "tec9" },
	{ WEAPON_HKP2000, "hkp2000" },
	{ WEAPON_USP, "usp_silencer" },
	{ WEAPON_P250, "p250" },

	//rifles
	{ WEAPON_AK47, "ak47" },
	{ WEAPON_AUG, "aug" },
	{ WEAPON_FAMAS, "famas" },
	{ WEAPON_GALILAR, "galilar" },
	{ WEAPON_M4A4, "m4a4" },
	{ WEAPON_M4A1, "m4a1_silencer" },
	{ WEAPON_SG556, "sg556" },

	//mgs
	{ WEAPON_M249, "m249" },
	{ WEAPON_NEGEV, "negev" },

	//shotguns
	{ WEAPON_XM1014, "xm1014" },
	{ WEAPON_MAG7, "mag7" },
	{ WEAPON_SAWEDOFF, "sawedoff" },
	{ WEAPON_NOVA, "nova" },

	//smgs
	{ WEAPON_MAC10, "mac10" },
	{ WEAPON_P90, "p90" },
	{ WEAPON_UMP45, "ump45" },
	{ WEAPON_BIZON, "bizon" },
	{ WEAPON_MP7, "mp7" },
	{ WEAPON_MP9, "mp9" },

	//snipers
	//	{ WEAPON_SSG08, "ssg08" },
	{ WEAPON_SCAR20, "scar20" },
	{ WEAPON_G3SG1, "g3sg1" },
	{ WEAPON_AWP, "awp" },

	{ WEAPON_NONE, "" }
};

#ifdef CLIENT_DLL
	#define CCSGameRules C_CSGameRules
	#define CCSGameRulesProxy C_CSGameRulesProxy
#endif

#ifndef CLIENT_DLL
class CCSMatch
{		
public:
	CCSMatch();

	void Reset( void );

	void SetPhase( GamePhase phase );
	GamePhase GetPhase( void ) const { return m_phase; }

	//These functions add to both the score and the number of rounds
	void AddTerroristWins( int numWins );
	void AddCTWins( int numWins);
	void IncrementRound( int nNumRounds );

	//These functions only adjust the score (without adding to the number of rounds played)
	void AddTerroristBonusPoints( int numWins );
	void AddCTBonusPoints( int numWins);

	int GetTerroristScore( void ) const 	{ return m_terroristScoreTotal; }
	int GetCTScore( void ) const  			{ return m_ctScoreTotal; }	

	int GetRoundsPlayed( void ) const		{ return m_actualRoundsPlayed; }	
	
	//Since the teams change in halftime modes and we want to retain their scores, we swap the scores
	//between halves.
	void SwapTeamScores( void );
	
	int GetWinningTeam( void );	

	//These are the internal functions that actually mess with the scores, adjusting the appropriate phase-specific scores as well.
	void AddTerroristScore( int score );
	void AddCTScore( int score );
	void GoToOvertime( int numOvertimesToAdd );

private:
	//This is called anytime the match-internal scores are updated to reflect the changes in the team object (so the scores can be replicated)
	void UpdateTeamScores( void );

	// Called when we wish to change the full all-talk rules, based on entering a specific phase of the match
	void EnableFullAlltalk( bool bEnable );

	//This is the number of rounds that have been played, regardless of the actual score of the match (e.g. Demolition mode can give bonus points)
	short m_actualRoundsPlayed;

	// This is the index of the overtime that is being played, 0 when game has no overtime or is still in regulation time, 1 for first overtime, 2 for second, etc.
	short m_nOvertimePlaying;

	short m_ctScoreFirstHalf;
	short m_ctScoreSecondHalf;
	short m_ctScoreOvertime;
	short m_ctScoreTotal;

	short m_terroristScoreFirstHalf;
	short m_terroristScoreSecondHalf;
	short m_terroristScoreOvertime;
	short m_terroristScoreTotal;

	GamePhase m_phase;
};

class SpawnPoint : public CServerOnlyPointEntity
{
	DECLARE_CLASS( SpawnPoint, CServerOnlyPointEntity );
	DECLARE_DATADESC();
public:
	SpawnPoint();
	void Spawn( void );
	bool IsEnabled() { return m_bEnabled; }
	void InputSetEnabled( inputdata_t &inputdata );
	void InputSetDisabled( inputdata_t &inputdata );
	void InputToggleEnabled( inputdata_t &inputdata );
	void SetSpawnEnabled( bool bEnabled );

	int		m_iPriority;
	bool	m_bEnabled;
	int		m_nType;

	enum Type
	{
		Default = 0,
		Deathmatch = 1,
		ArmsRace = 2,
	};
};
#endif

class CCSGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CCSGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class CCSGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CCSGameRules, CTeamplayRules );

	// Stuff that is shared between client and server.
	bool IsFreezePeriod();
	bool IsWarmupPeriod() const;
	float GetWarmupPeriodEndTime() const;	
	bool IsWarmupPeriodPaused();
	void SetWarmupPeriodStartTime( float fl )	{ m_fWarmupPeriodStart = fl; }
	float GetWarmupPeriodStartTime( void )	{ return m_fWarmupPeriodStart; }
	float GetWarmupRemainingTime();

	bool IsTimeOutActive() const { return ( IsTerroristTimeOutActive() || IsCTTimeOutActive() ); }
	bool IsTerroristTimeOutActive() const { return m_bTerroristTimeOutActive; }
	bool IsCTTimeOutActive() const { return m_bCTTimeOutActive; }

	void StartTerroristTimeOut( void );
	void StartCTTimeOut( void );
	void EndTerroristTimeOut( void );
	void EndCTTimeOut( void );

	float GetCTTimeOutRemaining() const { return m_flCTTimeOutRemaining; }
	float GetTerroristTimeOutRemaining() const { return m_flTerroristTimeOutRemaining; }

	int GetCTTimeOuts( ) const { return m_nCTTimeOuts; }
	int GetTerroristTimeOuts( ) const { return m_nTerroristTimeOuts; }

	void StartWarmup( void );
	void EndWarmup( void );

	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );

	virtual float	GetRoundRestartTime( void ) { return m_flRestartRoundTime; }
	virtual bool	IsGameRestarting( void ) { return m_bGameRestart; }
	float GetMapRemainingTime();	// time till end of map, -1 if timelimit is disabled
	float GetMapElapsedTime();	// How much time has elapsed since the map started.
	float GetRoundRemainingTime();	// time till end of round
	float GetRoundStartTime();		// When this round started.
	float GetRoundElapsedTime();	// How much time has elapsed since the round started.
	float GetBuyTimeLength();
	int GetRoundLength() const { return m_iRoundTime; }
	int   SelectDefaultTeam( bool ignoreBots = false );
	int   GetHumanTeam();			// TEAM_UNASSIGNED if no restrictions

	void CalculateMaxGunGameProgressiveWeaponIndex( void );
	int GetMaxGunGameProgressiveWeaponIndex( void ) { return m_iMaxGunGameProgressiveWeaponIndex; }

	void	LoadMapProperties();
	bool	UseMapFactionsForThisPlayer( CBasePlayer* pPlayer, int iTeamNumber );
	int		GetMapFactionsForThisPlayer( CBasePlayer* pPlayer, int iTeamNumber );
	bool	MapFactionsDefined( int iTeamNumber );

	bool IsBombDefuseMap() const;
	bool IsHostageRescueMap() const;
	bool IsIntermission() const;
	bool IsLogoMap() const;
	bool IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer );
	bool IsSpawnPointHiddenFromOtherPlayers( CBaseEntity *pSpot, CBasePlayer *pPlayer, int nHideFromTeam = 0 );

	int GetMaxSpectatorSlots( void ) const;

	bool IsBuyTimeElapsed();
	bool IsMatchWaitingForResume( void );
	void SetMatchWaitingForResume( bool pause ) { m_bMatchWaitingForResume = pause; };

	bool IsAwardsProgressAllowedForBotDifficulty() const; // returns false if the user is playing offline with trivial bots (no bots, harmless bots)

#ifndef CLIENT_DLL
	bool IsArmorFree();
#endif
	bool IsTeammateSolid( void ) const;				// returns true if teammates are solid obstacles in the current game mode
	bool IsEnemySolid( void ) const;				// returns true if enemies are solid obstacles in the current game mode

	bool HasHalfTime( void ) const;

	int GetCustomBotDifficulty( void ) const;

	int GetCurrentGunGameWeapon( int nCurrentWeaponIndex, int nTeamID );
	int GetNextGunGameWeapon( int nCurrentWeaponIndex, int nTeamID );
	int GetPreviousGunGameWeapon( int nCurrentWeaponIndex, int nTeamID );
	bool IsFinalGunGameProgressiveWeapon( int nCurrentWeaponIndex, int nTeamID );
	int GetGunGameNumKillsRequiredForWeapon( int nCurrentWeaponIndex, int nTeamID );

	void AddGunGameWeapon( const char* pWeaponName, int nNumKillsToUpgrade, int nTeamID );
	int GetNumProgressiveGunGameWeapons( int nTeamID ) const;
	int GetProgressiveGunGameWeapon( int nWeaponIndex, int nTeamID ) const { return nTeamID == TEAM_CT ? m_GGProgressiveWeaponOrderCT[nWeaponIndex] : m_GGProgressiveWeaponOrderT[nWeaponIndex]; }
	int GetProgressiveGunGameWeaponKillRequirement( int nWeaponIndex, int nTeamID ) const { return nTeamID == TEAM_CT ? m_GGProgressiveWeaponKillUpgradeOrderCT[nWeaponIndex] : m_GGProgressiveWeaponKillUpgradeOrderT[nWeaponIndex]; }
	int GetGunGameTRBonusGrenade( CCSPlayer *pPlayer );

	virtual int	DefaultFOV();

	// Get the view vectors for this mod.
	virtual const CViewVectors* GetViewVectors() const;

	int  GetStartMoney( void );

	void AddHostageRescueTime( void );

	bool IsPlayingCustomGametype( void ) const;
	bool IsPlayingGunGameProgressive( void ) const;
	bool IsPlayingGunGameDeathmatch( void ) const;
	bool IsPlayingGunGameTRBomb( void ) const;
	bool IsPlayingGunGame( void ) const;
	bool IsPlayingClassic( void ) const;
	bool IsPlayingOffline( void ) const;

	bool IsPlayingClassicCasual( void ) const;
	bool IsPlayingAnyCompetitiveStrictRuleset( void ) const;

	GamePhase GetGamePhase( void ) const { return (GamePhase) m_gamePhase.Get(); }
	int GetTotalRoundsPlayed( void ) const { return m_totalRoundsPlayed; }
	int GetOvertimePlaying( void ) const { return m_nOvertimePlaying; }
	
	virtual bool IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer );

private:

	float GetExplosionDamageAdjustment(Vector & vecSrc, Vector & vecEnd, CBaseEntity *pEntityToIgnore); // returns multiplier between 0.0 and 1.0 that is the percentage of any damage done from vecSrc to vecEnd that actually makes it.
	float GetAmountOfEntityVisible(Vector & src, CBaseEntity *player); // returns a value from 0 to 1 that is the percentage of player visible from src.

	CNetworkVar( bool, m_bFreezePeriod );	 // TRUE at beginning of round, set to FALSE when the period expires
	CNetworkVar( bool, m_bWarmupPeriod );	 // 
	CNetworkVar( float, m_fWarmupPeriodStart );

	CNetworkVar( bool, m_bTerroristTimeOutActive );
	CNetworkVar( bool, m_bCTTimeOutActive );
	CNetworkVar( float, m_flTerroristTimeOutRemaining );
	CNetworkVar( float, m_flCTTimeOutRemaining );
	CNetworkVar( int, m_nTerroristTimeOuts );
	CNetworkVar( int, m_nCTTimeOuts );

	CNetworkVar( bool, m_bMatchWaitingForResume ); // When mp_pause_match is called, this state becomes true and will prevent the next freezetime from ending.
	CNetworkVar( int, m_iRoundTime );		 // (From mp_roundtime) - How many seconds long this round is.
	CNetworkVar( float, m_fRoundStartTime ); // time round has started
	CNetworkVar( float, m_flRestartRoundTime ); // the global time when the round is supposed to end, if this is not 0
	CNetworkVar( bool, m_bGameRestart ); // True = mp_restartgame is being processed
	CNetworkVar( float, m_flGameStartTime );
	CNetworkVar( int, m_gamePhase );
	CNetworkVar( int, m_totalRoundsPlayed );
	CNetworkVar( int, m_nOvertimePlaying );
	CNetworkVar( int, m_iHostagesRemaining );
	CNetworkVar( bool, m_bAnyHostageReached );
	CNetworkVar( bool, m_bMapHasBombTarget );
	CNetworkVar( bool, m_bMapHasRescueZone );
	CNetworkVar( bool, m_bLogoMap );		 // If there's an info_player_logo entity, then it's a logo map.
	CNetworkVar( int, m_iNumGunGameProgressiveWeaponsCT );	// total number of CT gun game progressive weapons
	CNetworkVar( int, m_iNumGunGameProgressiveWeaponsT );	// total number of T gun game progressive weapons
	CNetworkVar( int,  m_iSpectatorSlotCount );				// max spectator slots available
	CNetworkArray( int, m_GGProgressiveWeaponOrderCT, 60 );	// CT gun game weapon order and # kills per weapon. Size is meant to be larger than the current number of different weapons defined in the CSWeaponID enum
	CNetworkArray( int, m_GGProgressiveWeaponOrderT, 60 );	// T gun game weapon order and # kills per weapon. Size is meant to be larger than the current number of different weapons defined in the CSWeaponID enum
	CNetworkArray( int, m_GGProgressiveWeaponKillUpgradeOrderCT, 60 );	// CT gun game number of kills per weapon. Size is meant to be larger than the current number of different weapons defined in the CSWeaponID enum
	CNetworkArray( int, m_GGProgressiveWeaponKillUpgradeOrderT, 60 );	// T gun game number of kills per weapon. Size is meant to be larger than the current number of different weapons defined in the CSWeaponID enum
	
	CNetworkVar( int, m_iMapFactionCT );
	CNetworkVar( int, m_iMapFactionT );

public:
	CNetworkVar( bool, m_bBombDropped );
	CNetworkVar( bool, m_bBombPlanted );
	CNetworkVar( int, m_iRoundWinStatus );
	CNetworkVar( bool, m_bTCantBuy );			// Who can and can't buy.
	CNetworkVar( bool, m_bCTCantBuy );

	CNetworkVar( int, m_iNumConsecutiveCTLoses );			//SupraFiend: the number of rounds the CTs have lost in a row.
 	CNetworkVar( int, m_iNumConsecutiveTerroristLoses );	//SupraFiend: the number of rounds the Terrorists have lost in a row.

	int GetNumHostagesRemaining( void ) { return m_iHostagesRemaining; }

	virtual CBaseCombatWeapon *GetNextBestWeapon( CBaseCombatCharacter *pPlayer, CBaseCombatWeapon *pCurrentWeapon );

	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"d7NSuLq2"; } // both the client and server need this key

	static int GetMinPlayers(); // always available

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.
	CCSGameRules();

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.

	CCSGameRules();
	virtual ~CCSGameRules();

	void DumpTimers( void ) const;	// debugging to help track down a stuck server (rare?)

	CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer );

	static void EndRound();

	virtual void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual void Think();

	void SwitchTeamsAtRoundReset( void );

	void ClearGunGameData( void );

	void FreezePlayers( void );
	void UnfreezeAllPlayers( void );

	// Called at the end of GameFrame (i.e. after all game logic has run this frame)
	virtual void EndGameFrame( void );

	// Called when game rules are destroyed by CWorld
	virtual void LevelShutdown( void );

	void UpdateTeamClanNames( int nTeam );

	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
			void ShowSpawnPoints();

	virtual void ClientCommandKeyValues( edict_t *pEntity, KeyValues *pKeyValues );

	// [menglish] Set up anything for all players that changes based on new players spawning mid-game
	//				Find and return fun fact data
	// [pfreese] Tracking of "pistol" round
	virtual void SpawningLatePlayer(CCSPlayer* pLatePlayer);

	bool IsPistolRound();

	void HostageKilled() { m_hostageWasKilled = true; }
	void HostageInjured() { m_hostageWasInjured = true; }

	bool WasHostageKilled() { return m_hostageWasKilled; }
	bool WasHostageInjured() { return m_hostageWasInjured; }

    // [tj] So game rules can react to damage taken
    void PlayerTookDamage(CCSPlayer* player, const CTakeDamageInfo &damageInfo);


	virtual bool PlayTextureSounds( void ) { return true; }
	// Let the game rules specify if fall death should fade screen to black
	virtual bool  FlPlayerFallDeathDoesScreenFade( CBasePlayer *pl ) { return FALSE; }

	virtual void  RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore );
	void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, bool bIgnoreWorld );

	virtual void UpdateClientData( CBasePlayer *pl );

	virtual CCSPlayer* CheckAndAwardAssists( CCSPlayer* pCSVictim, CCSPlayer* pKiller );

	// Death notices
	virtual void		DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );

	virtual void			InitDefaultAIRelationships( void );

	virtual const char *GetGameDescription( void ) { return "Counter-Strike: Source Offensive"; }  // this is the game name that gets seen in the server browser
	virtual const char *AIClassText(int classType);

	virtual bool FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );

	virtual const char *SetDefaultPlayerTeam( CBasePlayer *pPlayer );

	// Called before entities are created
	virtual void LevelInitPreEntity();

	// Called after the map has finished loading.
	virtual void LevelInitPostEntity();

	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );

	virtual void ClientDisconnected( edict_t *pClient );

	// Recreate all the map entities from the map data (preserving their indices),
	// then remove everything else except the players.
	// Also get rid of all world decals.
	void CleanUpMap();

	void CheckFreezePeriodExpired();
	void CheckRoundTimeExpired();

	// check if the scenario has been won/lost
	// return true if the scenario is over, false if the scenario is still in progress
	bool CheckWinConditions( void );

	void TerminateRound( float tmDelay, int reason );

	// [tj] A place to check achievements that occur at the end of the round
	void ProcessEndOfRoundAchievements(int iWinnerTeam, int iReason);

	// The following round-related functions are called as follows:
	//
	// At Match Start:
	//		PreRestartRound() -> RestartRound() -> PostRestartRound()
	//
	// During Subsequent Round Gameplay:
	//		RoundWin() is called at the point when the winner of the round has been determined - prior to free-play commencing
	//		PreRestartRound() is called with 1 second remaining prior to the round officially ending (This is after a round
	//						  winner has been chosen and players are allowed to continue playing)
	//		RoundEnd() is then called when the round has completely ended
	//		RestartRound() is then called immediately after RoundEnd()
	//		PostRestartRound() is called immediately after RestartRound() has completed
	void PreRestartRound( void );
	void RestartRound( void );
	void PostRestartRound( void );
	void RoundWin( void );
	int	 GetRoundsPlayed( void ) const { return m_match.GetRoundsPlayed(); }

	void BalanceTeams( void );
	void HandleScrambleTeams( void );
	void HandleSwapTeams( void );
	void MoveHumansToHumanTeam( void );
	bool TeamFull( int team_id );
	int	 MaxNumPlayersOnTerrTeam();
	int  MaxNumPlayersOnCTTeam();

	bool WillTeamHaveRoomForPlayer( CCSPlayer* pPlayer, int newTeam );

	bool TeamStacked( int newTeam_id, int curTeam_id  );
	bool FPlayerCanRespawn( CBasePlayer *pPlayer );
	void CheckMapConditions();
	void MarkLivingPlayersOnTeamAsNotReceivingMoneyNextRound(int team);

	// Check various conditions to end the map.
	bool CheckGameOver();
	bool CheckFragLimit();

	void CheckLevelInitialized();
	void CheckRestartRound();

	virtual bool FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker, const CTakeDamageInfo &info );
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );

	bool CanPlayerHearTalker( CBasePlayer* pListener, CBasePlayer *pSpeaker, bool bTeamOnly );
	virtual bool PlayerCanHearChat( CBasePlayer *pListener, CBasePlayer *pSpeaker, bool bTeamOnly );

	// repsawning
	void	CheckRespawnWaves( void );

	// Checks if it still needs players to start a round, or if it has enough players to start rounds.
	// Starts a round and returns true if there are enough players.
	bool NeededPlayersCheck( bool &bNeededPlayers );

	// Setup counts for m_iNumTerrorist, m_iNumCT, m_iNumSpawnableTerrorist, m_iNumSpawnableCT, etc.
	void InitializePlayerCounts(
		int &NumAliveTerrorist,
		int &NumAliveCT,
		int &NumDeadTerrorist,
		int &NumDeadCT
		);

	// Check to see if the round is over for the various game types. Terminates the round
	// and returns true if the round should end.
	bool BombRoundEndCheck( bool bNeededPlayers );
	bool HostageRescueRoundEndCheck( bool bNeededPlayers );

	CCSPlayer* CalculateEndOfRoundMVP();

	// Check to see if the teams exterminated each other. Ends the round and returns true if so.
	bool TeamExterminationCheck(
		int NumAliveTerrorist,
		int NumAliveCT,
		int NumDeadTerrorist,
		int NumDeadCT,
		bool bNeededPlayers
		);

	void ReadMultiplayCvars();
	void SwapAllPlayers();

	void OnTeamsSwappedAtRoundReset();

	void BroadcastSound( const char *sound, int team = -1 );


	// GUN GAME PROGRESSIVE FUNCTION
	bool GunGameProgressiveEndCheck( void );

	// BOMB MAP FUNCTIONS
	void GiveC4ToRandomPlayer();
	void GiveDefuserToRandomPlayer();
	bool IsThereABomber();
	bool IsThereABomb();

	// HOSTAGE MAP FUNCTIONS
	void HostageTouched();

	// Contribution score helpers
	void ScorePlayerKill( CCSPlayer* pPlayer );
	void ScorePlayerAssist( CCSPlayer* pPlayer, CCSPlayer* pCSVictim );
	void ScorePlayerObjectiveKill( CCSPlayer* pPlayer );
	void ScorePlayerTeamKill( CCSPlayer* pPlayer );
	void ScorePlayerSuicide( CCSPlayer* pPlayer );
	void ScoreBombPlant( CCSPlayer* pPlayer );
	void ScoreBombExploded( CCSPlayer* pPlayer );
	void ScoreBombDefuse( CCSPlayer* pPlayer, bool bMajorEvent );
	void ScoreHostageRescue( CCSPlayer* pPlayer, CHostage* pHostage, bool bMajorEvent );
	void ScoreHostageKilled( CCSPlayer* pPlayer );

	// Sets up g_pPlayerResource.
	virtual void CreateStandardEntities();
	virtual const char *GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );
	virtual const char *GetChatLocation( bool bTeamOnly, CBasePlayer *pPlayer );
	virtual const char *GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer );
	void ClientSettingsChanged( CBasePlayer *pPlayer );

	bool IsCareer( void ) const		{ return false; }		// returns true if this is a CZ "career" game

	virtual bool FAllowNPCs( void );

	CCSMatch* GetMatch( void );

	struct GrenadeRecording_t
	{
		Vector vecSrc;
		QAngle vecAngles;
		Vector vecVel;
		AngularImpulse angImpulse;
		CBaseCombatCharacter *pPlayer;
		CSWeaponID weaponID;
		bool bIsValid;
	};

#ifndef CLIENT_DLL
	void RecordGrenadeThrow( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBaseCombatCharacter *pPlayer, CSWeaponID weaponID );
	void RethrowLastGrenade();
	GrenadeRecording_t m_pLastGrenade;
#endif

protected:
	virtual void GoToIntermission( void );

public:
	// Let's the match store recplicated vars in the game rules.
	void SetGamePhase( GamePhase newPhase ) { m_gamePhase = newPhase; }
	void SetTotalRoundsPlayed( int roundsPlayed ) { m_totalRoundsPlayed = roundsPlayed; }
	void SetOvertimePlaying( int nOvertimePlaying ) { m_nOvertimePlaying = nOvertimePlaying; }

	void SetScrambleTeamsOnRestart( bool scramble ) { m_bScrambleTeamsOnRestart = scramble; }
	bool GetScrambleTeamsOnRestart( void ) { return m_bScrambleTeamsOnRestart; }

	void SetSwapTeamsOnRestart( bool swapTeams ) { m_bSwapTeamsOnRestart = swapTeams; }
	bool GetSwapTeamsOnRestart( void ) { return m_bSwapTeamsOnRestart; }

	bool IsFriendlyFireOn();

	bool	IsLastRoundBeforeHalfTime( void );

	virtual void	SetAllowWeaponSwitch( bool allow );
	virtual bool	GetAllowWeaponSwitch( void );

	bool			IsClanTeam( CTeam *pTeam );

	// VARIABLES FOR ALL TYPES OF MAPS
	bool m_bLevelInitialized;
	//int m_iRoundWinStatus;		// 1 == CT's won last round, 2 == Terrorists did, 3 == Draw, no winner
	int m_iTotalRoundsPlayed;
	int m_iUnBalancedRounds;	// keeps track of the # of consecutive rounds that have gone by where one team outnumbers the other team by more than 2

	// GAME TIMES
	int m_iFreezeTime;		// (From mp_freezetime) - How many seconds long the intro round (when players are frozen) is.

	int m_iNumTerrorist;		// The number of terrorists on the team (this is generated at the end of a round)
	int m_iNumCT;				// The number of CTs on the team (this is generated at the end of a round)
	int m_iNumSpawnableTerrorist;
	int m_iNumSpawnableCT;
	CUtlVector< int > m_arrSelectedHostageSpawnIndices; // The indices of hostage spawn locations selected for the match

	bool m_bFirstConnected;
	bool m_bCompleteReset;		// Set to TRUE to have the scores reset next time round restarts
	bool m_bScrambleTeamsOnRestart;
	bool m_bSwapTeamsOnRestart;

	class ICalculateEndOfRoundMVPHook_t
	{
	public:
		virtual CCSPlayer* CalculateEndOfRoundMVP() = 0;
	};
	ICalculateEndOfRoundMVPHook_t *m_pfnCalculateEndOfRoundMVPHook;

	int m_iSpawnPointCount_Terrorist;		// Number of Terrorist spawn points
	int m_iSpawnPointCount_CT;				// Number of CT spawn points

	int m_iMaxNumTerrorists;
	int m_iMaxNumCTs;

	bool m_bMapHasBuyZone;

	int m_iLoserBonus;			// SupraFiend: the amount of money the losing team gets. This scales up as they lose more rounds in a row
	float m_tmNextPeriodicThink;

	bool m_bVoiceWonMatchBragFired;

	float m_fWarmupNextChatNoticeTime;


	// HOSTAGE RESCUE VARIABLES
	int		m_iHostagesRescued;
	int		m_iHostagesTouched;
	float	m_flNextHostageAnnouncement;

    // [tj] Accessor for weapons donation ability
    bool GetCanDonateWeapon() { return m_bCanDonateWeapons; }

    // [tj] flawless and lossless round related flags
    bool m_bNoTerroristsKilled;
    bool m_bNoCTsKilled;
    bool m_bNoTerroristsDamaged;
    bool m_bNoCTsDamaged;

    // [tj] Find out if dropped weapons count as donations
    bool m_bCanDonateWeapons;

	// [tj] Keep track of first kill
	CHandle<CCSPlayer> m_pFirstKill;
	float m_firstKillTime;

	// [menglish] Keep track of first blood
	CHandle<CCSPlayer> m_pFirstBlood;
	float m_firstBloodTime;


    // [dwenger] Rescue-related achievement values
    CHandle<CCSPlayer> m_pLastRescuer;
    int     m_iNumRescuers;

	bool m_hostageWasInjured;
	bool m_hostageWasKilled;

	// [menglish] Fun Fact Manager
	CCSFunFactMgr *m_pFunFactManager;

	// [tj] To avoid rewriting the same piece of code, we can get all the information
	//		we want from one call that fills in an array of structures.
	struct TeamPlayerCounts
	{
		int totalPlayers;
		int totalAlivePlayers;
		int totalDeadPlayers; //sum of killedPlayers + suicidedPlayers + unenteredPlayers
		int killedPlayers;
		int suicidedPlayers;
		int unenteredPlayers;
	};

	void GetPlayerCounts(TeamPlayerCounts teamCounts[TEAM_MAXCOUNT]);

	int m_nLastFreezeEndBeep;


	// BOMB MAP VARIABLES
	bool	m_bTargetBombed;	// whether or not the bomb has been bombed
	bool	m_bBombDefused;	// whether or not the bomb has been defused
	bool	m_bMapHasBombZone;
//	bool	m_bBombDropped; -- moved to network cvars
//	bool	m_bBombPlanted; -- moved to network cvars
	EHANDLE m_pLastBombGuy;

	int		TeamCashAwardValue( int reason );
	int		PlayerCashAwardValue( int reason );
	void	AddTeamAccount( int team, int reason );
	void	AddTeamAccount( int team, int reason, int amount, const char* szAwardText = NULL );

	int GetNumWinsToClinch( void ) const;
	bool IsLastRoundOfMatch() const;
	bool IsMatchPoint() const;

	// AreTeamsPlayingSwitchedSides() -- will return true when match is in second half, or in the half of overtime period where teams are switched.
	// Overtime logic is as follows: TeamA plays CTs as first half of regulation, then Ts as second half of regulation,
	//				then if tied in regulation continues to play Ts as first half of 1st overtime, then switches to CTs for second half of 1st overtime,
	//				then if still tied after 1st OT they continue to play CTs as first half of 2nd overtime, then switch to Ts for second half of 2nd overtime,
	//				then if still tied after 2nd OT they continue to play Ts as first half of 3rd overtime, then switch to CTs for second half of 3rd overtime,
	//				and so on until the match determines a winner.
	// So AreTeamsPlayingSwitchedSides will return true when TeamA is playing T-side and will return false when TeamA plays CT-side as they started match on CT
	// in scenario outlined above.
	bool AreTeamsPlayingSwitchedSides() const;

	void SetIsWarmupPeriod( bool bIsWarmup ) { m_bWarmupPeriod = bIsWarmup; }

	int GetWeaponScoreForDeathmatch( CSWeaponID nWeapID );

public:
	CBaseEntity* GetNextSpawnpoint( int teamNumber );

	void DoCoopSpawnAndNavInit( void );
	void AddSpawnPointToMasterList( SpawnPoint* pSpawnPoint );
	void GenerateSpawnPointListsFirstTime( void );
	void RefreshCurrentSpawnPointLists( void );

	CCSMatch		m_match;

	void ShuffleSpawnPointLists( void );
	void ShuffleMasterSpawnPointLists( void );
	void SortSpawnPointLists( void );
	void SortMasterSpawnPointLists( void );
	void ShufflePlayerList( CUtlVector< CCSPlayer* > &playersList );

protected:
	CUtlVector< SpawnPoint* >	m_CTSpawnPointsMasterList;			// The master list of CT spawn points (contains all points whether enabled or disabled)
	CUtlVector< SpawnPoint* >	m_TerroristSpawnPointsMasterList;	// The master list of Terrorist spawn points (contains all points whether enabled or disabled)

	int m_iNextCTSpawnPoint;						// Used when picking the next CT spawn point to assign
	int m_iNextTerroristSpawnPoint;					// Used when picking the next Terrorist spawn point to assign
	CUtlVector< SpawnPoint* >	m_CTSpawnPoints;		// List of CT spawn points sorted by their priorities
	CUtlVector< SpawnPoint* >	m_TerroristSpawnPoints;	// List of Terrorist spawn points sorted by their priorities

private:

	// Don't allow switching weapons while gaining new technologies
	bool			m_bAllowWeaponSwitch;

	bool			m_bRoundTimeWarningTriggered;

	float			m_phaseChangeAnnouncementTime;
	float			m_fNextUpdateTeamClanNamesTime;

	float			m_flLastThinkTime;

#endif


#ifdef GAME_DLL
public:
	virtual void	GetTaggedConVarList( KeyValues *pCvarTagList );
#endif

public:
	bool IsSwitchingTeamsAtRoundReset( void ) { return m_bSwitchingTeamsAtRoundReset; }

	float CheckTotalSmokedLength( float flRadius, Vector vecGrenadePos, Vector from, Vector to );

protected:
	void InitializeGameTypeAndMode( void );

	bool m_bHasTriggeredRoundStartMusic;

private:
	bool m_bSwitchingTeamsAtRoundReset;
	int m_iMaxGunGameProgressiveWeaponIndex;
};

//-----------------------------------------------------------------------------
// Gets us at the team fortress game rules
//-----------------------------------------------------------------------------

inline CCSGameRules* CSGameRules()
{
	return static_cast<CCSGameRules*>(g_pGameRules);
}

#define IGNORE_SPECTATORS false
#define IGNORE_UNASSIGNED true
int UTIL_HumansInGame( bool ignoreSpectators = false, bool ignoreUnassigned = false );
int UTIL_SpectatorsInGame( void );


//-----------------------------------------------------------------------------
// Music Selection
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
void CheckMusicDuration();
void PlayMusicSelection( IRecipientFilter& filter, CsMusicType_t nMusicType, int nPlayerEntIndex = 0 );
#endif


//-----------------------------------------------------------------------------
// Purpose: Useful utility functions
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL

#else

	class CTFTeam;
	CTFTeam *GetOpposingTeam( CTeam *pTeam );
	bool EntityPlacementTest( CBaseEntity *pMainEnt, const Vector &vOrigin, Vector &outPos, bool bDropToGround );

#endif

#endif // TF_GAMERULES_H