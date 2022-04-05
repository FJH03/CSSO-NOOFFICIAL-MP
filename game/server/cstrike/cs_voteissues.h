//========= Copyright, Valve Corporation, All rights reserved. ============
//
// Purpose:  CS-specific things to vote on
//
//=============================================================================

#ifndef CS_VOTEISSUES_H
#define CS_VOTEISSUES_H

#ifdef _WIN32
#pragma once
#endif

#include "vote_controller.h"

class CCSPlayer;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CBaseCSIssue : public CBaseIssue
{
	// Overrides to BaseIssue standard to this mod.
public:
	CBaseCSIssue( const char *typeString, CVoteController *pVoteController ) : CBaseIssue( typeString, pVoteController )
	{
	}
	virtual const char *GetOtherTeamDisplayString() { return "#CStrike_otherteam_vote_unimplemented"; }
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CRestartGameIssue : public CBaseCSIssue
{
public:
	CRestartGameIssue( CVoteController* pVoteController ) : CBaseCSIssue( "RestartGame", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString();
	virtual void		ListIssueDetails( CBasePlayer *forWhom );
	virtual bool		IsTeamRestrictedVote( void ){ return false; }
	virtual const char *GetVotePassedString();
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CKickIssue : public CBaseCSIssue
{
public:
	CKickIssue( CVoteController* pVoteController ) : CBaseCSIssue( "Kick", pVoteController ), m_bPlayerCrashed( false )
	{
	}

	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual const char *GetVotePassedString( void );
	virtual bool		IsTeamRestrictedVote( void ) { return true; }
	virtual void		OnVoteStarted( void );
	virtual const char *GetDetailsString( void );
	virtual const char *GetOtherTeamDisplayString() { return "#CStrike_otherteam_vote_kick_player"; }

private:
	void				ExtractDataFromDetails( const char *pszDetails, CCSPlayer **pSubject );

	const char*			m_uniqueIDtoBan;
	bool				m_bPlayerCrashed;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CBanIssue : public CBaseCSIssue
{
public:
	CBanIssue( CVoteController* pVoteController ) : CBaseCSIssue( "Ban", pVoteController )
	{
	}

	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual const char *GetVotePassedString( void );
	virtual bool		IsTeamRestrictedVote( void ) { return true; }
	virtual void		OnVoteStarted( void );
	virtual const char *GetDetailsString( void );

private:
	void				ExtractDataFromDetails( const char *pszDetails, CCSPlayer **pSubject );

	const char*			m_uniqueIDtoBan;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CChangeLevelIssue : public CBaseCSIssue
{
public:
	CChangeLevelIssue( CVoteController* pVoteController ) : CBaseCSIssue( "ChangeLevel", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsTeamRestrictedVote( void ){ return false; }
	virtual bool		IsEnabled( void );
	virtual bool		CanTeamCallVote( int iTeam ) const;		// Can someone on the given team call this vote?
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual const char *GetVotePassedString( void );
	virtual const char *GetDetailsString( void );
	virtual bool		IsYesNoVote( void );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNextLevelIssue : public CBaseCSIssue
{
public:
	CNextLevelIssue( CVoteController* pVoteController ) : CBaseCSIssue( "NextLevel", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsTeamRestrictedVote( void ){ return false; }
	virtual bool		IsEnabled( void );
	virtual bool		CanTeamCallVote( int iTeam ) const;		// Can someone on the given team call this vote?
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual const char *GetVotePassedString( void );
	virtual const char *GetDetailsString( void );
	virtual bool		IsYesNoVote( void );
	virtual int			GetNumberVoteOptions( void );

private:
	CUtlVector <const char *> m_IssueOptions;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CScrambleTeams : public CBaseCSIssue
{
public:
	CScrambleTeams( CVoteController* pVoteController ) : CBaseCSIssue( "ScrambleTeams", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual bool		IsTeamRestrictedVote( void ){ return false; }
	virtual const char *GetVotePassedString( void );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSwapTeams : public CBaseCSIssue
{
public:
	CSwapTeams( CVoteController* pVoteController ) : CBaseCSIssue( "SwapTeams", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual bool		IsTeamRestrictedVote( void ){ return false; }
	virtual const char *GetVotePassedString( void );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPauseMatchIssue : public CBaseCSIssue
{
public:
	CPauseMatchIssue( CVoteController* pVoteController ) : CBaseCSIssue( "PauseMatch", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual bool		ShouldIgnoreCreationTimer( void ) { return true; }
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual int			GetVotesRequiredToPass( void ){ return 1; }
	virtual const char *GetVotePassedString( void );
	virtual float		GetCommandDelay( void ) { return 0.0; }
	virtual bool		IsEnabledDuringWarmup( void )	{ return true; } // Can this vote be called during warmup?
	virtual float		GetFailedVoteLockOutTime( void ) { return 1.0; }

};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CUnpauseMatchIssue : public CBaseCSIssue
{
public:
	CUnpauseMatchIssue( CVoteController* pVoteController ) : CBaseCSIssue( "UnpauseMatch", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual bool		ShouldIgnoreCreationTimer( void ) { return true; }
	virtual bool		IsUnanimousVoteToPass( void ) { return true; }	// Requires all potential voters to pass
	virtual bool		IsEnabledDuringWarmup( void )	{ return true; } // Can this vote be called during warmup?
	virtual const char *GetVotePassedString( void );
	virtual float		GetFailedVoteLockOutTime( void ) { return 1.0; }
};

class CStartTimeOutIssue : public CBaseCSIssue
{
public:
	CStartTimeOutIssue( CVoteController* pVoteController ) : CBaseCSIssue( "StartTimeOut", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual bool		CanTeamCallVote( int iTeam ) const;		// Can someone on the given team call this vote?
	virtual const char *GetDisplayString( void );
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual bool		IsTeamRestrictedVote( void ) { return true; }
	virtual const char *GetVotePassedString( void );
	virtual const char *GetOtherTeamDisplayString() { return "#CStrike_otherteam_vote_timeout"; }
	virtual vote_create_failed_t MakeVoteFailErrorCodeForClients( vote_create_failed_t eDefaultFailCode );
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSurrender : public CBaseCSIssue
{
public:
	CSurrender( CVoteController* pVoteController ) : CBaseCSIssue( "Surrender", pVoteController )
	{
	}
	virtual void		ExecuteCommand( void );
	virtual bool		IsEnabled( void );
	virtual bool		CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual bool		CanTeamCallVote( int iTeam ) const;
	virtual const char *GetDisplayString( void );
	virtual const char *GetOtherTeamDisplayString() { return "#CStrike_otherteam_vote_surrender"; }
	virtual void		ListIssueDetails( CBasePlayer *pForWhom );
	virtual bool		IsTeamRestrictedVote( void ){ return true; }
	virtual const char *GetVotePassedString( void );
	virtual bool		IsUnanimousVoteToPass( void ) {return true; }	// Requires all potential voters to pass
	virtual vote_create_failed_t MakeVoteFailErrorCodeForClients( vote_create_failed_t eDefaultFailCode )
	{
		switch ( eDefaultFailCode )
		{
		case VOTE_FAILED_WAITINGFORPLAYERS:
		case VOTE_FAILED_TEAM_CANT_CALL:
			return VOTE_FAILED_TOO_EARLY_SURRENDER;
		default:
			return eDefaultFailCode;
		}
	}
};

#endif // CS_VOTEISSUES_H