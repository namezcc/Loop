#ifndef PACK_MODULE_I_H
#define PACK_MODULE_I_H
#include "BaseModule.h"
#include <uv.h>

class NetModule;

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
	NetModule* netmodule;
	NetBuffer buffer;
	int socket;
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
		HEAD_SIZE = PACK_HEAD_SIZE,
	};

	static void Encode(char* buf,const int& mid,int len)
	{
		int flag = (len & mid) ^ HEAD_FLAG;

		PB::WriteInt(buf, len);
		PB::WriteInt(buf+4, mid);
		PB::WriteInt(buf+8, flag);
		/*buf[0] = (char)len;
		buf[1] = (char)len>>8;
		buf[2] = (char)len>>16;
		buf[3] = (char)len>>24;

		buf[4] = (char)mid;
		buf[5] = (char)mid >> 8;
		buf[6] = (char)mid >> 16;
		buf[7] = (char)mid >> 24;

		buf[8] = (char)flag;
		buf[9] = (char)flag >> 8;
		buf[10] = (char)flag >> 16;
		buf[11] = (char)flag >> 24;*/
	}

	static bool Decode(MsgHead& mh,char* buf)
	{
		mh.size = PB::GetInt(buf);
		mh.mid = PB::GetInt(buf+4);
		mh.flag = PB::GetInt(buf+8);

		/*mh.size = buf[0];
		mh.size |= buf[1] << 8;
		mh.size |= buf[2] << 16;
		mh.size |= buf[3] << 24;

		mh.mid = buf[4];
		mh.mid |= buf[5] << 8;
		mh.mid |= buf[6] << 16;
		mh.mid |= buf[7] << 24;

		mh.flag = buf[8];
		mh.flag |= buf[9] << 8;
		mh.flag |= buf[10] << 16;
		mh.flag |= buf[11] << 24;*/

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