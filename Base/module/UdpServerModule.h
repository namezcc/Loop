#ifndef UDP_SERVER_MODULE_H
#define UDP_SERVER_MODULE_H

#include "BaseModule.h"
#include <boost/asio.hpp>

namespace as = boost::asio;
using as::ip::udp;


#define UDP_CASH_SIZE 256
#define UDP_MUT_SIZE 512
#define UDP_DATA_SIZE 500
#define UDP_OUT_TIME_RESEND 40
#define UDP_OUT_TIME_REQ_PACK 10
#define UDP_OUT_TIME_LINK 300000

enum PACK_TYPE
{
	NOR_PACK,
	REQ_PACK,
	ACK_GET,
};

struct UdpBuff :public LoopObject
{
	uint16_t packId;
	char buf[UDP_MUT_SIZE];
	int32_t offset;
	int32_t size;
	int64_t reSendDT;

	virtual void init(FactorManager * fm) {
		packId = 0;
		size = 0;
		offset = 0;
	};
	virtual void recycle(FactorManager * fm)
	{}

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
	uint8_t minIndex;
	uint8_t maxIndex;
	UdpBuff* hestory[UDP_CASH_SIZE];
	as::ip::udp::endpoint addr;
	FactorManager* m_fm;
	static thread_local int32_t SOCKET;

	UdpConn()
	{
		socket = ++UdpConn::SOCKET;
	}

	void PushHestory(UdpBuff* buff)
	{
		buff->packId = maxPackId++;
		hestory[maxIndex] = buff;
		MoveIndex();
	}

	void MoveIndex()
	{
		++maxIndex;
		if (hestory[maxIndex])
		{
			m_fm->recycle(hestory[maxIndex]);
			hestory[maxIndex] = NULL;
		}
		if (maxIndex == minIndex)
			++minIndex;
	}

	// Í¨¹ý LoopObject ¼Ì³Ð
	virtual void init(FactorManager * fm) {
		maxPackId = 0;
		minIndex = maxIndex = 0;
		memset(hestory, 0, sizeof(hestory));
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

struct RsendNode
{
	int32_t sock;
	uint16_t packId;
	int64_t point;
	RsendNode* next;
};

class ScheduleModule;

class LOOP_EXPORT UdpServerModule:public BaseModule
{
public:
	UdpServerModule(BaseLayer* l);
	~UdpServerModule();

	virtual void Init() override;
	virtual void Execute() override;

	void Listen(const int32_t& port);
	inline void BindOnConnect(const std::function<void(int32_t)>& f) { m_onConnect = f; };
	void SendData(int32_t sock, const char* data, const int32_t& size);
	inline void BindOnReadPack(const std::function<void(const int32_t&, const char*, const int32_t&)>& f) { m_onRead = f; };
	void CloseSocket(int32_t sock);
protected:
	void Loop_Once(int64_t& dt);
	inline void SetTick(const int64_t& dt) { m_nowTick = dt; };

	void Do_receive();
	void ReceiveDecode(UdpBuff* buf);
	void ReceiveNorPack(const int32_t& sock, SHARE<UdpConn>& conn, UdpBuff* buf);
	void ReceiveReqPack(const int32_t& sock, SHARE<UdpConn>& conn, UdpBuff* buf);
	void ReceiveAckPack(const int32_t& sock, SHARE<UdpConn>& conn, UdpBuff* buf);

	void Resendpack(SHARE<UdpConn>& conn, const uint16_t& packId);

	void OnClientConnect();

	UdpBuff* DecodeSendBuff(SHARE<UdpConn>& conn, const char* buff, const int16_t& size, const int8_t& pn, const int8_t& idx);
	void SendData(SHARE<UdpConn>& conn, UdpBuff* buff);
	void RealSendData(const char* data, const int32_t& len,udp::endpoint& addr);

	void CheckReSend();
	void PushBack(const int32_t& sock, const uint16_t& packId);
	void PushBack(RsendNode* node);
	RsendNode* PopHead();
private:

	int64_t m_nowTick;
	as::io_context m_context;
	SHARE<udp::socket> m_socket;

	std::unordered_map<int32_t, SHARE<UdpConn>> m_clients;
	int32_t m_sockIndex;
	udp::endpoint m_accept;
	UdpBuff* m_tmpcash;

	std::function<void(int32_t)> m_onConnect;
	std::function<void(const int32_t&, const char*, const int32_t&)> m_onRead;

	RsendNode m_rSendHead;
	RsendNode* m_rSendTail;

	ScheduleModule* m_schedule;
};

#endif
