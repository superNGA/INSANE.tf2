#pragma once

struct global_var_base
{
	float			realtime;
	int				framecount;
	float			absoluteframetime;
	float			curtime; // <- this is the good stuff
	float			frametime;
	int				maxClients;
	int				tickcount;
	float			interval_per_tick;
	float			interpolation_amount;
	int				simTicksThisFrame;
	int				network_protocol;
	void*			pSaveData;
	bool			m_bClient;
	int				nTimestampNetworkingBase;
	int				nTimestampRandomizeWindow;
};

class I_engine_client_replay
{
private:
	virtual void* DUMMY_FUNCTION_00() = 0;
	virtual void* DUMMY_FUNCTION_01() = 0;
	virtual void* DUMMY_FUNCTION_02() = 0;
	virtual void* DUMMY_FUNCTION_03() = 0;
	virtual void* DUMMY_FUNCTION_04() = 0;
	virtual void* DUMMY_FUNCTION_05() = 0;
	virtual void* DUMMY_FUNCTION_06() = 0;
	virtual void* DUMMY_FUNCTION_07() = 0;
	virtual void* DUMMY_FUNCTION_08() = 0;
	virtual void* DUMMY_FUNCTION_09() = 0;
	virtual void* DUMMY_FUNCTION_10() = 0;
	virtual void* DUMMY_FUNCTION_11() = 0;
	virtual void* DUMMY_FUNCTION_12() = 0;

public:
	virtual global_var_base* GetClientGlobalVars() = 0;
};