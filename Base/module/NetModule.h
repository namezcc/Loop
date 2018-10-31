#ifndef PACK_MODULE_I_H
#define PACK_MODULE_I_H
#include "BaseModule.h"
#include <uv.h>

class NetModule;

struct Conn:public LoopObject
{
	Conn()
	{
		socket = ++Conn::SOCKET;
	}

	void init(FactorManager* fm)
	{
		buffer.buf = NULL;
		buffer.len = 0;
		buffer.use = 0;
		conn = NULL;
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

	static thread_local int32_t SOCKET;
};

struct Write_t:public LoopObject
{
	void init(FactorManager* fm)
	{
	}

	void recycle(FactorManager* fm)
	{
		//SAFE_FREE(buf.base);
		buf.base = NULL;
		buf.len = 0;
		if (block)
		{
			--block->m_ref;
			if (block->m_ref <= 0)
			{
				if (block->m_looplist)
					baseModule->RECYCLE_LAYER_MSG(block);
				else
					baseModule->LOOP_RECYCLE((LocalBuffBlock*)block);
			}
			block = NULL;
		}
	}

	void SetBlock(BuffBlock* b)
	{
		buf.base = b->m_buff;
		buf.len = b->m_size;
		block = b;
		++block->m_ref;
		if (block->m_looplist)
			block->recycleCheck();
	}

	uv_buf_t buf;
	BuffBlock* block;
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
		len += HEAD_SIZE;
		int flag = (len & mid) ^ HEAD_FLAG;

		PB::WriteInt(buf, len);
		PB::WriteInt(buf+4, mid);
		PB::WriteInt(buf+8, flag);
	}

	static bool Decode(MsgHead& mh,char* buf)
	{
		mh.size = PB::GetInt(buf);
		mh.mid = PB::GetInt(buf+4);
		mh.flag = PB::GetInt(buf+8);
		return mh.flag == ((mh.size & mh.mid)^HEAD_FLAG);
	}
}MsgHead;

class LOOP_EXPORT NetModule:public BaseModule
{
public:
	NetModule(BaseLayer* l):BaseModule(l)
	{};
	~NetModule() {};
	static void read_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf);
	static void after_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
	void Connected(uv_tcp_t* conn,bool client=true);
	void RemoveConn(const int& socket);
	inline MsgModule* GetMsgModule() { return m_mgsModule; };
	inline void Setuvloop(uv_loop_t* loop) { m_uvloop = loop; };
protected:
	virtual void Init();
	virtual void Execute();

	static void on_close_client(uv_handle_t* client);
	static void OnActiveClose(uv_handle_t* client);

	virtual bool ReadPack(Conn* conn, char* buf, int len);
	static void After_write(uv_write_t* req, int status);

	void OnCloseSocket(NetSocket* msg);
	void OnSocketSendData(NetMsg* nMsg);
	void OnBroadData(BroadMsg* nMsg);
protected:
	MsgModule* m_mgsModule;
	std::unordered_map<int32_t, SHARE<Conn>> m_conns;
	std::unordered_map<int32_t, SHARE<Conn>> m_waitClose;
	uv_loop_t* m_uvloop;
	std::vector<BuffBlock*> m_broadBuff;
};

#endif