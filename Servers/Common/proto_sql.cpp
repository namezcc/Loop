//文件由配置生成,请执行server/bin/tool/genProtoDb.bat 在proto_db_conf.lua里配置
#include "protoPB/server/dbdata.pb.h"
#include "proto_sql.h"
#include "MysqlModule.h"

bool update_db_player_num_info(LPMsg::DB_player_num_info& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"UPDATE `db_player_num_info` SET `num` = %d, `maxnum` = %d, `hostid` = %d WHERE `dbid` = %d;",
	pb.num(),pb.maxnum(),pb.hostid(),	pb.dbid());

    return db->Query(const_cast<const char*>(_buff));
}

bool select_db_player_num_info(gopb::RepeatedPtrField<LPMsg::DB_player_num_info>& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"SELECT * FROM `db_player_num_info`;");

    auto r = db->query(_buff);

    while (!r->eof())
    {
        auto& e = *pb.Add();
		e.set_dbid(r->getInt32("dbid"));
		e.set_num(r->getInt32("num"));
		e.set_maxnum(r->getInt32("maxnum"));
		e.set_hostid(r->getInt32("hostid"));

        r->nextRow();
    }
    return true;
}

bool insert_account(LPMsg::DB_account& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"INSERT INTO `account`(`platform_uid`,`game_uid`,`create_time`) VALUES ('%s',%lld,%d);",
	pb.platform_uid().c_str(),pb.game_uid(),pb.create_time());

    return db->Query(const_cast<const char*>(_buff));
}

bool select_account(gopb::RepeatedPtrField<LPMsg::DB_account>& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"SELECT * FROM `account`;");

    auto r = db->query(_buff);

    while (!r->eof())
    {
        auto& e = *pb.Add();
		e.set_platform_uid(r->getString("platform_uid"));
		e.set_game_uid(r->getInt64("game_uid"));
		e.set_create_time(r->getInt32("create_time"));

        r->nextRow();
    }
    return true;
}

bool insert_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"INSERT INTO `lp_player`(`cid`,`uid`,`name`,`level`,`gold`,`login_time`,`logout_time`,`create_time`) VALUES (%d,%d,'%s',%d,%d,%d,%d,%d);",
	pb.cid(),pb.uid(),pb.name().c_str(),pb.level(),pb.gold(),pb.login_time(),pb.logout_time(),pb.create_time());

    return db->Query(const_cast<const char*>(_buff));
}

bool update_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"UPDATE `lp_player` SET `cid` = %d, `name` = '%s', `level` = %d, `gold` = %d, `login_time` = %d, `logout_time` = %d, `create_time` = %d WHERE `uid` = %d;",
	pb.cid(),pb.name().c_str(),pb.level(),pb.gold(),pb.login_time(),pb.logout_time(),pb.create_time(),	pb.uid());

    return db->Query(const_cast<const char*>(_buff));
}

bool select_lp_player(gopb::RepeatedPtrField<LPMsg::DB_player>& pb,MysqlModule* db,char* _buff,size_t _size,int32_t _uid){
    snprintf(_buff, _size,"SELECT * FROM `lp_player` WHERE `uid` = %d;", _uid);

    auto r = db->query(_buff);

    while (!r->eof())
    {
        auto& e = *pb.Add();
		e.set_cid(r->getInt32("cid"));
		e.set_uid(r->getInt32("uid"));
		e.set_name(r->getString("name"));
		e.set_level(r->getInt32("level"));
		e.set_gold(r->getInt32("gold"));
		e.set_login_time(r->getInt32("login_time"));
		e.set_logout_time(r->getInt32("logout_time"));
		e.set_create_time(r->getInt32("create_time"));

        r->nextRow();
    }
    return true;
}

bool select_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size,int32_t _uid){
    snprintf(_buff, _size,"SELECT * FROM `lp_player` WHERE `uid` = %d;", _uid);

    auto r = db->query(_buff);

    if (!r->eof())
    {
		pb.set_cid(r->getInt32("cid"));
		pb.set_uid(r->getInt32("uid"));
		pb.set_name(r->getString("name"));
		pb.set_level(r->getInt32("level"));
		pb.set_gold(r->getInt32("gold"));
		pb.set_login_time(r->getInt32("login_time"));
		pb.set_logout_time(r->getInt32("logout_time"));
		pb.set_create_time(r->getInt32("create_time"));

        return true;
    }else return false;
}

