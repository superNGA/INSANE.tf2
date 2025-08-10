#pragma once

#include <cstdint>

#include "../../Utility/Interface Handler/Interface.h"


class CBaseClientState
{
public:
	// Connection to server.			
	int				m_Socket;		// network socket 
	void*		    m_NetChannel;		// Our sequenced channel to the remote server.
	unsigned int	m_nChallengeNr;	// connection challenge number
	double			m_flConnectTime;	// If gap of connect_time to net_time > 3000, then resend connect packet
	int				m_nRetryNumber;	// number of retry connection attemps
	char			m_szRetryAddress[260];
	char*			m_sRetrySourceTag; // string that describes why we decided to connect to this server (empty for command line, "serverbrowser", "quickplay", etc)
	int				m_retryChallenge; // challenge we sent to the server
	int				m_nSignonState;    // see SIGNONSTATE_* definitions
	double			m_flNextCmdTime; // When can we send the next command packet?
	int				m_nServerCount;	// server identification for prespawns, must match the svs.spawncount which
	// is incremented on server spawning.  This supercedes svs.spawn_issued, in that
	// we can now spend a fair amount of time sitting connected to the server
	// but downloading models, sounds, etc.  So much time that it is possible that the
	// server might change levels again and, if so, we need to know that.
	uint64_t			m_ulGameServerSteamID; // Steam ID of the game server we are trying to connect to, or are connected to.  Zero if unknown
	int				m_nCurrentSequence;	// this is the sequence number of the current incoming packet	

	char		m_ClockDriftMgr[76];

	int			m_nDeltaTick;		//	last valid received snapshot (server) tick
	bool		m_bPaused;			// send over by server
	float		m_flPausedExpireTime;
	int			m_nViewEntity;		// cl_entitites[cl.viewentity] == player point of view

	int			m_nPlayerSlot;		// own player entity index-1. skips world. Add 1 to get cl_entitites index;

	char		m_szLevelFileName[128];	// for display on solo scoreboard
	char		m_szLevelBaseName[128]; // removes maps/ and .bsp extension

	int			m_nMaxClients;		// max clients on server

	void*				m_pEntityBaselines[2][(1 << 11)];	// storing entity baselines

	// This stuff manages the receiving of data tables and instantiating of client versions
	// of server-side classes.
	void*			    m_pServerClasses;
	int					m_nServerClasses;
	int					m_nServerClassBits;
	char				m_szEncrytionKey[2048];
	unsigned int		m_iEncryptionKeySize;

	void* m_StringTableContainer;

	bool m_bRestrictServerCommands;	// If true, then the server is only allowed to execute commands marked with FCVAR_SERVER_CAN_EXECUTE on the client.
	bool m_bRestrictClientCommands;	// If true, then IVEngineClient::ClientCmd is only allowed to execute commands marked with FCVAR_CLIENTCMD_CAN_EXECUTE on the client.
};

class CClientFrameManager
{
public:
	void* m_Frames;		// updates can be delta'ed from here
	void* m_LastFrame;
	int	  m_nFrames;
	const char m_padding_03[48];
};

class CClientState : public CBaseClientState, public CClientFrameManager
{
public:
	const char  m_padding_04[0xD8];
	float		m_flLastServerTickTime;		// the timestamp of last message
	bool		insimulation;

	int			oldtickcount;		// previous tick
	float		m_tickRemainder;	// client copy of tick remainder
	float		m_frameTime;		// dt of the current frame

	int			lastoutgoingcommand;// Sequence number of last outgoing command
	int			chokedcommands;		// number of choked commands
	int			last_command_ack;	// last command sequence number acknowledged by server
	int			command_ack;		// current command sequence acknowledged by server
	int			m_nSoundSequence;	// current processed reliable sound sequence number
};


MAKE_INTERFACE_SIGNATURE(cClientState, "48 8D 0D ? ? ? ? E8 ? ? ? ? F3 0F 5E 05", CClientState, ENGINE_DLL, 3, 7)