#ifndef PACK_MODULE_I_H
#define PACK_MODULE_I_H
#include "BaseModule.h"
#include <uv.h>

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

class LOOP_EXPORT NetModule:public BaseModule
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
	inline void Setuvloop(uv_loop_t* loop) { m_uvloop = loop; };
protected:
	virtual void Init();
	virtual void Execute();

	static void on_close_client(uv_handle_t* client);

	virtual bool ReadPack(int socket, char* buf, int len);
	static void After_write(uv_write_t* req, int status);

	void OnCloseSocket(NetSocket* msg);
	void OnSocketSendData(NetMsg* nMsg);
protected:
	MsgModule* m_mgsModule;
	std::unordered_map<int, SHARE<Conn>> m_conns;
	uv_loop_t* m_uvloop;
};

#endif