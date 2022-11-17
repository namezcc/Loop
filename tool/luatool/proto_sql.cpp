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
    snprintf(_buff, _size,"INSERT INTO `lp_player`(`uid`,`pid`,`name`,`level`,`gold`) VALUES (%lld,%lld,'%s',%d,%d) ON DUPLICATE KEY UPDATE `uid` = %lld, `name` = '%s', `level` = %d, `gold` = %d;",
	pb.uid(),pb.pid(),pb.name().c_str(),pb.level(),pb.gold(),	pb.uid(),pb.name().c_str(),pb.level(),pb.gold());

    return db->Query(const_cast<const char*>(_buff));
}

bool update_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"UPDATE `lp_player` SET `uid` = %lld, `name` = '%s', `level` = %d, `gold` = %d WHERE `pid` = %lld;",
	pb.uid(),pb.name().c_str(),pb.level(),pb.gold(),	pb.pid());

    return db->Query(const_cast<const char*>(_buff));
}

bool select_lp_player(gopb::RepeatedPtrField<LPMsg::DB_player>& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _uid){
    snprintf(_buff, _size,"SELECT * FROM `lp_player` WHERE `uid` = %lld;", _uid);

    auto r = db->query(_buff);

    while (!r->eof())
    {
        auto& e = *pb.Add();
		e.set_uid(r->getInt64("uid"));
		e.set_pid(r->getInt64("pid"));
		e.set_name(r->getString("name"));
		e.set_level(r->getInt32("level"));
		e.set_gold(r->getInt32("gold"));

        r->nextRow();
    }
    return true;
}

bool select_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _pid){
    snprintf(_buff, _size,"SELECT * FROM `lp_player` WHERE `pid` = %lld;", _pid);

    auto r = db->query(_buff);

    if (!r->eof())
    {
		pb.set_uid(r->getInt64("uid"));
		pb.set_pid(r->getInt64("pid"));
		pb.set_name(r->getString("name"));
		pb.set_level(r->getInt32("level"));
		pb.set_gold(r->getInt32("gold"));

        return true;
    }else return false;
}

bool select_lp_player_relation(gopb::RepeatedPtrField<LPMsg::DB_player_relation>& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _pid){
    snprintf(_buff, _size,"SELECT * FROM `lp_player_relation` WHERE `pid` = %lld;", _pid);

    auto r = db->query(_buff);

    while (!r->eof())
    {
        auto& e = *pb.Add();
		e.set_pid(r->getInt64("pid"));
		e.set_rpid(r->getInt64("rpid"));
		e.set_name(r->getString("name"));
		e.set_level(r->getInt32("level"));
		e.set_time(r->getInt32("time"));
		e.set_type(r->getInt32("type"));

        r->nextRow();
    }
    return true;
}

bool insert_lp_player_relation(LPMsg::DB_player_relation& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"INSERT INTO `lp_player_relation`(`pid`,`rpid`,`name`,`level`,`time`,`type`) VALUES (%lld,%lld,'%s',%d,%d,%d) ON DUPLICATE KEY UPDATE `name` = '%s', `level` = %d, `time` = %d, `type` = %d;",
	pb.pid(),pb.rpid(),pb.name().c_str(),pb.level(),pb.time(),pb.type(),	pb.name().c_str(),pb.level(),pb.time(),pb.type());

    return db->Query(const_cast<const char*>(_buff));
}

bool update_lp_player_relation(LPMsg::DB_player_relation& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"UPDATE `lp_player_relation` SET `name` = '%s', `level` = %d, `time` = %d, `type` = %d WHERE `pid` = %lld AND `rpid` = %lld;",
	pb.name().c_str(),pb.level(),pb.time(),pb.type(),	pb.pid(),pb.rpid());

    return db->Query(const_cast<const char*>(_buff));
}

bool delete_lp_player_relation(LPMsg::DB_player_relation& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"DELETE FROM `lp_player_relation` WHERE `pid` = %lld AND `rpid` = %lld;",	pb.pid(),pb.rpid());

    return db->Query(const_cast<const char*>(_buff));
}

