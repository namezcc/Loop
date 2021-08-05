#ifndef COMMON_DEFINE_H
#define COMMON_DEFINE_H

#define ONE_SCENE_NUM	30
#define MATCH_BATTLE_NUM	4

#include <string>

struct ProxyNode
{
	int16_t proxyId;
	int16_t serid;
};

enum ServerBusyState
{
	SBS_NORMAL,
	SBS_BUSY,
	SBS_CLOSE,
};

struct ServerInfoState
{
	int32_t type;
	int32_t server_id;
	std::string ip;
	int32_t port;
	int32_t state;
};

enum SQL_OPRATION
{
	SOP_NONE = 0,
	SOP_ROLE_SELET,
	SOP_CREATE_ROLE,
	SOP_LOAD_PLAYER_DATA,
	SOP_UPDATE_PLAYER_DATA,
	SOP_DELETE_PLAYER_DATA,




};

union Int32Struct
{
	Int32Struct() :i32(0)
	{}

	Int32Struct(int32_t n) :i32(n)
	{}

	int32_t i32;
	struct
	{
		int32_t lower16 : 16;
		int32_t height16 : 16;
	};
	struct
	{
		int8_t bit8[4];
	};
};

#endif
