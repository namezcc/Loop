#ifndef DATA_DEFINE_H
#define DATA_DEFINE_H
#include "FactorManager.h"
#include "LTime.h"
#include "BaseMsg.h"
#include <cstring>
#include <boost/coroutine2/all.hpp>

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
	LOOP_GAME = 0,
	LOOP_PROXY_SQL = 1,
	LOOP_LOGIN = 2,
	LOOP_MYSQL = 3,
	LOOP_PROXY_PP = 4,
	LOOP_MASTER = 5,
	LOOP_CONSOLE = 6,
	LOOP_PROXY_DB = 7,
	LOOP_SERVER_END,
};

enum LAYER_TYPE
{
	LY_NET,
	LY_LOGIC,
	LY_HTTP_LOGIC,
	LY_LOG,
	LY_MYSQL,
};

struct NetBuffer
{
	char* buf;
	int len;
	int use;
	int scan;

	NetBuffer():buf(nullptr),len(0),use(0),scan(0)
	{}

	~NetBuffer()
	{
		if (buf)
			free(buf);
	}

	NetBuffer(NetBuffer&& b):buf(b.buf),len(b.len),use(b.use),scan(b.scan)
	{
		b.buf = nullptr;
		b.use = b.len = b.scan = 0;
	}

	NetBuffer& operator=(NetBuffer&& b)
	{
		assert(this != &b);
		swap(buf, b.buf);
		/*buf = ;
		b.buf = nullptr;*/

		swap(len,b.len);
		swap(use,b.use);
		swap(scan,b.scan);
		
		//b.len = b.use = b.scan = 0;
		return *this;
	}

	void Clear()
	{
		if (buf)
			free(buf);
		buf = nullptr;
		len = use = scan = 0;
	}

	void MakeRoome(const int& size)
	{
		if (len>=size)
			return;
		auto room = (char*)malloc(size);
		if (use > 0)
			memcpy(room, buf, use);
		free(buf);
		buf = room;
		len = size;
	}

	void combin(char* newbuf, int nlen)
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
			char* room = (char*)malloc(use + nlen);
			if (buf)
			{
				if (use>0)
					memcpy(room, buf, use);
				free(buf);
			}
			memcpy(room + use, newbuf, nlen);
			buf = room;
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

	NetBuffer& append(char* p, int len)
	{
		combin(p, len);
		return *this;
	}

	NetBuffer& append(char* p)
	{
		combin(p, strlen(p));
		return *this;
	}

	NetBuffer& append(const string& s)
	{
		combin(const_cast<char*>(s.c_str()), s.size());
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
		ctime = GetSecend();
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

struct BaseCoro:public LoopObject
{
	virtual void init(FactorManager* fm)
	{
		m_coroId = 0;
		m_endPoint = 0;
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
		m_endPoint = GetSecend() + CORO_TIME_OUT;
	}

	void Clear()
	{
		if(m_coro)
			m_coro.reset();
	}

	int64_t m_endPoint;
	int32_t m_coroId;
	SHARE<c_push> m_coro;
};

#endif