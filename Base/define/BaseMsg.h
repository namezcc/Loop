#ifndef BASE_MSG_H
#define BASE_MSG_H
#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <memory>
#include "Define.h"
#include "FactorManager.h"
//#include "protoPB/base/LPBase.pb.h"
#include <google/protobuf/message.h>


#define SAFE_FREE(ptr) if(ptr){free(ptr);ptr=NULL;}

struct NoCopy
{
	NoCopy() = default;
	NoCopy(const NoCopy&) = delete;  //��ֹ����
	NoCopy & operator = (const NoCopy&) = delete; //��ֹ��ֵ
	~NoCopy() = default;
};

struct ServerNode
{
	int8_t type;
	int16_t serid;
	enum
	{
		SIZE = 3,
	};
};

struct BaseData
{
	BaseData() = default;
	BaseData(const BaseData&)
	{}
	BaseData & operator = (const BaseData&)
	{
		return *this;
	}


	virtual ~BaseData()
	{}

	virtual void initMsg()=0;
	virtual void recycleMsg()=0;

	void* m_looplist;
};

struct BaseMsg:public BaseData
{
	BaseMsg():m_data(NULL)
	{}

	virtual void initMsg() override {};
	virtual void recycleMsg() override;
	int32_t msgId;
	BaseData* m_data;
};


struct CoroMsg:public BaseMsg
{
	virtual void initMsg() override {
		m_coroId = -1;
		m_subMsgId = -1;
		m_mycoid = -1;
	}
	int32_t m_coroId;
	int32_t m_subMsgId;
	int32_t m_mycoid;
};


struct LOOP_EXPORT BuffBlock:public BaseData,public LoopObject
{
	char* m_buff;
	int32_t m_size;
	BuffBlock* m_next;

	BuffBlock();

	void makeRoom(const int32_t& size);
	void append(const char* buf, const int32_t& size);
	void write(char* buf, const int32_t& size);
	virtual void initMsg() override {
		m_size = 0;
	};
	virtual void recycleMsg() override;

	// ͨ�� LoopObject �̳�
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;
};

class BaseLayer;

struct LOOP_EXPORT NetMsg:public BaseData
{
	NetMsg();

	virtual void initMsg() override;
	virtual void recycleMsg() override;

	void push_front(BuffBlock* buff);
	virtual void push_front(BaseLayer* l,const char* buf,const int32_t& size);
	SHARE<BuffBlock> getCombinBuff(BaseLayer* l);
	void write_front(const char* buf, const int32_t& size);

	inline int32_t getLen() { return len; };

	int32_t socket;
	int32_t mid;
	BuffBlock* m_buff;
protected:
	int32_t len;
};


struct LOOP_EXPORT NetServerMsg:public NetMsg,public LoopObject
{
	virtual void initMsg() override {
		NetMsg::initMsg();
		path.clear();
	};
	virtual void recycleMsg() override;
	std::vector<std::shared_ptr<ServerNode>> path;
	// ͨ�� LoopObject �̳�
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;
	//path ֻ����init��ʱ������
	virtual void push_front(BaseLayer* l, const char* buf, const int32_t& size);
};


struct LOOP_EXPORT NetSocket:public BaseData
{
	virtual void initMsg() override {};
	virtual void recycleMsg() override;
	int32_t socket;
};

struct LOOP_EXPORT NetServer:public BaseData
{
	virtual void initMsg() override {};
	virtual void recycleMsg() override;

	int32_t type;
	int32_t serid;
	int32_t socket;
	int32_t state;
	std::string ip;
	int32_t port;
};

struct LOOP_EXPORT LogInfo:public BaseData
{
	virtual void initMsg() override {};
	virtual void recycleMsg() override;
	
	int32_t level;
	std::stringstream log;
};

namespace gpb = google::protobuf;

struct LOOP_EXPORT PB
{
	static BuffBlock* PBToBuffBlock(BaseLayer* l,google::protobuf::Message& msg);
	static BuffBlock* PBToBuffBlock(BaseLayer* l, google::protobuf::Message& msg,const int32_t& append);

	static int32_t GetInt(char* msg)
	{
		int32_t res = msg[0]&0xff;
		res |= (0xff & msg[1]) << 8;
		res |= (0xff & msg[2]) << 16;
		res |= (0xff & msg[3]) << 24;
		return res;
	}

	static void WriteInt(char* res,int32_t n)
	{
		res[0] = (unsigned char)n;
		res[1] = (unsigned char)(n >> 8);
		res[2] = (unsigned char)(n >> 16);
		res[3] = (unsigned char)(n >> 24);
	}
};

#endif
