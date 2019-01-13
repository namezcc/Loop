#ifndef LOOP_IO_H
#define LOOP_IO_H
#include <thread>
#include "LoopArray.h"
#include <WinSock2.h>
#include <MSWSock.h>
#include <functional>
#include <chrono>

#define IO_WORK_THREAD_NUM 1
#define IO_SEND_THREAD_NUM 7
#define LP_ENTRY_SIZE 10

enum OV_TYPE
{
	OV_RECV,
	OV_SEND,
};

struct OV :public OVERLAPPED
{
	int _ovtype;
	int32_t _err;
	void* _socket;
};

struct Addr
{
	int _port;
	sockaddr_in _addr;
	Addr() {};
	Addr(const int& port) :_port(port), _addr({ 0 })
	{
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(port);
		_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	}

	Addr(const char* ip, const int& port) :_port(port), _addr({ 0 })
	{
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(port);
		_addr.sin_addr.S_un.S_addr = inet_addr(ip);
	}

	Addr(const sockaddr_in& addr) :_addr(addr)
	{}
};

typedef std::function<void(const int32_t&, const int32_t&, const Addr&, void*)> _ReadCallBack;
typedef std::function<void(const int32_t&, const int32_t&, void*)> _SendCallBack;

struct OVR :public OV
{
	OVR()
	{
		memset(this, 0, sizeof(OVR));
	}
	Addr	_addr_from;
	char*	_buf;
	int32_t	_recv_size;
	void*	_userdata;
};

struct OVS :public OV
{
	OVS()
	{
		memset(this, 0, sizeof(OVS));
	}

	char*	_buf;
	int32_t	_send_size;
	Addr	_addr;
	void*	_userdata;
};

class IOCP
{
	friend class Single;
public:
	~IOCP()
	{};

	HANDLE GetIocp() { return m_iocp; }
	void SendOVSData(void* ovs);
private:
	IOCP():m_sendIndex(0)
	{
		WSADATA wsd;
		WORD wv = MAKEWORD(2, 2);
		int r = WSAStartup(wv, &wsd);
		if (r == SOCKET_ERROR)
			WSACleanup();

		m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

		InitThread();
	}

	void InitThread()
	{
		for (uint32_t i = 0; i < IO_WORK_THREAD_NUM; i++)
		{
			m_worker.emplace_back([this, i]() {
				DoRecv(i);
			});
		}

		for (uint32_t i = 0; i < IO_SEND_THREAD_NUM; i++)
		{
			m_sender.emplace_back([this, i]() {
				DoSend(i);
			});
		}
	}

	void DoRecv(const uint32_t& tidx);
	void DoSend(const uint32_t& tidx);

private:
	HANDLE m_iocp;

	std::vector<std::thread> m_worker;
	std::vector<std::thread> m_sender;

	int32_t m_sendIndex;
	LoopList<OVS*> m_sendData[IO_SEND_THREAD_NUM];
	OVERLAPPED_ENTRY m_entry[LP_ENTRY_SIZE * IO_WORK_THREAD_NUM];
};

class LoopUDPIO
{
	typedef void(*IODownFunc)(void*);
	struct IODown
	{
		IODownFunc func;
		void* data;
		LoopList<IODown*>* poolptr;
	};
public:
	LoopUDPIO()
	{
		m_iocp = Single::GetInstence<IOCP>()->GetIocp();
	}

	void Loop_Once()
	{
		IODown* down = NULL;
		for (size_t i = 0; i < IO_WORK_THREAD_NUM; i++)
		{
			while (m_down[i].pop(down))
			{
				down->func(down->data);
				down->poolptr->write(down);
			}
		}
		for (size_t i = 0; i < IO_SEND_THREAD_NUM; i++)
		{
			while (m_sendDown[i].pop(down))
			{
				down->func(down->data);
				down->poolptr->write(down);
			}
		}
	}

	HANDLE GetIocp() { return m_iocp; }

	//other thread call
	void PushDownRecv(const int32_t& tIndex, IODownFunc func, void* data)
	{
		auto poollist = Single::LocalInstance<LoopList<IODown*>>();
		IODown* down = NULL;
		if (!poollist->pop(down))
		{
			down = Single::LocalInstance<FactorManager>()->getLoopObj<IODown>();
			down->poolptr = poollist;
		}
		down->func = func;
		down->data = data;
		m_down[tIndex].write(down);
	}

	void PushDownSend(const int32_t& tIndex, IODownFunc func, void* data)
	{
		auto poollist = Single::LocalInstance<LoopList<IODown*>>();
		IODown* down = NULL;
		if (!poollist->pop(down))
		{
			down = Single::LocalInstance<FactorManager>()->getLoopObj<IODown>();
			down->poolptr = poollist;
		}
		down->func = func;
		down->data = data;
		m_sendDown[tIndex].write(down);
	}

private:
	HANDLE m_iocp;
	LoopList<IODown*> m_down[IO_WORK_THREAD_NUM];
	LoopList<IODown*> m_sendDown[IO_SEND_THREAD_NUM];
};

class UdpSocket
{
	friend class IOCP;
public:
	UdpSocket(const LoopUDPIO& ctx);
	~UdpSocket()
	{}

	int Bind(const int32_t& port);

	inline void BindRead(const _ReadCallBack& _call) { m_readCall = _call; }
	inline void BindSend(const _SendCallBack& _call) { m_sendCall = _call; }

	int RecvFrom(const char* buf, const int32_t& _maxsize, void* _userdata = NULL)
	{
		wb.buf = (CHAR*)buf;
		wb.len = _maxsize;
		OVR* ovr = m_ovrpool->get();
		ovr->_socket = this;
		ovr->_buf = (char*)buf;
		ovr->_ovtype = OV_TYPE::OV_RECV;
		ovr->_userdata = _userdata;

		int ret = WSARecvFrom(m_socket, &wb, 1, NULL, &m_wsaflag, (sockaddr*)&ovr->_addr_from._addr,
			&m_addr_size, (LPWSAOVERLAPPED)ovr, 0);
		if (ret == SOCKET_ERROR)
		{
			ret = WSAGetLastError();
			if (ret != ERROR_IO_PENDING)
				printf("RecvFrom Error %d", ret);
		}
		return ret;
	}

	int SendTo(const char* buf,const int32_t& size,const Addr& addr,void* _userdata = NULL)
	{
		OVS* sdata = m_ovspool->get();

		sdata->_socket = this;
		sdata->_buf = (char*)buf;
		sdata->_send_size = size;
		sdata->_addr = addr;
		sdata->_userdata = _userdata;

		m_insiocp->SendOVSData(sdata);
		return 0;
	}

protected:

	static void Do_recv(void* data)
	{
		OVR* ovr = (OVR*)data;
		UdpSocket* sock = (UdpSocket*)ovr->_socket;
		sock->m_readCall(ovr->_err, ovr->_recv_size, ovr->_addr_from, ovr->_userdata);
		sock->m_ovrpool->recycle(ovr);
	}

	static void Do_send(void* data)
	{
		OVS* ovs = (OVS*)data;
		UdpSocket* sock = (UdpSocket*)ovs->_socket;
		sock->m_sendCall(ovs->_err, ovs->_send_size, ovs->_userdata);
		sock->m_ovspool->recycle(ovs);
	}

private:
	IOCP * m_insiocp;
	LoopUDPIO * m_ctx;
	SOCKET m_socket;
	WSABUF wb;

	int m_addr_size;
	DWORD m_wsaflag;

	_ReadCallBack m_readCall;
	_SendCallBack m_sendCall;

	LoopFactor<OVR>* m_ovrpool;
	LoopFactor<OVS>* m_ovspool;
};

SET_T_BLOCK_SIZE(OVS,10000)

#endif
