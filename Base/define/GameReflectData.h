#ifndef GAME_REFLECT_DATA
#define GAME_REFLECT_DATA

#include "Reflection.h"

struct AccoutInfo
{
	int64_t id;
	string name;
	string pass;
	string createTime;
};

REFLECT(AccoutInfo,id,name,pass,createTime)
TABLE_DESC_BEGAN(AccoutInfo,id)
FIELD_DESC(id, SQL_BIGINT,20,false,"",false,"'' AUTO_INCREMENT")
FIELD_DESC(name,SQL_VARCHAR,64,false,"",true,"")
FIELD_DESC(pass, SQL_VARCHAR, 64, false, "", false, "")
FIELD_DESC(createTime, SQL_TIMESTAMP, 0, false, "CURRENT_TIMESTAMP", false, "")
TABLE_DESC_END

#endif // !GAME_REFLECT_DATA