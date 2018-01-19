#ifndef PACK_MODULE_I_H
#define PACK_MODULE_I_H
#include "BaseModule.h"
#include <uv.h>

struct NetBuffer
{
	char* buf;
	int len;
	int use;

	void combin(char* newbuf, int nlen)
	{
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
				if(use>0)
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
		if(nuse>0)
			memcpy(buf, buf + readed, nuse);
		use = nuse;
	}
};

struct Conn:public LoopObject
{
	void init(FactorManager* fm)
	{
		buffer.buf = nullptr;
		buffer.len = 0;
		buffer.use = 0;
		conn = nullptr;
	}

	void recycle(FactorManager* fm)
	{
		if (buffer.buf)
			delete[] buffer.buf;
		fm->recycle(conn);
	}

	uv_tcp_t* conn;
	NetBuffer buffer;
};

struct Write_t:public LoopObject
{
	void init(FactorManager* fm)
	{
	}

	void recycle(FactorManager* fm)
	{
		delete[] buf.base;
	}

	uv_buf_t buf;
	BaseModule* baseModule;
};

#define HEAD_FLAG 0x55555555
class MsgModule;

struct Head
{
	int size;
	int mid;
	int flag;
};

typedef struct MsgHead:public Head
{
	enum
	{
		HEAD_SIZE = sizeof(Head),
	};

	static void Encode(char* buf,const int& mid,int len)
	{
		MsgHead mh;
		mh.size = len+sizeof(mh);
		mh.mid = mid;
		mh.flag = (mh.size & mh.mid)^ HEAD_FLAG;
		memcpy(buf, &mh, sizeof(mh));
	}

	static bool Decode(MsgHead& mh,char* buf)
	{
		memcpy(&mh, buf, sizeof(mh));
		return mh.flag == ((mh.size & mh.mid)^HEAD_FLAG);
	}
}MsgHead;

class NetModule:public BaseModule
{
public:
	NetModule(BaseLayer* l):BaseModule(l) 
	{};
	~NetModule() {};
	static void read_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf)
	{
		buf->base = new char[suggested_size];
		buf->len = suggested_size;
	}

	static void after_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
	void Connected(uv_tcp_t* conn,bool client=true);
	void RemoveConn(const int& socket);
	inline MsgModule* GetMsgModule() { return m_mgsModule; };
protected:
	void Init();
	void Execute();

	/*static void after_shutdown(uv_shutdown_t* shutdown, int status)
	{
		auto server = (NetModule*)shutdown->data;
		uv_close((uv_handle_t*)shutdown->handle, shutdown->handle->close_cb);
		server->GetLayer()->Recycle(shutdown);
	}*/

	static void on_close_client(uv_handle_t* client);

	bool ReadPack(int socket, char* buf, int len);
	static void After_write(uv_write_t* req, int status);
private:

	void OnCloseSocket(NetSocket* msg);
	void OnSocketSendData(NetMsg* nMsg);

private:
	MsgModule* m_mgsModule;
	std::unordered_map<int, Conn*> m_conns;
};

#endif