#ifndef PROTO_SQL_H
#define PROTO_SQL_H

namespace gopb = google::protobuf;
class MysqlModule;

bool update_db_player_num_info(LPMsg::DB_player_num_info& pb,MysqlModule* db,char* _buff,size_t _size);
bool select_db_player_num_info(gopb::RepeatedPtrField<LPMsg::DB_player_num_info>& pb,MysqlModule* db,char* _buff,size_t _size);

bool insert_account(LPMsg::DB_account& pb,MysqlModule* db,char* _buff,size_t _size);
bool select_account(gopb::RepeatedPtrField<LPMsg::DB_account>& pb,MysqlModule* db,char* _buff,size_t _size);

bool insert_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size);
bool update_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size);
bool select_lp_player(gopb::RepeatedPtrField<LPMsg::DB_player>& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _uid);
bool select_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _pid);

bool select_lp_player_relation(gopb::RepeatedPtrField<LPMsg::DB_player_relation>& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _pid);
bool insert_lp_player_relation(LPMsg::DB_player_relation& pb,MysqlModule* db,char* _buff,size_t _size);
bool update_lp_player_relation(LPMsg::DB_player_relation& pb,MysqlModule* db,char* _buff,size_t _size);
bool delete_lp_player_relation(LPMsg::DB_player_relation& pb,MysqlModule* db,char* _buff,size_t _size);

#endif