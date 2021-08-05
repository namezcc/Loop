#include "protoPB/server/dbdata.pb.h"
#include "proto_sql.h"
#include "MysqlModule.h"

bool insert_account(LPMsg::DB_account& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"INSERT INTO `account`(`platform_uid`,`hash_index`,`game_uid`,`dbindex`) VALUES ('%s',%d,%lld,%d) ON DUPLICATE KEY UPDATE `hash_index`=%d, `game_uid`=%lld, `dbindex`=%d;",
	pb.platform_uid().c_str(),pb.hash_index(),pb.game_uid(),pb.dbindex(),	pb.hash_index(),pb.game_uid(),pb.dbindex());

    return db->Query(_buff);
}

bool update_account(LPMsg::DB_account& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"UPDATE `account` SET `hash_index`=%d, `game_uid`=%lld, `dbindex`=%d WHERE `platform_uid`='%s';",
	pb.hash_index(),pb.game_uid(),pb.dbindex(),	pb.platform_uid().c_str());

    return db->Query(_buff);
}

bool select_account(LPMsg::DB_account& pb,MysqlModule* db,char* _buff,size_t _size,std::string _platform_uid){
    snprintf(_buff, _size,"SELECT * FROM `account` WHERE `platform_uid`='%s';", _platform_uid.c_str());

    auto& r = db->query(_buff);

    if (!r->eof())
    {
		pb.set_platform_uid(r->getString("platform_uid"));
		pb.set_hash_index(r->getInt32("hash_index"));
		pb.set_game_uid(r->getInt64("game_uid"));
		pb.set_dbindex(r->getInt32("dbindex"));

        return true;
    }else return false;
}

bool insert_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"INSERT INTO `lp_player`(`uid`,`rid`,`name`,`level`,`gold`) VALUES (%lld,%d,'%s',%d,%d) ON DUPLICATE KEY UPDATE `name`='%s', `level`=%d, `gold`=%d;",
	pb.uid(),pb.rid(),pb.name().c_str(),pb.level(),pb.gold(),	pb.name().c_str(),pb.level(),pb.gold());

    return db->Query(_buff);
}

bool update_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size){
    snprintf(_buff, _size,"UPDATE `lp_player` SET `name`='%s', `level`=%d, `gold`=%d WHERE `uid`=%lld AND `rid`=%d;",
	pb.name().c_str(),pb.level(),pb.gold(),	pb.uid(),pb.rid());

    return db->Query(_buff);
}

bool select_lp_player(gopb::RepeatedPtrField<LPMsg::DB_player>& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _uid){
    snprintf(_buff, _size,"SELECT * FROM `lp_player` WHERE `uid`=%lld;", _uid);

    auto& r = db->query(_buff);

    while (!r->eof())
    {
        auto& e = *pb.Add();
		e.set_uid(r->getInt64("uid"));
		e.set_rid(r->getInt32("rid"));
		e.set_name(r->getString("name"));
		e.set_level(r->getInt32("level"));
		e.set_gold(r->getInt32("gold"));

        r->nextRow();
    }
    return true;
}

bool select_lp_player(LPMsg::DB_player& pb,MysqlModule* db,char* _buff,size_t _size,int64_t _uid,int32_t _rid){
    snprintf(_buff, _size,"SELECT * FROM `lp_player` WHERE `uid`=%lld AND `rid`=%d;", _uid, _rid);

    auto& r = db->query(_buff);

    if (!r->eof())
    {
		pb.set_uid(r->getInt64("uid"));
		pb.set_rid(r->getInt32("rid"));
		pb.set_name(r->getString("name"));
		pb.set_level(r->getInt32("level"));
		pb.set_gold(r->getInt32("gold"));

        return true;
    }else return false;
}

