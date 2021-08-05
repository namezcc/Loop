#ifndef PROTO_SQL_H
#define PROTO_SQL_H

namespace gopb = google::protobuf;
class MysqlModule;

bool insert_account(LPMsg::DB_account& pb,MysqlModule* db,char* _buff,size_t _size);
bool update_account(LPMsg::DB_account& pb,MysqlModule* db,char* _buff,size_t _size);
bool select_account(LPMsg::DB_account& pb,MysqlModule* db,char* _buff,size_t _size,std::string _platform_uid);

bool insert_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size);
bool update_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size);
bool select_lp_player(gopb::RepeatedPtrField<LPMsg::DB_player>& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _uid);
bool select_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _uid,int32_t _rid);

#endif