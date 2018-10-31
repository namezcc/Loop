#include <stdint.h>
#include <stdlib.h>

#include "mpb.h"

#define LAPI 
#define bool int
#define false 0
#define true 1

enum Stream_Type
{
	S_ENCODE,
	S_DECODE,
};

typedef union
{
	int i;
	char a;
}LittleEndian;

static const LittleEndian _checkEndian = {1};

LAPI static bool IsLittleEndian()
{
	return _checkEndian.a;
}

//-------------------------

LAPI static bool WriteRawLittleEndian64(OutPutStream* stream,uint64_t v)
{
	if (stream->pos + 8 > stream->len)
		return false;
	stream->buf[stream->pos++] = ((char) v);
	stream->buf[stream->pos++] = ((char) (v >> 8));
	stream->buf[stream->pos++] = ((char)(v >> 16));
	stream->buf[stream->pos++] = ((char)(v >> 24));
	stream->buf[stream->pos++] = ((char)(v >> 32));
	stream->buf[stream->pos++] = ((char)(v >> 40));
	stream->buf[stream->pos++] = ((char)(v >> 48));
	stream->buf[stream->pos++] = ((char)(v >> 56));
	return true;
}

LAPI static bool WriteDouble(OutPutStream* str,double v)
{
	uint16_t* nv = (uint16_t*)&v;
	return WriteRawLittleEndian64(str, *nv);
}

LAPI static bool WriteFloat(OutPutStream* str, float v)
{
	if (str->pos + 4 > str->len)
		return false;

	char* nv = (char*)&v;

	str->buf[str->pos++] = nv[0];
	str->buf[str->pos++] = nv[1];
	str->buf[str->pos++] = nv[2];
	str->buf[str->pos++] = nv[3];
	return true;
}

LAPI static bool WriteRawVarint64(OutPutStream* str,uint64_t v)
{
	while (v > 127)
	{
		if (str->pos >= str->len)
			return false;
		str->buf[str->pos++] = (char)((v & 0x7F) | 0x80);
		v >>= 7;
	}
	if (str->pos >= str->len)
		return false;
	str->buf[str->pos++] = (char)v;
	return true;
}

LAPI static bool WriteUInt64(OutPutStream* str,uint64_t v)
{
	return WriteRawVarint64(str,v);
}

LAPI static bool WriteInt64(OutPutStream* str, int64_t value)
{
	return WriteRawVarint64(str,(uint64_t)value);
}

LAPI static bool WriteRawVarint32(OutPutStream* str, uint32_t v)
{
	while (v>127)
	{
		if (str->pos >= str->len)
			return false;
		str->buf[str->pos++] = (char)((v & 0x7F) | 0x80);
		v >>= 7;
	}
	if (str->pos >= str->len)
		return false;
	str->buf[str->pos++] = (char)v;
	return true;
}

LAPI static bool WriteInt32(OutPutStream* str, int value)
{
	if (value >= 0)
		return WriteRawVarint32(str,(uint32_t)value);
	else
		return WriteRawVarint64(str, (uint64_t)value);
}

LAPI static bool WriteLength(OutPutStream* str, int length)
{
	return WriteRawVarint32(str, (uint32_t)length);
}

LAPI static bool WriteString(OutPutStream* str,char* src,int len)
{
	if (!WriteLength(str, len))
		return false;

	if (str->pos + len > str->len)
		return false;

	for (size_t i = 0; i < len; i++)
	{
		str->buf[str->pos + i] = src[i];
	}
	str->pos += len;
	return true;
}

LAPI static bool WriteUInt32(OutPutStream* str, uint32_t v)
{
	return WriteRawVarint32(str,v);
}

LAPI static bool WriteTag(OutPutStream* str, uint32_t tag)
{
	return WriteRawVarint32(str,tag);
}
//-----------------------------------

const static int LittleEndian64Size = 8;
const static int LittleEndian32Size = 4;

LAPI static uint64_t EncodeZigZag64(int64_t n)
{
	return (uint64_t)((n << 1) ^ (n >> 63));
}

LAPI static uint32_t EncodeZigZag32(int n)
{
	return (uint32_t)((n << 1) ^ (n >> 31));
}

LAPI static int ComputeDouble(double v)
{
	return LittleEndian64Size;
}

LAPI static int ComputeFloat(float v)
{
	return LittleEndian32Size;
}

LAPI static int ComputeRawVarint32Size(uint32_t value)
{
	if ((value & (0xffffffff << 7)) == 0)
	{
		return 1;
	}
	if ((value & (0xffffffff << 14)) == 0)
	{
		return 2;
	}
	if ((value & (0xffffffff << 21)) == 0)
	{
		return 3;
	}
	if ((value & (0xffffffff << 28)) == 0)
	{
		return 4;
	}
	return 5;
}

LAPI static int ComputeRawVarint64Size(uint64_t value)
{
	if ((value & (0xffffffffffffffffL << 7)) == 0)
	{
		return 1;
	}
	if ((value & (0xffffffffffffffffL << 14)) == 0)
	{
		return 2;
	}
	if ((value & (0xffffffffffffffffL << 21)) == 0)
	{
		return 3;
	}
	if ((value & (0xffffffffffffffffL << 28)) == 0)
	{
		return 4;
	}
	if ((value & (0xffffffffffffffffL << 35)) == 0)
	{
		return 5;
	}
	if ((value & (0xffffffffffffffffL << 42)) == 0)
	{
		return 6;
	}
	if ((value & (0xffffffffffffffffL << 49)) == 0)
	{
		return 7;
	}
	if ((value & (0xffffffffffffffffL << 56)) == 0)
	{
		return 8;
	}
	if ((value & (0xffffffffffffffffL << 63)) == 0)
	{
		return 9;
	}
	return 10;
}

LAPI static int ComputeInt32Size(int value)
{
	if (value >= 0)
	{
		return ComputeRawVarint32Size((uint32_t)value);
	}
	else
	{
		// Must sign-extend.
		return 10;
	}
}

LAPI static int ComputeInt64Size(uint64_t value)
{
	return ComputeRawVarint64Size(value);
}

LAPI static int ComputeUInt32Size(uint32_t value)
{
	return ComputeRawVarint32Size(value);
}

LAPI static int ComputeUInt64(uint64_t v)
{
	return ComputeRawVarint64Size(v);
}

LAPI static int ComputeSInt64Size(uint64_t value)
{
	return ComputeRawVarint64Size(EncodeZigZag64(value));
}

LAPI static int ComputeSInt32Size(uint32_t value)
{
	return ComputeRawVarint32Size(EncodeZigZag32(value));
}

LAPI static int ComputeBoolSize(bool value)
{
	return 1;
}

LAPI static int ComputeLengthSize(int length)
{
	return ComputeRawVarint32Size((uint32_t)length);
}

LAPI static int ComputeStringSize(int slen)
{
	return ComputeLengthSize(slen) + slen;
}
//---------------------------------------

LAPI static int ReadRawByte(OutPutStream* str,char* res)
{
	if (str->pos >= str->curLimit)
	{
		return -1;
	}
	*res = str->buf[str->pos++];
	return 0;
}

LAPI static int ReadRawVarint32(OutPutStream* str, uint32_t* res)
{
	int len = str->curLimit - str->pos;
	if (len > 5)
		len = 5;
	int shift = 0;
	uint32_t tmp = 0;
	*res = 0;
	for (size_t i = 0; i < len; i++)
	{
		int b = str->buf[str->pos++];
		tmp |= (b & 0x7F) << shift;
		if ((b & 0x80) == 0)
		{
      *res = tmp;
      return 0;
		}
		shift += 7;
	}
	*res = tmp;
	return -1;
}

LAPI static int ReadTag(OutPutStream* str,uint32_t* res)
{
	if (str->hashNextTag)
	{
		str->lastTag = str->nextTag;
		str->hashNextTag = false;
		*res = str->lastTag;
		return 0;
	}

	if(str->pos<str->curLimit)
	{
		int r = ReadRawVarint32(str, res);
		str->lastTag = *res;
		return r;
	}
	else
	{
		str->lastTag = *res = 0;
		return 0;
	}
	return 0;
}

LAPI static int ReadRawLittleEndian32(OutPutStream* str,uint32_t* res)
{
	if (str->pos + 4 > str->curLimit)
		return -1;
  uint32_t tmp;
	tmp = str->buf[str->pos++];
	tmp |= (str->buf[str->pos++]) << 8;
	tmp |= (str->buf[str->pos++]) << 16;
	tmp |= (str->buf[str->pos++]) << 24;
	*res = tmp;
	return 0;
}

/// <summary>
/// Reads a 64-bit little-endian integer from the stream.
/// </summary>
LAPI static int ReadRawLittleEndian64(OutPutStream* str, uint64_t* res)
{
	if (str->pos + 8 > str->curLimit)
		return -1;
  uint64_t tmp;
	tmp = str->buf[str->pos++];
	tmp |= (uint64_t)(str->buf[str->pos++]) << 8;
	tmp |= (uint64_t)(str->buf[str->pos++]) << 16;
	tmp |= (uint64_t)(str->buf[str->pos++]) << 24;
	tmp |= (uint64_t)(str->buf[str->pos++]) << 32;
	tmp |= (uint64_t)(str->buf[str->pos++]) << 40;
	tmp |= (uint64_t)(str->buf[str->pos++]) << 48;
	tmp |= (uint64_t)(str->buf[str->pos++]) << 56;
	*res = tmp;
	return 0;
}

LAPI static int ReadDouble(OutPutStream* str, double* res)
{
	return ReadRawLittleEndian64(str,(uint64_t*)res);
}

LAPI static int ReadFloat(OutPutStream* str, float* res)
{
	return ReadRawLittleEndian32(str, (uint32_t*)res);
}

LAPI static int ReadRawVarint64(OutPutStream* str,uint64_t* res)
{
	int len = str->curLimit - str->pos;
	if (len > 10)
		len = 10;
	int shift = 0;
	*res = 0;
	for (size_t i = 0; i < len; i++)
	{
		char b = str->buf[str->pos++];
		*res |= (uint64_t)(b & 0x7F) << shift;
		if ((b & 0x80) == 0)
			return 0;
		shift += 7;
	}
	return -1;
}

LAPI static int ReadUInt64(OutPutStream* str,uint64_t* res)
{
	return ReadRawVarint64(str, res);
}

LAPI static int ReadInt64(OutPutStream* str, int64_t* res)
{
	return ReadRawVarint64(str, (uint64_t*)res);
}

LAPI static int ReadUInt32(OutPutStream* str, uint32_t* res)
{
	return ReadRawVarint32(str, res);
}

LAPI static int ReadInt32(OutPutStream* str, int32_t* res)
{
	return ReadRawVarint32(str, (uint32_t*)res);
}

LAPI static int ReadLength(OutPutStream* str,int* len)
{
	return ReadRawVarint32(str, (uint32_t*)len);
}

LAPI static int ReadString(OutPutStream* str,char** res,int* len)
{
	int err = ReadLength(str, len);
	if (err < 0)
		return -1;
	if (*len == 0)
		return -1;
	if (str->pos + *len > str->curLimit)
		return -1;
	*res = (char*)malloc(*len);
	for (size_t i = 0; i < *len; i++)
	{
		(*res)[i] = str->buf[str->pos + i];
	}
	str->pos += *len;
	return 0;
}
//------------------------------------

static int pb_sizeTag(lua_State* L)
{
	int v = lua_tointeger(L, 1);
	int s = ComputeInt32Size(v);
	lua_pushinteger(L, s);
	return 1;
}

static int pb_sizeInt(lua_State* L)
{
	int v = lua_tointeger(L, 1);
	int s = ComputeInt32Size(v);
	lua_settop(L, 0);
	lua_pushinteger(L, s);
	return 1;
}

static int pb_sizeUInt(lua_State* L)
{
	uint32_t v = lua_tointeger(L, 1);
	int s = ComputeUInt32Size(v);
	lua_settop(L, 0);
	lua_pushinteger(L, s);
	return 1;
}

static int pb_sizeInt64(lua_State* L)
{
	int64_t v = lua_tointeger(L, 1);
	int s = ComputeInt64Size(v);
	lua_settop(L, 0);
	lua_pushinteger(L, s);
	return 1;
}

static int pb_sizeUInt64(lua_State* L)
{
	uint64_t v = lua_tointeger(L, 1);
	int s = ComputeUInt64(v);
	lua_settop(L, 0);
	lua_pushinteger(L, s);
	return 1;
}

static int pb_sizeFloat(lua_State* L)
{
	float v = lua_tonumber(L, 1);
	int s = ComputeFloat(v);
	lua_pushinteger(L, s);
	return 1;
}

static int pb_sizeDouble(lua_State* L)
{
	double v = lua_tonumber(L, 1);
	int s = ComputeDouble(v);
	lua_pushinteger(L, s);
	return 1;
}

static int pb_sizeString(lua_State* L)
{
	size_t len;
	const char* v = lua_tolstring(L, 1,&len);
	int s = ComputeStringSize(len);
	lua_pushinteger(L, s);
	return 1;
}

static OutPutStream* pb_getStream(lua_State* L)
{
	void* ud = luaL_checkudata(L, 1,"OutPutStream");
	if (ud == NULL)
	{
		luaL_error(L, "pb Stream is nil");
		return NULL;
	}
	return *(OutPutStream**)ud;
}

static int pb_clearStream(lua_State* L)
{
	OutPutStream** ud = (OutPutStream**)luaL_checkudata(L, 1,"OutPutStream");
	if (ud == NULL)
	{
		luaL_error(L, "pb Stream is nil");
		return 0;
	}
	//OutPutStream** 
	OutPutStream* s = *ud;
	if (s)
	{
		if (s->buf)
		{
			if(s->type==S_ENCODE)
				free(s->buf);
			s->buf = NULL;
		}
		free(s); //
		*ud = NULL;
	}
	return 0;
}

static int pb_toStringByte(lua_State* L)
{
	OutPutStream* s = pb_getStream(L);
	if (s)
	{
		lua_pushlstring(L, s->buf, s->len);
		return 1;
	}
	return 0;
}

static int pb_fromString(lua_State* L)
{
	size_t len;
	const char* buf = lua_tolstring(L, 1, &len);

	OutPutStream** udata = (OutPutStream**)lua_newuserdata(L, sizeof(OutPutStream*));
	*udata = (OutPutStream*)malloc(sizeof(OutPutStream));
	luaL_getmetatable(L, "OutPutStream");
	lua_setmetatable(L, -2);
	OutPutStream* pbstream = *udata;
	pbstream->buf = (char*)buf;
	pbstream->pos = 0;
	pbstream->len = len;
	pbstream->curLimit = len;
	pbstream->lastTag = 0;
	pbstream->nextTag = 0;
	pbstream->hashNextTag = false;
	pbstream->type = S_DECODE;
	return 1;
}

static int pb_newStream(lua_State* L)
{
	int len = lua_tointeger(L, 1);
	if (len == 0)
		return 0;

	OutPutStream** udata = (OutPutStream**)lua_newuserdata(L, sizeof(OutPutStream*));
	*udata = (OutPutStream*)malloc(sizeof(OutPutStream));
	luaL_getmetatable(L, "OutPutStream");
	lua_setmetatable(L, -2);
	OutPutStream* pbstream = *udata;
	pbstream->buf = (char*)malloc(len);
	pbstream->pos = 0;
	pbstream->len = len;
	pbstream->curLimit = len;
	pbstream->lastTag = 0;
	pbstream->nextTag = 0;
	pbstream->hashNextTag = false;
	pbstream->type = S_ENCODE;
	//lua_pushlightuserdata(L, pbstream);
	return 1;
}

static int pb_wInt(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int v = lua_tointeger(L, 2);
	WriteInt32(pbstream, v);
	return 0;
}

static int pb_wInt64(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int64_t v = lua_tointeger(L, 2);
	WriteInt64(pbstream, v);
	return 0;
}

static int pb_wUint(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	uint32_t v = lua_tointeger(L, 2);
	WriteUInt32(pbstream,v);
	return 0;
}

static int pb_wUint64(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int64_t v = lua_tointeger(L, 2);
	WriteUInt64(pbstream, v);
	return 0;
}

static int pb_wDouble(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	double v = lua_tonumber(L, 2);
	WriteDouble(pbstream, v);
	return 0;
}

static int pb_wFloat(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	float v = lua_tonumber(L, 2);
	WriteFloat(pbstream, v);
	return 0;
}

static int pb_wBool(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int v = lua_toboolean(L, 2);
	WriteInt32(pbstream, v);
	return 0;
}

static int pb_wString(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	size_t len;
	const char* v = lua_tolstring(L, 2,&len);
	WriteString(pbstream,(char*)v,len);
	return 0;
}

static int pb_rTag(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	uint32_t v;
	ReadTag(pbstream, &v);
	lua_pushinteger(L, v);
	return 1;
}

static int pb_rInt(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int v;
	ReadInt32(pbstream, &v);
	lua_pushinteger(L, v);
	return 1;
}

static int pb_rInt64(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int64_t v;
	ReadInt64(pbstream, &v);
	lua_pushinteger(L, v);
	return 1;
}

static int pb_rUint(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	uint32_t v;
	ReadUInt32(pbstream, &v);
	lua_pushinteger(L, v);
	return 1;
}

static int pb_rUint64(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	uint64_t v;
	ReadUInt64(pbstream, &v);
	lua_pushinteger(L, v);
	return 1;
}

static int pb_rDouble(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	double v;
	ReadDouble(pbstream, &v);
	lua_pushnumber(L, v);
	return 1;
}

static int pb_rFloat(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	float v;
	ReadFloat(pbstream, &v);
	lua_pushnumber(L, v);
	return 1;
}

static int pb_rBool(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int v;
	ReadInt32(pbstream, &v);
	lua_pushboolean(L, v);
	return 1;
}

static int pb_rString(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int v;
	char* s=NULL;
	ReadString(pbstream, &s,&v);
	if (s)
	{
		lua_pushlstring(L, s, v);
		free(s);
		return 1;
	}
	return 0;
}

static int PeekTag(OutPutStream* str)
{
	if (str->hashNextTag)
	{
		return str->nextTag;
	}
	uint32_t tag;
	ReadTag(str, &tag);
	int saveLast = str->lastTag;
	str->nextTag = tag;
	str->hashNextTag = true;
	str->lastTag = saveLast;
	return tag;
}

static int pb_MaybeConsumeTag(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int tag = lua_tointeger(L, 2);

	if (PeekTag(pbstream) == tag)
	{
		pbstream->hashNextTag = false;
		lua_pushboolean(L, 1);
	}else
		lua_pushboolean(L, 0);
	return 1;
}

static int pb_PushLimit(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int limit = lua_tointeger(L, 2);
	int oldlimit = pbstream->curLimit;
	limit += pbstream->pos;
	if (limit > oldlimit)
		return 0;

	pbstream->curLimit = limit;
	lua_pushinteger(L, oldlimit);
	return 1;
}

static int pb_IsEndLimit(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;

	if (pbstream->pos >= pbstream->curLimit)
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);
	return 1;
}

static int pb_PopLimit(lua_State* L)
{
	OutPutStream* pbstream = pb_getStream(L);
	if (!pbstream)
		return 0;
	int limit = lua_tointeger(L, 2);
	if (limit > pbstream->curLimit && limit <= pbstream->len)
		pbstream->curLimit = limit;
	return 0;
}
//------------------------------------------

LUALIB_API int luaopen_mpb(lua_State * L)
{
	luaL_Reg reg[] = {
		{"clearStream",pb_clearStream},
		{ "newStream",pb_newStream },
		{ "sizeInt",pb_sizeInt },
		{ "sizeUInt",pb_sizeUInt },
		{ "sizeInt64",pb_sizeInt64 },
		{ "sizeUInt64",pb_sizeUInt64 },
		{ "sizeFloat",pb_sizeFloat },
		{ "sizeDouble",pb_sizeDouble },
		{ "sizeString",pb_sizeString },
		{ "sizeTag",pb_sizeTag },
		{ "wBool",pb_wBool },
		{ "wDouble",pb_wDouble },
		{ "wFloat",pb_wFloat },
		{ "wInt",pb_wInt},
		{ "wInt64",pb_wInt64 },
		{ "wString",pb_wString},
		{ "wUInt",pb_wUint },
		{ "wUInt64",pb_wUint64 },
		{ "rBool",pb_rBool },
		{ "rDouble",pb_rDouble },
		{ "rFloat",pb_rFloat },
		{ "rInt",pb_rInt },
		{ "rInt64",pb_rInt64 },
		{ "rString",pb_rString },
		{ "rUInt",pb_rUint },
		{ "rUInt64",pb_rUint64 },
		{ "rTag",pb_rTag },
		{ "MaybeConsumeTag",pb_MaybeConsumeTag },
		{ "PushLismit",pb_PushLimit },
		{ "IsEndLimit",pb_IsEndLimit },
		{ "PopLimit",pb_PopLimit },
		{ "toStringbyte",pb_toStringByte },
		{ "fromString",pb_fromString },
		{NULL,NULL},
	};

	luaL_Reg regmeta[] = {
		{"__gc",pb_clearStream },
		{ NULL,NULL },
	};

	luaL_newmetatable(L, "OutPutStream");
	luaL_setfuncs(L, regmeta,0);

	//luaL_newlib(L, reg);
	luaL_register(L, "mpb", reg);
	return 1;
}