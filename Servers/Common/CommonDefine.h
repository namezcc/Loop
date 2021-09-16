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
	SOP_ROLE_SELET = 1,
	SOP_CREATE_ROLE = 2,
	SOP_LOAD_PLAYER_DATA = 3,
	SOP_UPDATE_PLAYER_DATA = 4,
	SOP_DELETE_PLAYER_DATA = 5,
	SOP_SEARCH_PLAYER = 6,
	SOP_SAVE_PlAYER_TO_DB = 7,





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

union Int64Struct
{
	Int64Struct() :i64(0)
	{}

	Int64Struct(int64_t n) :i64(n)
	{}

	Int64Struct(int32_t h, int32_t l) : height32(h), lower32(l)
	{
	}

	int64_t i64;
	struct
	{
		int32_t lower32;
		int32_t height32;
	};
};

#endif
