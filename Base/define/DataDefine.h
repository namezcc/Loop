﻿#ifndef DATA_DEFINE_H
#define DATA_DEFINE_H
#include "FactorManager.h"
#include "LTime.h"
#include "BaseMsg.h"
#include <cstring>
#include <boost/coroutine2/all.hpp>
#include "BuffPool.h"
#include "LoopList.h"

enum CONN_TYPE
{
	CONN_NONE,
	CONN_CLIENT,
	CONN_SERVER,
	CONN_HTTP_CLIENT,
	CONN_PHP_CGI,
};

enum CONN_STATE
{
	CONNECT,
	CLOSE,
};

enum SERVER_TYPE
{
	LOOP_SERVER_NONE = -1,
	LOOP_GATE = 0,
	LOOP_PROXY = 1,
	LOOP_LOGIN = 2,
	LOOP_MYSQL = 3,
	LOOP_PROXY_PP = 4,
	LOOP_MASTER = 5,
	LOOP_CONSOLE = 6,
	LOOP_PROXY_DB = 7,
	LOOP_ROOM = 8,
	LOOP_ROOM_MANAGER = 9,
	LOOP_LOGIN_LOCK = 10,

	//-------------------------
	LOOP_BATTLE_SCENE = 11,
	LOOP_BATTLE_TRANS = 12,
	LOOP_MATCH = 13,
	LOOP_MATCH_STATE = 14,
	LOOP_TEAM = 15,
	LOOP_TEAM_STATE = 16,
	LOOP_MYSQL_ACCOUNT = 17,
	LOOP_PROXY_TEAM = 18,

	LOOP_SERVICE_FIND = 40,

	LOOP_SERVER_END,
};

enum LAYER_TYPE
{
	LY_NET,
	LY_UDP_NET,
	LY_LOGIC,
	LY_HTTP_LOGIC,
	LY_LOG,
	LY_MYSQL,
};

struct NetBuffer
{
	char* buf;
	uint32_t len;
	uint32_t use;
	uint32_t scan;

	NetBuffer():buf(NULL),len(0),use(0),scan(0)
	{}

	~NetBuffer()
	{
		if (buf)
			PUSH_POOL_BUFF(buf, len);
		//free(buf);
	}

	NetBuffer(NetBuffer&& b):buf(b.buf),len(b.len),use(b.use),scan(b.scan)
	{
		b.buf = NULL;
		b.use = b.len = b.scan = 0;
	}

	NetBuffer& operator=(NetBuffer&& b)
	{
		assert(this != &b);
		swap(buf, b.buf);
		/*buf = ;
		b.buf = NULL;*/

		swap(len,b.len);
		swap(use,b.use);
		swap(scan,b.scan);
		
		//b.len = b.use = b.scan = 0;
		return *this;
	}

	void Clear()
	{
		if (buf)
			PUSH_POOL_BUFF(buf, len);
		//free(buf);
		buf = NULL;
		len = use = scan = 0;
	}

	void MakeRoome(const uint32_t& size)
	{
		if (len>=size)
			return;
		//auto room = (char*)malloc(size);
		int32_t newlen = 0;
		auto room = GET_POOL_BUFF(size, newlen);
		if (use > 0 && buf)
		{
			memcpy(room, buf, use);
			PUSH_POOL_BUFF(buf, len);
		}
		buf = room;
		len = static_cast<uint32_t>(newlen);
	}

	void combin(char* newbuf, uint32_t nlen)
	{
		if (nlen <= 0)
			return;
		if (use + nlen <= len)
		{
			memcpy(buf + use, newbuf, nlen);
			use += nlen;
		}
		else
		{
			/*char* room = (char*)malloc(use + nlen);
			if (buf)
			{
				if (use>0)
					memcpy(room, buf, use);
				free(buf);
			}*/
			MakeRoome(use + nlen);
			memcpy(buf + use, newbuf, nlen);
			len = use + nlen;
			use = len;
		}
	}

	void moveHalf(const int& readed)
	{
		if (readed == 0)
			return;
		int nuse = use - readed;
		if (nuse>0)
			memcpy(buf, buf + readed, nuse);
		use = nuse;
		scan = 0;
	}

	NetBuffer& append(char* p, uint32_t len)
	{
		combin(p, len);
		return *this;
	}

	NetBuffer& append(char* p)
	{
		combin(p, (uint32_t)strlen(p));
		return *this;
	}

	NetBuffer& append(const string& s)
	{
		combin(const_cast<char*>(s.c_str()), (uint32_t)s.size());
		return *this;
	}
};

struct NetObject:public LoopObject
{
	int socket;
	int64_t ctime;
	int type;
	// ͨ�� LoopObject �̳�
	void init(FactorManager * fm)
	{
		ctime = Loop::GetSecend();
		type = CONN_NONE;
	}

	void recycle(FactorManager * fm)
	{

	}
};

struct TransHead
{
	int8_t size;
	int8_t index;

	enum
	{
		SIZE = 2,
	};
};

namespace bc = boost::coroutines2;
typedef bc::coroutine<SHARE<BaseMsg>&>::push_type c_push;
typedef bc::coroutine<SHARE<BaseMsg>&>::pull_type c_pull;

#define CORO_TIME_OUT 5		//second

struct BaseCoro:public Loop::mnode,public LoopObject
{
	virtual void init(FactorManager* fm)
	{
		m_coroId = 0;
		m_endPoint = 0;
		m_next = NULL;
		m_prev = NULL;
	}

	virtual void recycle(FactorManager* fm)
	{
		if (m_coro) 
			m_coro.reset();
	}

	void SetCoro(c_push* nCo)
	{
		m_coro.reset(nCo);
	}

	void Refresh(const int32_t& nCoid)
	{
		m_coroId = nCoid;
		m_endPoint = Loop::GetSecend() + CORO_TIME_OUT;
	}

	inline void SetFail() { m_endPoint = -1; };

	inline bool IsFail() { return m_endPoint == -1; };

	void Clear()
	{
		if(m_coro)
			m_coro.reset();
	}

	int64_t m_endPoint;
	int32_t m_coroId;
	SHARE<c_push> m_coro;
};

union Int32Struct
{
	Int32Struct() :i32(0)
	{}

	Int32Struct(int32_t n) :i32(n)
	{}

	Int32Struct(int32_t h, int32_t l) :height16(h),lower16(l)
	{}

	void set(int32_t h, int32_t l)
	{
		height16 = h;
		lower16 = l;
	}

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

	void set(int32_t h, int32_t l)
	{
		height32 = h;
		lower32 = l;
	}

	int64_t i64;
	struct
	{
		int32_t lower32;
		int32_t height32;
	};
};

#endif