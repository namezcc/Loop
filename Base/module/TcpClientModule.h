#ifndef TCP_CLIENT_MODULE_H
#define TCP_CLIENT_MODULE_H
#include "BaseModule.h"
#include <uv.h>

class NetModule;
class MsgModule;
class TcpClientModule:public BaseModule
{
public:
	TcpClientModule(BaseLayer* l);
	~TcpClientModule();

	virtual void Init() override;
	virtual void Execute() override;
	inline void Setuvloop(uv_loop_t* loop) { m_uvloop = loop; };
protected:

	void OnConnectServer(NetServer* ser);
	static void Connect_cb(uv_connect_t* req, int status);
	//static void On_server_close(uv_handle_t* client);
private:
	NetModule* m_netmodule;
	MsgModule* m_msgmodule;
	uv_loop_t* m_uvloop;
};

#endif