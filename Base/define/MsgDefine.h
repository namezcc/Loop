﻿#ifndef MSG_DEFINE_H
#define MSG_DEFINE_H
#include <string>
#include <sstream>
#include <vector>

enum MSG_FROM_LAYER
{
	L_BEGAN = 0,

	L_REQUEST_CORO_MSG,
	L_RESPONSE_CORO_MSG,

	L_SOCKET_CONNET,
	L_SOCKET_CLOSE,
	L_UDP_SOCKET_CONNECT,
	L_UDP_SOCKET_CLOSE,
	L_SOCKET_SEND_DATA,
	L_SOCKET_BROAD_DATA,

	L_TO_CONNET_SERVER,
	L_SERVER_CONNECTED,

	L_LOG_INFO,

	L_MYSQL_MSG,
	L_MYSQL_CORO_MSG,
	//http
	L_HL_GET_MACHINE_LIST,	//��ȡconsole��

	L_UPDATE_TABLE_GROUP,

	L_CORO_1_TEST_1,
	L_CORO_1_TEST_2,
	L_CORO_2_TEST_1,
	L_CORO_2_TEST_2,

	L_END_BEFOR,
	L_END = 300,
};

enum MSG_FROM_NET
{
	N_BEGAN = MSG_FROM_LAYER::L_END,

	N_REQUEST_CORO_MSG,
	N_RESPONSE_CORO_MSG,

	N_REGISTE_SERVER,
	N_TRANS_SERVER_MSG,
	N_RECV_HTTP_MSG,
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> common ------------------

	N_MYSQL_MSG,
	N_MYSQL_CORO_MSG,

	N_FORWARD_DB_PROXY,
	N_FORWARD_DB_PROXY_GROUP,
	N_TRANS_TEST,

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< common -----------------
	
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> server msg ----------------------
	
	N_SEND_SERVER_STATE,
	N_ALL_SERVER_INFO,
	N_CONN_SERVER_INFO,
	N_GET_LINK_SERVER_INFO,

	N_UPDATE_TABLE_GROUP,
	N_ADD_TABLE_GROUP,
	N_GET_ACCOUNT_INFO,

	N_GET_MYSQL_GROUP,	//��ȡmysql ���ݿ� ��id

	N_ML_CREATE_ACCOUNT,	//mysql -> login
	N_ML_GET_ACCOUNT,

	N_CORO_TEST_1,

	N_LOGIN_LOCK,
	N_LOGIN_UNLOCK,

	N_GET_DBINDEX,			//获取分配的数据库index
	N_ADD_DBINDEX_NUM,		//增加玩家数量

	N_ROOM_STATE,
	N_REQ_ROOM_LIST,
	N_ACK_ROOM_LIST,

	N_ROOM_READY_TAKE_PLAYER,

	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< server msg ----------------------

	
	N_END_BEFOR = 1000,
	N_END = 5000,
};
//�ͻ����������֮����Ϣid > 10000
enum BASE_EVENT
{
	E_SOCKEK_CONNECT,
	E_SOCKET_CLOSE,
	E_UDP_SOCKET_CONNECT,
	E_UDP_SOCKET_CLOSE,

	E_SERVER_CONNECT,
	E_SERVER_SOCKET_CLOSE,
	E_SERVER_CLOSE,

	E_CLIENT_HTTP_CLOSE,

	E_PHP_CGI_CLOSE,

	E_BASE_EVENT_END,
};

#endif