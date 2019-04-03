#ifndef PROTO_DEFINE
#define PROTO_DEFINE

#include "BaseMsg.h"

#define HEAD_FLAG 0x55555555
struct Head
{
	int size;
	int mid;
	int flag;
};

typedef struct MsgHead :public Head
{
	enum
	{
		HEAD_SIZE = PACK_HEAD_SIZE,
	};

	static void Encode(char* buf, const int& mid, int len)
	{
		len += HEAD_SIZE;
		int flag = (len & mid) ^ HEAD_FLAG;

		PB::WriteInt(buf, len);
		PB::WriteInt(buf + 4, mid);
		PB::WriteInt(buf + 8, flag);
	}

	static bool Decode(MsgHead& mh, char* buf)
	{
		mh.size = PB::GetInt(buf);
		mh.mid = PB::GetInt(buf + 4);
		mh.flag = PB::GetInt(buf + 8);
		return mh.flag == ((mh.size & mh.mid) ^ HEAD_FLAG);
	}
}MsgHead;

enum ProtoType
{
	PT_MY_PROTO,
	PT_HTTP,
};

#endif
