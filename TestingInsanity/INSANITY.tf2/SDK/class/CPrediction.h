#pragma once
#include "../../Utility/Interface Handler/Interface.h"

struct vec;
struct qangle;
class BaseEntity;
class CUserCmd;

class CPrediction
{
public:
	virtual			~CPrediction(void) {};

	virtual void	Init(void) = 0;
	virtual void	Shutdown(void) = 0;

	// Run prediction
	virtual void	Update
					(
						int startframe,				// World update ( un-modded ) most recently received
						bool validframe,			// Is frame data valid
						int incoming_acknowledged,	// Last command acknowledged to have been run by server (un-modded)
						int outgoing_command		// Last command (most recent) sent to server (un-modded)
					) = 0;

	// We are about to get a network update from the server.  We know the update #, so we can pull any
	//  data purely predicted on the client side and transfer it to the new from data state.
	virtual void	PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet) = 0;
	virtual void	PostEntityPacketReceived(void) = 0;
	virtual void	PostNetworkDataReceived(int commands_acknowledged) = 0;

	virtual void	OnReceivedUncompressedPacket(void) = 0;

	// The engine needs to be able to access a few predicted values
	virtual void	GetViewOrigin(vec& org) = 0;
	virtual void	SetViewOrigin(vec& org) = 0;
	virtual void	GetViewAngles(qangle& ang) = 0;
	virtual void	SetViewAngles(qangle& ang) = 0;
	virtual void	GetLocalViewAngles(qangle& ang) = 0;
	virtual void	SetLocalViewAngles(qangle& ang) = 0;

	// CPrediction Fns
	virtual bool	InPrediction(void) = 0;
	virtual bool	IsFirstTimePredicted(void) = 0;
	virtual int		GetIncomingPacketNumber(void) = 0;

	virtual void	RunCommand(BaseEntity* player, CUserCmd* ucmd, void* moveHelper) = 0;


	int				m_hLastGround;
	bool			m_bInPrediction;
	bool			m_bFirstTimePredicted;
	bool			m_bOldCLPredictValue;
	bool			m_bEnginePaused;

	int				m_nPreviousStartFrame;

	int				m_nCommandsPredicted;
	int				m_nServerCommandsAcknowledged;
	int				m_bPreviousAckHadErrors;
	int				m_nIncomingPacketNumber;

	float			m_flIdealPitch;
};

MAKE_INTERFACE_VERSION(cPrediction, "VClientPrediction001", CPrediction, CLIENT_DLL);