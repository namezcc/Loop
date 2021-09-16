#ifndef BASE_MSG_H
#define BASE_MSG_H
#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <memory>
#include "Define.h"
#include "FactorManager.h"
#include <google/protobuf/message.h>


#define SAFE_FREE(ptr) if(ptr){free(ptr);ptr=NULL;}

class BaseLayer;

struct ServerNode
{
	int8_t type;
	int16_t serid;
	enum
	{
		SIZE = 3,
	};
};

typedef std::vector<ServerNode> ServerPath;

struct LOOP_EXPORT BaseData
{
	BaseData()
	{};
	/*BaseData(const BaseData&)
	{}*/
	//这个函数是为了子类拷贝是防止拷贝 m_looplist
	BaseData & operator = (const BaseData&)
	{
		return *this;
	};


	virtual ~BaseData()
	{}

	virtual void initMsg()=0;
	virtual void recycleMsg()=0;
	virtual void recycleCheck(){};
};

struct LOOP_EXPORT BaseMsg:public BaseData
{
	BaseMsg():m_data(NULL)
	{}

	virtual void initMsg() override {};
	virtual void recycleMsg() override;
	int32_t msgId;
	BaseData* m_data;
};


struct LOOP_EXPORT CoroMsg:public BaseMsg
{
	virtual void initMsg() override {
		m_coroId = -1;
		m_subMsgId = -1;
		m_mycoid = -1;
	}
	virtual void recycleMsg() override;
	int32_t m_coroId;
	int32_t m_subMsgId;
	int32_t m_mycoid;
};


struct LOOP_EXPORT BuffBlock:public BaseData
{
	BuffBlock();
	virtual void makeRoom(const size_t& size);
	void extandSize(int32_t size = 0)
	{
		if (m_allsize + size > m_allsize * 2)
			makeRoom(m_allsize + size);
		else
			makeRoom(m_allsize * 2);
	}
	void writeBuff(const char* buf, const int32_t& size);
	virtual void initMsg() override {
		m_size = 0;
		m_allsize = 0;
		m_ref = 0;
		m_offect = 0;
		if (m_next != NULL) assert(0);
	};
	virtual void recycleMsg() override;
	virtual void recycleCheck() override;
	int32_t MaxSize() { return m_allsize; };
	char* m_buff;
	BuffBlock* m_next;
	int16_t m_ref;

	int8_t readInt8() { return read<int8_t>(); }
	uint8_t readUint8() { return read<uint8_t>(); }
	int16_t readInt16() { return read<int16_t>(); }
	uint16_t readUint16() { return read<uint16_t>(); }
	int32_t readInt32() { return read<int32_t>(); }
	uint32_t readUint32() { return read<uint32_t>(); }
	int64_t readInt64() { return read<int64_t>(); }
	uint64_t readUint64() { return read<uint64_t>(); }

	char* readBuff(int32_t& buffsize)
	{
		buffsize = m_size - m_offect;
		char* buff = m_buff + m_offect;
		m_offect += buffsize;
		return buff;
	}

	char* readString(int32_t& res)
	{
		res = readInt32();
		if (m_offect + res > m_size)
			return NULL;
		m_offect += res;
		return m_buff + m_offect - res;
	}

	std::string readString()
	{
		auto size = 0;
		auto chr = readString(size);
		if (chr == NULL)
			return std::string();	//error
		return std::string(chr, size);
	}

	void writeInt8(const int8_t& t) { write(t); }
	void writeUInt8(const uint8_t& t) { write(t); }
	void writeInt16(const int16_t& t) { write(t); }
	void writeUInt16(const uint16_t& t) { write(t); }
	void writeInt32(const int32_t& t) { write(t); }
	void writeUInt32(const uint32_t& t) { write(t); }
	void writeInt64(const int64_t& t) { write(t); }
	void writeUInt64(const uint64_t& t) { write(t); }
	void writeString(const std::string& s)
	{
		writeInt32((int32_t)s.length());
		writeBuff(s.data(), (int32_t)s.length());
	}

	void write(const google::protobuf::Message& msg)
	{
		int32_t msize = msg.ByteSize();
		if (m_buff == NULL)
			makeRoom(msize);

		if (msize + m_offect > m_allsize)
		{
			extandSize();
			//error
			return;
		}

		msg.SerializeToArray(m_buff + m_offect, msize);

		if (m_offect == m_size)
			m_size += msize;
		m_offect += msize;
	}

	void writeProto(const google::protobuf::Message& msg)
	{
		write(msg);
	}

	void writeString(const google::protobuf::Message& msg)
	{
		writeInt32(msg.ByteSize());
		writeProto(msg);
	}

	int32_t getOffect() { return m_offect; }
	void setOffect(const int32_t& offect) { m_offect = offect; }
	int32_t getSize() { return m_size; }
	int32_t getUnReadSize() { return m_size - m_offect; }

private:

	template<typename T>
	T read()
	{
		auto ts = sizeof(T);
		if (ts + m_offect > m_size)
		{
			//error 
			return T();
		}
		T t = *(T*)(m_buff + m_offect);
		m_offect += (int32_t)ts;
		return t;
	}

	template<typename T>
	void write(const T& t)
	{
		auto ts = sizeof(T);

		if (m_buff == NULL)
			makeRoom(64);

		if (ts + m_offect > m_allsize)
		{
			extandSize();
			//error 
			return;
		}
		T* pt = (T*)(m_buff + m_offect);
		*pt = t;

		if (m_offect == m_size)
			m_size += (int32_t)ts;
		m_offect += (int32_t)ts;
	}

protected:
	int32_t m_size;		//used size
	int32_t m_allsize;
	//void* m_recylist;
	int32_t m_offect;

};

struct LOOP_EXPORT LocalBuffBlock:public BuffBlock,public LoopObject
{
	LocalBuffBlock();
	virtual void makeRoom(const int32_t& size);
	virtual void recycleMsg() override {};

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
	char* getNetBuff();
	SHARE<LocalBuffBlock> getCombinBuff();
	void getCombinBuff(BuffBlock* buff);
	BuffBlock* popBuffBlock();

	inline int32_t getLen() { return len; };

	int32_t socket;
	int32_t mid;
	BuffBlock* m_buff;
	NetMsg* m_next_data;
protected:
	int32_t len;
};

struct BroadMsg:public NetMsg
{
	std::vector<int32_t> m_socks;

	virtual void initMsg() override {
		NetMsg::initMsg();
		m_socks.clear();
	};

	virtual void recycleMsg() override;
};

struct LOOP_EXPORT NetServerMsg:public NetMsg,public LoopObject
{
	virtual void initMsg() override {
		NetMsg::initMsg();
		path.clear();
	};
	virtual void recycleMsg() override;
	ServerPath path;
	// ͨ�� LoopObject �̳�
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;
	//path ֻ����init��ʱ������
	virtual void push_front(BaseLayer* l, const char* buf, const int32_t& size);
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
	bool activeLink;	//是否主动链接
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
