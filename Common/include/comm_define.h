/**
* \file comm_define.h
* \brief 公共头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author zJun
*/

#ifndef __COMM_DEFINE_H__
#define __COMM_DEFINE_H__
#include <map>
#include "typedef.h"
#include <string>

#define  MAX_ROBOT_ROLE_ID          (0x1FFFFFFF)        //分配给机器人的最大角色ID
#define  MAX_SERVER_REGISTER_CNT    (50)                //一个服务最大客户端注册数
#define  MAX_NET_WORKER_THREADS     (20)                //网络工作线程数

/**
* \brief 宏定义
*/

//连接时间定义
enum
{
    CONN_KEEP_ALIVE_TIME    = (1*30*1000),                          // 连接心跳时间(毫秒)
    CONN_RECONNECT_TIME     = (2*1000),                             // 定时重连时间(毫秒)
    CONN_DEFAULT_TIMEOUT    = (10+2*(CONN_KEEP_ALIVE_TIME/1000)),   // 连接空闲超时断开时间(秒) -- [默认超时时间,个别程序可单独配置]
};


//玩家类型
enum PLAYER_TYPE
{   
    COMMON_PLAYER       = 0,    //普通玩家
    ROBOT_PLAYER        = 1,    //机器人玩家
    TPA_PLAYER          = 2,    //第三方玩家
};

//加入房间的方式
enum JOIN_ROOM_TYPE
{
    JOIN_ROOM_TYPE_COMMON       = 1,    //普通类型
    JOIN_ROOM_TYPE_ROBOT        = 2,    //机器人
    JOIN_ROOM_TYPE_TPA_PLAYER   = 3,    //第三方平台方式
};

//游戏类型
enum GAME_TYPE
{
    GAME_TYPE_LEDUC             = 0,
    GAME_TYPE_LIMIT_2P          = 1,
    GAME_TYPE_NOLIMIT_2P        = 2,    //当前支持
    GAME_TYPE_MAX,
};

//离开房间的类型
enum {
    LEAVE_ROOM_TYPE_RESPONSE    = 0,    //主动请求的应答   
    LEAVE_ROOM_TYPE_FORCE       = 1,    //错误强制退出
    LEAVE_ROOM_TYPE_TIMEOUT     = 2,    //超时强制退出
    LEAVE_ROOM_TYPE_ROBOT       = 3,    //机器人异常退出
    LEAVE_ROOM_TYPE_DATA        = 4,    //数据异常退出
};

enum {
    MATCH_DISABLE       = 0,    //比赛模式关闭
    MATCH_ENABLE        = 1,    //比赛模式打开
};

enum 
{
    LOGIN_TYPE_GUEST    = 1,    //游客登录
    LOGIN_TYPE_REGISTER = 2,    //注册用户登录
};

enum
{
    TERMINAL_TYPE_CLIENT    = 1,    //客户端
    TERMINAL_TYPE_WEB       = 2,    //Web
    TERMINAL_TYPE_H5        = 3,    //H5
    TERMINAL_TYPE_TPA       = 4,    //第三方
};

enum 
{
    PLAYER_UNREADY_STATE   = 0,
    PLAYER_READY_STATE     = 1,
};

#define SWITCH_DISABLE  (0)         //关闭功能开关
#define SWITCH_ENABLE   (1)         //打开功能开关


//Role属性
//const std::string UID = "uid";
const std::string ROLE_ID           = "roleid";
const std::string ROLE_USERID       = "userid";
const std::string ROLE_NAME         = "nickname";
const std::string ROLE_TYPE         = "type";
const std::string ROLE_IMG          = "img";
const std::string ROLE_ROOM_ID      = "room_id";
const std::string ROLE_VIEW_ROOM_ID = "view_room_id";
const std::string ROLE_REG          = "region";
const std::string ROLE_SVR          = "svr";
const std::string ROLE_SEX          = "sex";
const std::string ROLE_LVL          = "lvl";
const std::string ROLE_MONEY        = "money";
const std::string ROLE_SEAT         = "seat";
const std::string ROLE_ACPC_SERVERID= "acpc_server_id"; //用户对战所在的acpcsvr的服务ID
const std::string ROLE_DRAW         = "draw";
const std::string ROLE_WIN          = "win";
const std::string ROLE_LOSE         = "lose";
const std::string ROLE_VERSION      = "version";
const std::string ROLE_SERVERID     = "serverid";       //用户连接的服务ID(apisvr\tpasvr的ID)
const std::string ROLE_TIME         = "time";           //操作时间
const std::string ROLE_CLI_CONN_ID  = "cli_connid";     //客户端连接标识
const std::string ROLE_RESET_ID     = "resetid";        //重置标记ID
const std::string ROLE_SCORE_DATE   = "score_date";     //(最后的)计分日期
const std::string ROLE_ROBOT_ATTR   = "robot_attr";     //robot的私有参数

//房间属性
const std::string ROOM_TPA_ATTR     = "tpa_attr";       //tpa方式接入的私有属性
const std::string ROOM_BASE         = "room_base";      //room的基本信息  
const std::string ROOM_READY        = "room_ready";     //ready更新的信息
const std::string ROOM_ACTION       = "room_action";    //action更新的信息

//公共定义
const int32  ROLE_MAX_MONEY         = 0x7FFFFFF;    //玩家最大金钱额
const int32  MATCH_MAX_MONEY        = 0xFFFFF;      //一局游戏最大的额度(一个玩家)

typedef std::map<std::string, std::string>  FieldNodes;
#define KEY_ROLE(x)                     ("role_"       + std::to_string(x))
#define KEY_ROBOT(x)                    ("robot_"      + std::to_string(x))
#define KEY_USERID(x)                   ("userid_"     + x)
#define KEY_ROOM(x)                     ("room_"       + std::to_string(x))
#define KEY_ROOM_SET(x, y, z)           ("room_set_"   + std::to_string(x) + "." + std::to_string(y) + "." + std::to_string(z))  
#define KEY_ROOM_INCR(x, y, z)          ("room_incr_"  + std::to_string(x) + "." + std::to_string(y) + "." + std::to_string(z))  
#define KEY_ROBOT_INCR(x, y, z)         ("robot_incr_" + std::to_string(x) + "." + std::to_string(y) + "." + std::to_string(z))  
#define KEY_WAIT_SAVE_TO_DB(x, y, z)    ("WaitSaveToDB_RoleIDSet_" + std::to_string(x) + "." + std::to_string(y) + "." + std::to_string(z))  
#define KEY_DATA_NOTIFY_LIST            ("lstDataNotify")         


//通知队列的数据类型--对应KEY_DATA_NOTIFY_LIST键的不同类型
const uint32 DNL_TYPE_USER_REGISTER = 1;        //用户注册
const uint32 DNL_TYPE_USER_LOGIN    = 2;        //用户登录
const uint32 DNL_TYPE_USER_RESET    = 3;        //用户重置钱币


const uint32 REDIS_ROLEID_TTL           = 2*60*60;  //KEY_ROLE的存活时间
const uint32 REDIS_USERID_TTL           = 60*60;    //KEY_USERID的存活时间
const uint32 REDIS_INTERNAL_ROLEID_TTL  = 20*60;    //更新KEY_ROLE的存活时间的时间间隔

const uint32 REDIS_ROOMID_TTL           = 2*60*60;  //KEY_ROOM的存活时间
const uint32 REDIS_INTERNAL_ROOMID_TTL  = 20*60;    //更新KEY_ROOM的存活时间的时间间隔

const uint32 DAY_SECONDS                = 24*60*60; //一条的秒数

//betting tree 存储在redis服务的相关参数定义
const std::string g_strBTInitFlagKey        = "BTInitKey";              //betting tree数据在redis初始化完成状态标记的key
const std::string g_strBTInitCompleted      = "BTInitCompleted";        //存储在redis的betting tree数据已初始化完成
const std::string g_strBTInitNotCompleted   = "BTInitNotCompleted";     //存储在redis的betting tree数据未初始化完成
const std::string g_strBTKeyPre             = "BTK_";                   //betting tree数据的key的前缀

//board tree 存储在redis服务的相关参数定义
const std::string g_strBDInitFlagKey        = "BDInitKey";              //board tree数据在redis初始化完成状态标记的key
const std::string g_strBDInitCompleted      = "BDInitCompleted";        //存储在redis的board tree数据已初始化完成
const std::string g_strBDInitNotCompleted   = "BDInitNotCompleted";     //存储在redis的board tree数据未初始化完成
const std::string g_strBDKeyPre             = "BDK_";                   //board tree数据的key的前缀

//Bucket 存储在redis服务的相关参数定义
const std::string g_strBKInitFlagKey        = "BKInitKey";              //Bucket数据在redis初始化完成状态标记的key
const std::string g_strBKInitCompleted      = "BKInitCompleted";        //存储在redis的Bucket数据已初始化完成
const std::string g_strBKInitNotCompleted   = "BKInitNotCompleted";     //存储在redis的Bucket数据未初始化完成
const std::string g_strBKKeyPre             = "BKK_";                   //Bucket数据的key的前缀


#endif // __COMM_DEFINE_H__


