#ifndef PACK_MODULE_I_H
#define PACK_MODULE_I_H
#include "BaseModule.h"
#include <uv.h>
#include "ProtoDefine.h"

class NetModule;
class Protocol;

struct Conn:public LoopObject
{
	Conn()
	{
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
		buffer.Clear();
		if (conn)
		{
			LOOP_RECYCLE(conn);
			conn = NULL;
		}
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
		buf.base = NULL;
		buf.len = 0;
		if (block)
		{
			--block->m_ref;
			if (block->m_ref <= 0)
				LOOP_RECYCLE(block);
			block = NULL;
		}
	}

	void SetBlock(LocalBuffBlock* b)
	{
		buf.base = b->m_buff;
		buf.len = b->getSize();
		block = b;
		++block->m_ref;
	}

	uv_buf_t buf;
	LocalBuffBlock* block;
};

class MsgModule;

class LOOP_EXPORT NetModule:public BaseModule
{
public:
	NetModule(BaseLayer* l);
	~NetModule() {};
	void SetProtoType(ProtoType ptype);
	void SetBind(const int& port, uv_loop_t* loop);
	void StartListen();

	static void Connection_cb(uv_stream_t* serhand, int status);
	static void read_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf);
	static void after_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
	void Connected(uv_tcp_t* conn,bool client=true);
protected:
	virtual void Init();
	virtual void Execute();
	virtual void AfterInit();

	static void on_close_client(uv_handle_t* client);
	static void OnActiveClose(uv_handle_t* client);

	virtual bool ReadPack(Conn* conn, char* buf, int len);
	static void After_write(uv_write_t* req, int status);

	void OnCloseSocket(NetMsg* msg);
	void OnSocketSendData(NetMsg* nMsg);
	void OnBroadData(BroadMsg* nMsg);
	void OnConnectServer(NetServer* ser);

	static void ConnectServerBack(uv_connect_t* req, int status);
protected:
	Protocol * m_proto;

	MsgModule* m_mgsModule;

	Conn* m_conns[MAX_CLIENT_CONN];
	std::list<int32_t> m_sock_pool;

	int32_t m_port;
	uv_tcp_t m_hand;
	uv_loop_t* m_uvloop;
};

#endif