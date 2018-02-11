#ifndef DATA_DEFINE_H
#define DATA_DEFINE_H
#include "FactorManager.h"
#include "LTime.h"

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
	LOOP_GAME,
	LOOP_PROXY_GS,
	LOOP_PROXY_G,
	LOOP_MYSQL,
	LOOP_PROXY_PP,
	LOOP_MASTER,
	LOOP_CONSOLE,
};

static const char* server_name[] {"game","proxy-gs","proxy-g","mysql","proxy-pp","master","console"};

enum LAYER_TYPE
{
	LY_NET,
	LY_LOGIC,
	LY_HTTP_LOGIC,
	LY_LOG,
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
			delete[] buf;
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
			delete[] buf;
		buf = nullptr;
		len = use = scan = 0;
	}

	void MakeRoome(const int& size)
	{
		if (len>=size)
			return;
		auto room = new char[size];
		if (use > 0)
			memcpy(room, buf, use);
		delete[] buf;
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
			char* room = new char[use + nlen];
			if (buf)
			{
				if (use>0)
					memcpy(room, buf, use);
				delete[] buf;
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
	// Í¨¹ý LoopObject ¼Ì³Ð
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
	int size;
	int index;
};

struct ServerNode
{
	int type;
	int serid;
};


#endif