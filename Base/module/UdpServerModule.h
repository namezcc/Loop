#ifndef UDP_SERVER_MODULE_H
#define UDP_SERVER_MODULE_H

#include "BaseModule.h"
//#include <boost/asio.hpp>
#include "LoopHeap.h"
#include "LoopIO.h"

//namespace as = boost::asio;
//using as::ip::udp;


#define UDP_CASH_SIZE 256
#define UDP_MUT_SIZE 512
#define UDP_DATA_SIZE 500
#define UDP_OUT_TIME_RESEND 100
#define UDP_OUT_TIME_REQ_PACK 10
#define UDP_OUT_TIME_LINK 300000

enum PACK_TYPE
{
	NOR_PACK,
	ACK_GET,
	PING_PACK,
};

struct UdpBuff;
struct UdpConn;

struct RsendNode
{
	int32_t sock;
	uint16_t packId;
	int64_t point;
	UdpBuff** m_buff;
	UdpConn* m_conn;
	RsendNode* next;
};

struct UdpBuff :public LoopObject
{
	uint16_t packId;
	char buf[UDP_MUT_SIZE];
	int32_t offset;
	int32_t size;
	RsendNode* m_rsendNode;

	virtual void init(FactorManager * fm) {
		packId = 0;
		size = 0;
		offset = 0;
		m_rsendNode = NULL;
	};
	virtual void recycle(FactorManager * fm)
	{
		if (m_rsendNode)
		{
			if (m_rsendNode->m_buff)
			{
				assert(*m_rsendNode->m_buff == this);
				m_rsendNode->m_buff = NULL;
			}
			m_rsendNode = NULL;
		}
	}

	template<typename T>
	void write(T&& t)
	{
		typename std::decay<T>::type* p = (typename std::decay<T>::type*)((char*)buf + size);
		*p = t;
		size += sizeof(typename std::decay<T>::type);
	}

	template<typename T>
	void write(const int16_t& _offset, T&& t)
	{
		typename std::decay<T>::type* p = (typename std::decay<T>::type*)((char*)buf + _offset);
		*p = t;
	}

	void write(const char* buff, int16_t _size)
	{
		memcpy((void*)((char*)buf + size), buff, _size);
		size += _size;
	}

	template<typename T>
	T read()
	{
		if (offset + sizeof(T) > size)
		{
			assert(0);
			return T();
		}
		T t = *((T*)((char*)buf + offset));
		offset += sizeof(T);
		return t;
	}

	template<typename T>
	T read(const int16_t& _offset)
	{
		if (_offset + sizeof(T) > size)
		{
			assert(0);
			return T();
		}
		T t = *((T*)((char*)buf + _offset));
		return t;
	}
};


struct UdpConn :public LoopObject
{
	int32_t socket;
	uint16_t maxPackId;
	uint8_t maxIndex;
	int64_t outTime;
	int32_t heapIndex;
	UdpBuff* hestory[UDP_CASH_SIZE];
	//as::ip::udp::endpoint addr;
	Addr m_naddr;

	FactorManager* m_fm;
	static thread_local int32_t SOCKET;

	UdpConn()
	{
		socket = ++UdpConn::SOCKET;
		auto s = sizeof(hestory);
		memset(hestory, 0, sizeof(hestory));
	}

	UdpBuff** PushHestory(UdpBuff* buff,bool _ack = false)
	{
		buff->packId = maxPackId++;
		if(_ack)
			hestory[maxIndex] = buff;
		UdpBuff** ptr = &hestory[maxIndex];
		MoveIndex();
		return ptr;
	}

	void MoveIndex()
	{
		++maxIndex;
		if (hestory[maxIndex])
		{
			m_fm->recycle(hestory[maxIndex]);
			hestory[maxIndex] = NULL;
		}
	}

	// Í¨¹ý LoopObject ¼Ì³Ð
	virtual void init(FactorManager * fm) {
		maxPackId = 0;
		maxIndex = 0;
		heapIndex = -1;
		m_fm = fm;
	};
	virtual void recycle(FactorManager * fm) {
		for (size_t i = 0; i < UDP_CASH_SIZE; i++)
		{
			if (hestory[i])
			{
				fm->recycle(hestory[i]);
				hestory[i] = NULL;
			}
		}
	};
};

class ScheduleModule;

class LOOP_EXPORT UdpServerModule:public BaseModule
{
public:
	struct UdpConnLess
	{
		bool operator()(const SHARE<UdpConn>& l, const SHARE<UdpConn>& r)
		{
			return l->outTime < r->outTime;
		}
	};

	UdpServerModule(BaseLayer* l);
	~UdpServerModule();

	virtual void Init() override;
	virtual void Execute() override;

	void Listen(const int32_t& port);
	inline void BindOnConnect(const std::function<void(int32_t)>& f) { m_onConnect = f; };
	inline void BindOnClose(const std::function<void(int32_t)>& f) { m_onClose = f; }
	void SendData(int32_t sock, const char* data, const int32_t& size,bool _ack = false);
	inline void BindOnReadPack(const std::function<void(const int32_t&, const char*, const int32_t&)>& f) { m_onRead = f; };
	void CloseSocket(int32_t sock, bool call = true);
protected:
	void Loop_Once(int64_t& dt);
	inline void SetTick(const int64_t& dt) { m_nowTick = dt; };

	void Do_receive();
	void ReceiveDecode(UdpBuff* buf);
	void ReceiveNorPack(const int32_t& sock, SHARE<UdpConn>& conn, UdpBuff* buf);
	void ReceiveAckPack(const int32_t& sock, SHARE<UdpConn>& conn, UdpBuff* buf);
	void ReceivePingPack(SHARE<UdpConn>& conn);

	void OnClientConnect();

	UdpBuff* DecodeSendBuff(SHARE<UdpConn>& conn, const char* buff, const int16_t& size, const int8_t& pn, const int8_t& idx,bool _ack = false);
	void SendData(SHARE<UdpConn>& conn, UdpBuff* buff);
	void SendData(UdpConn* conn, UdpBuff* buff);
	void SendDataRecycle(SHARE<UdpConn>& conn, UdpBuff* buff);
	//void RealSendData(const char* data, const int32_t& len,udp::endpoint& addr);
	void RealSendData(const char* data, const int32_t& len,const Addr& addr);

	void CheckReSend();
	void CheckOutTime();
	void PushBack(UdpConn* conn,UdpBuff** buff);
	void PushBack(RsendNode* node);
	RsendNode* PopHead();
private:

	int64_t m_nowTick;

	//as::io_context m_context;
	//SHARE<udp::socket> m_socket;

	LoopUDPIO m_loopctx;
	UdpSocket m_udpsock;

	std::unordered_map<int32_t, SHARE<UdpConn>> m_clients;
	//std::map<int32_t, SHARE<UdpConn>> m_clients;

	MinHeap<std::shared_ptr<UdpConn>, UdpConnLess> m_heapOutTime;
	
	//udp::endpoint m_accept;
	Addr m_naccept;

	UdpBuff* m_tmpcash;

	std::function<void(int32_t)> m_onConnect;
	std::function<void(int32_t)> m_onClose;
	std::function<void(const int32_t&, const char*, const int32_t&)> m_onRead;

	RsendNode m_rSendHead;
	RsendNode* m_rSendTail;

	ScheduleModule* m_schedule;
};

#endif
