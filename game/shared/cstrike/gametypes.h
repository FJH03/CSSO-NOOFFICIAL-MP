//========= Copyright (c) 1996-2011, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game types and modes
//
// $NoKeywords: $
//=============================================================================//

#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#ifdef _WIN32
#pragma once
#endif

#include "gametypes/igametypes.h"
#include "tier1/KeyValues.h"
#include "const.h"


#define GAMETYPES_MAX_NAME	64
#define GAMETYPES_MAX_ID	64
#define GAMETYPES_MAX_MAP_NAME	32

#define GAMETYPES_MAX_MAP_GROUP_NAME 32

// enum for "game_type" convar.
// Note: these values match the order of the game types in GameModes.txt
enum CS_GameType
{
	CS_GameType_Min = 0,
	CS_GameType_Classic = CS_GameType_Min,		// Casual and competitive (see gamerules_*.cfg)
	CS_GameType_GunGame,						// Arms race
	//CS_GameType_Training,						// training map - just one map in this mode
	CS_GameType_Custom,							// capture the flag and such, set by map; not supported by matchmaking, supported through workshop, there are very few maps that use this game mode
												// PiMoN TODO: is it needed? probably only after VScripts are added
	//CS_GameType_Cooperative,					// players vs bots - matchmade, operation missions
	CS_GameType_Max = CS_GameType_Custom		// PiMoN: change this to CS_GameType_Cooperative if I ever uncomment it
};

// enums for "game_mode" convar.
// Note: these values match the order of the game modes by type in GameModes.txt
// Note: if you add or delete a mode that you want to track ELO rankings for, 
//       you need to update inc_playerrankings_usr.inc to reflect the total
//       number of modes that are ELO ranked
namespace CS_GameMode
{
	enum Classic
	{
		Classic_Min = 0,
		Classic_Casual = Classic_Min,
		Classic_Competitive,
		Classic_Competitive_2v2,
		Classic_Max = Classic_Competitive_2v2
	};

	enum GunGame
	{
		GunGame_Min = 0,
		GunGame_Progressive = GunGame_Min,
		GunGame_Bomb,
		GunGame_Deathmatch,
		GunGame_Max = GunGame_Deathmatch
	};

	/*enum Training
	{
		Training_Min = 0,
		Training_Training = Training_Min,
		Training_Max = Training_Training
	};*/

	enum Custom
	{
		Custom_Min = 0,
		Custom_Custom = Custom_Min,
		Custom_Max = Custom_Custom
	};

	/*enum Cooperative
	{
		Cooperative_Min = 0,
		Cooperative_Guardian = Cooperative_Min,
		Cooperative_Mission,
		Cooperative_Max = Cooperative_Mission
	};*/
};

class GameTypes : public IGameTypes
{
private:
	struct GameType;
	struct GameMode;
	struct Map;
	struct MapGroup;
	struct CustomBotDifficulty;

public:
	GameTypes();
	virtual ~GameTypes();

	// Initialization
	virtual bool Initialize( bool force = false );
	virtual bool IsInitialized( void ) const { return m_Initialized; };

	//
	// Game Types and Modes
	//

	// Set the game type and mode convars from the given strings.
	virtual bool SetGameTypeAndMode( const char *gameType, const char *gameMode );
	virtual bool GetGameTypeAndModeFromAlias( const char *modeAlias, int& iOutGameType, int& iOutGameMode );
	virtual bool SetGameTypeAndMode( int nGameType, int nGameMode );
	virtual void SetAndParseExtendedServerInfo( KeyValues *pExtendedServerInfo );
	virtual void CheckShouldSetDefaultGameModeAndType( const char* szMapNameFull );

	// Get the indexes for the current game type and mode.
	virtual int GetCurrentGameType() const;
	virtual int GetCurrentGameMode() const;

	virtual const char *GetCurrentMapName() { return m_pServerMap ? m_pServerMap->m_Name : ""; }

	// Get the current game type and mode UI strings.
	virtual const char *GetCurrentGameTypeNameID( void );
	virtual const char *GetCurrentGameModeNameID( void );
	virtual const char *GetCurrentGameModeDescID( void );
	
	virtual const char *GetGameTypeNameID( int nGameType );
	virtual const char *GetGameModeNameID( int nGameType, int nGameMode );
	virtual int GetGameTypesCount( void );
	virtual int GetGameModesCount( int nGameType );

	// Apply the game mode convars for the current game type and mode.
	virtual bool ApplyConvarsForCurrentMode( bool isMultiplayer );

	// Output the values of the convars for the current game mode.
	virtual void DisplayConvarsForCurrentMode( void );

	// Returns the weapon progression for the current game type and mode.
	virtual const CUtlVector< IGameTypes::WeaponProgression > *GetWeaponProgressionForCurrentModeCT( void );
	virtual const CUtlVector< IGameTypes::WeaponProgression > *GetWeaponProgressionForCurrentModeT( void );

	virtual const char *GetGameTypeFromInt( int gameType );
	virtual const char *GetGameModeFromInt( int gameType, int gameMode );

	virtual bool GetGameModeAndTypeIntsFromStrings( const char* szGameType, const char* szGameMode, int& iOutGameType, int& iOutGameMode );
	virtual bool GetGameModeAndTypeNameIdsFromStrings( const char* szGameType, const char* szGameMode, const char*& szOutGameTypeNameId, const char*& szOutGameModeNameId );
	virtual bool GetGameTypeFromMode( const char *szGameMode, const char *&pszGameTypeOut );

	//
	// Maps
	//

	virtual const char *GetRandomMapGroup( const char *gameType, const char *gameMode );
	virtual const char *GetFirstMap( const char *mapGroup );
	virtual const char *GetRandomMap();
	virtual const char *GetRandomMap( const char *mapGroup );
	virtual const char *GetNextMap( const char *mapName );
	virtual const char *GetNextMap( const char *mapGroup, const char *mapName );

	virtual int GetMaxPlayersForTypeAndMode( int iType, int iMode );

	virtual int GetCurrentServerNumSlots( void );
	virtual int GetCurrentServerSettingInt( const char *szSetting, int iDefaultValue = 0 );

	virtual bool IsValidMapGroupName( const char * mapGroup );
	virtual bool IsValidMapInMapGroup( const char * mapGroup, const char *mapName );
	virtual bool IsValidMapGroupForTypeAndMode( const char * mapGroup, const char *gameType, const char *gameMode );
	virtual bool IsValidMapForTypeAndMode( const char *mapName, const char *gameType, const char *gameMode );

	// Apply the convars for the given map.
	virtual bool ApplyConvarsForMap( const char *mapName, bool isMultiplayer );

	// Get specifics about a map.
	virtual bool GetMapInfo( const char *mapName );
	virtual const char* GetMapNameID( const char* mapName );

	virtual const int GetDefaultGameTypeForMap( const char *mapName );
	virtual const int GetDefaultGameModeForMap( const char *mapName );

	// Map group properties
	virtual const CUtlStringList *GetMapGroupMapList( const char *mapGroup );

	//
	// Custom Bot Difficulty
	//

	// Sets the bot difficulty for Offline games.
	virtual bool SetCustomBotDifficulty( int botDiff );

	// Returns the bot difficulty for Offline games.
	virtual int GetCustomBotDifficulty( void );

	// Returns the number of bot difficulties for Offline games.
	virtual int GetCustomBotDifficultyCount( void );
	virtual const char *GetCustomBotDifficultyNameID( int botDiff );


	virtual bool LoadMapEntry( KeyValues *pKV );

private:

	void InitMapSidecars( KeyValues *pKV );
	void AddMapKVs( KeyValues* pKVMaps, const char* curMap );
	bool LoadGameTypes( KeyValues *pKV );
	bool LoadMaps( KeyValues *pKV );
	bool LoadMapGroups( KeyValues *pKV );
	bool LoadCustomBotDifficulties( KeyValues *pKV );
	void LoadWeaponProgression( KeyValues * pKV_WeaponProgression, CUtlVector< WeaponProgression > & vecWeaponProgression, const char * szGameType, const char * szGameMode );

	GameType *GetGameType_Internal( const char *gameType );
	GameMode *GetGameMode_Internal( GameType *pGameType, const char *gameMode );
	MapGroup *GetMapGroup_Internal( const char *mapGroup );

	GameType *GetCurrentGameType_Internal( void );
	GameMode *GetCurrentGameMode_Internal( GameType *pGameType );

	Map *GetMap_Internal( const char *mapName );

	CustomBotDifficulty *GetCurrentCustomBotDifficulty_Internal( void );

	bool GetGameModeAndTypeFromStrings( const char* szGameType, const char* szGameMode, GameType*& outGameType, GameMode*& outGameMode );

private:

	CUniformRandomStream m_randomStream;

	struct GameType
	{
		GameType();
		~GameType();

		int m_Index; // index of the game type in the file
		char m_Name[GAMETYPES_MAX_NAME]; // internal name for the game type
		char m_NameID[GAMETYPES_MAX_ID]; // UI identifier for the game type name
		CUtlVector< GameMode* > m_GameModes; // list of associated game modes
	};

	struct GameMode
	{
		GameMode();
		~GameMode();

		int m_Index; // index of the game mode within the game type
		char m_Name[GAMETYPES_MAX_NAME]; // internal name of the game mode
		char m_NameID[GAMETYPES_MAX_ID]; // UI identifier for the game mode name
		char m_NameID_SP[GAMETYPES_MAX_ID]; // UI identifier for the game mode name
		char m_DescID[GAMETYPES_MAX_ID]; // UI identifier game mode description
		char m_DescID_SP[GAMETYPES_MAX_ID]; // UI identifier game mode description
		int m_MaxPlayers; // maximum number of players for the mode
		KeyValues* m_pExecConfings; // configs to exec for this game mode
		CUtlStringList m_MapGroupsSP; // single player map groups
		CUtlStringList m_MapGroupsMP; // multiplayer map groups
		CUtlVector< WeaponProgression > m_WeaponProgressionCT; // CT weapon progression for this game mode (only gun game)
		CUtlVector< WeaponProgression > m_WeaponProgressionT; // T weapon progression for this game mode (only gun game)
	};

	struct Map
	{
		Map();

		int m_Index; // index of the map
		char m_Name[MAX_MAP_NAME]; // internal name for the map (matches the filename without extension)
		char m_NameID[GAMETYPES_MAX_ID]; // UI identifier for the map name
		int m_nDefaultGameType;
		int m_nDefaultGameMode;
	};

	struct MapGroup
	{
		MapGroup();

		char m_Name[GAMETYPES_MAX_MAP_GROUP_NAME]; // internal name for the map group
		char m_NameID[GAMETYPES_MAX_ID]; // UI identifier for the map name
		CUtlStringList m_Maps;
	};

	struct CustomBotDifficulty
	{
		CustomBotDifficulty();
		~CustomBotDifficulty();

		int m_Index; // index of the difficulty level
		char m_Name[GAMETYPES_MAX_NAME]; // internal name
		char m_NameID[GAMETYPES_MAX_ID]; // UI identifier
		KeyValues* m_pConvars; // convars associated with the bot difficulty
		bool m_HasBotQuota; // true if this difficulty level has a bot quota in the convars
	};

	int FindWeaponProgressionIndex( CUtlVector< WeaponProgression > & vecWeaponProgression, const char * szWeaponName );
	void ClearServerMapGroupInfo( void );
	bool m_Initialized; // true if the game types interface has been initialized
	CUtlVector< GameType* > m_GameTypes; // list of game types
	CUtlVector< Map* > m_Maps; // list of maps
	CUtlVector< MapGroup* > m_MapGroups; // list of map groups for cycling maps
	CUtlVector< CustomBotDifficulty* > m_CustomBotDifficulties; // list of custom bot difficulty levels for Offline Games

	bool GetRunMapWithDefaultGametype() { return m_bRunMapWithDefaultGametype; }
	void SetRunMapWithDefaultGametype( bool bDefaultGametype ) { m_bRunMapWithDefaultGametype = bDefaultGametype; }

	// These are filled out on the client when connecting to a server
	KeyValues *m_pExtendedServerInfo;
	Map* m_pServerMap; // map on the currently connected server
	MapGroup* m_pServerMapGroup; // map group for cycling maps on the currently connected server
	int m_iCurrentServerNumSlots;

	// if this is true when the Level init happens, we'll run whatever game type and mode that the map defines in its KV file
	// if this is set to false, we know the map was executed via a method that sets the mode and type (like via the menu UI)
	bool m_bRunMapWithDefaultGametype;
};

#endif // GAME_TYPES_H
