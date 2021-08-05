#ifndef PROTO_DEFINE
#define PROTO_DEFINE

#include "BaseMsg.h"

#define HEAD_FLAG 0x55555555
struct Head
{
	uint32_t size;
	int mid;
	int flag;
};

typedef struct MsgHead :public Head
{
	enum
	{
		HEAD_SIZE = PACK_HEAD_SIZE,
	};

	static void Encode(BuffBlock& buf, const int& mid, int len)
	{
		len += HEAD_SIZE;
		int flag = (len & mid) ^ HEAD_FLAG;

		buf.writeInt32(len);
		buf.writeInt32(mid);
		buf.writeInt32(flag);
	}

	static bool Decode(MsgHead& mh, char* buf)
	{
		mh.size = PB::GetInt(buf);
		mh.mid = PB::GetInt(buf + 4);
		mh.flag = PB::GetInt(buf + 8);
		return true;//mh.flag == ((mh.size & mh.mid) ^ HEAD_FLAG);
	}
}MsgHead;

enum ProtoType
{
	PT_MY_PROTO,
	PT_HTTP,
};


#define MAX_CLIENT_CONN 20000
#define CHECK_SOCK_INDEX(sock) (sock >= 0 && sock < MAX_CLIENT_CONN)

#endif
