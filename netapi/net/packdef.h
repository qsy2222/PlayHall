#pragma once
#include<memory.h>

#define _DEF_BUFFER         (4096)
#define _DEF_CONTENT_SIZE	(1024)
#define _MAX_SIZE           (40)
#define _MAX_PATH           (260)

//自定义协议   先写协议头 再写协议结构
//登录 注册 获取好友信息 添加好友 聊天 发文件 下线请求
#define _DEF_PROTOCOL_BASE	(10000)
#define _DEF_PROTOCOL_COUNT (100)

//注册
#define _DEF_PACK_REGISTER_RQ	(_DEF_PROTOCOL_BASE + 0 )
#define _DEF_PACK_REGISTER_RS	(_DEF_PROTOCOL_BASE + 1 )
//登录
#define _DEF_PACK_LOGIN_RQ	(_DEF_PROTOCOL_BASE + 2 )
#define _DEF_PACK_LOGIN_RS	(_DEF_PROTOCOL_BASE + 3 )



//返回的结果
//注册请求的结果
#define tel_is_exist		(0)
#define register_success	(1)
#define name_is_exist       (2)

//登录请求的结果
#define user_not_exist		(0)
#define password_error		(1)
#define login_success		(2)


typedef int PackType;

//协议结构
//注册请求
typedef struct STRU_REGISTER_RQ
{
    STRU_REGISTER_RQ():type(_DEF_PACK_REGISTER_RQ)
    {
        memset( tel  , 0, sizeof(tel));
        memset( name  , 0, sizeof(name));
        memset( password , 0, sizeof(password) );
    }
    //需要手机号码 , 密码, 昵称
    PackType type;
    char tel[_MAX_SIZE];
    char name[_MAX_SIZE];
    char password[_MAX_SIZE];

}STRU_REGISTER_RQ;
//注册回复
typedef struct STRU_REGISTER_RS
{
    //回复结果
    STRU_REGISTER_RS(): type(_DEF_PACK_REGISTER_RS) , result(register_success)
    {
    }
    PackType type;
    int result;

}STRU_REGISTER_RS;

//登录请求
typedef struct STRU_LOGIN_RQ
{
    //登录需要: 手机号 密码
    STRU_LOGIN_RQ():type(_DEF_PACK_LOGIN_RQ)
    {
        memset( tel , 0, sizeof(tel) );
        memset( password , 0, sizeof(password) );
    }
    PackType type;
    char tel[_MAX_SIZE];
    char password[_MAX_SIZE];

}STRU_LOGIN_RQ;
//登录回复
typedef struct STRU_LOGIN_RS
{
    // 需要 结果 , 用户的id
    STRU_LOGIN_RS(): type(_DEF_PACK_LOGIN_RS) , result(login_success),userid(0)
    {
        memset(name, 0, sizeof(name));
    }
    PackType type;
    int result;
    int userid;
    char name[_MAX_SIZE];
}STRU_LOGIN_RS;

typedef struct UserInfo
{
   UserInfo()
   {
       m_sockfd = 0;
       m_id = 0;
       m_roomid = 0;
       m_hasUndo = false;
       memset(m_userName, 0, _MAX_SIZE);
   }
   int m_sockfd;
   int m_id;
   int m_roomid; // 房间id
   int m_zoneid; // 游戏区id
   bool m_hasUndo;
   char m_userName[_MAX_SIZE];
}UserInfo;


///////            游戏相关内容      ////////
#define DEF_PACK_JOIN_ZONE     (_DEF_PROTOCOL_BASE + 4 )
#define DEF_PACK_LEAVE_ZONE    (_DEF_PROTOCOL_BASE + 5 )


enum WNUM_PLAY_ZONE{Five_In_Line = 0x10, E_L_S, D_D_Z};

enum ENUM_JOIN_RESULT {
    _join_fail = 0,
    _join_success = 1,
    _wrong_password = 2,
    _room_full = 3
};
// 加入专区
struct STRU_JOIN_ZONE{
    STRU_JOIN_ZONE():type(DEF_PACK_JOIN_ZONE), userid(0), zoneid(0){

    }
    PackType type;
    int userid;
    int zoneid;
};
// 退出专区
struct STRU_LEAVE_ZONE{
    STRU_LEAVE_ZONE():type(DEF_PACK_LEAVE_ZONE), userid(0){

    }
    PackType type;
    int userid;
};

// 专区内每房间人数
#define DEF_ZONE_ROOM_INFO (_DEF_PROTOCOL_BASE + 10 )
#define DEF_ZONE_INFO_RQ (_DEF_PROTOCOL_BASE + 11 )
#define DEF_ZONE_ROOM_COUNT     121
// 请求
struct STRU_ZONE_INFO_RQ{// 解决这是什么包，请求专区每个房间人数
    STRU_ZONE_INFO_RQ():type(DEF_ZONE_INFO_RQ), zoneid(0){

    }
    PackType type;
    int zoneid;
};

struct STRU_ZONE_ROOM_INFO{// 解决这是什么包，专区每个房间的人数
    STRU_ZONE_ROOM_INFO():type(DEF_ZONE_ROOM_INFO), zoneid(0) {
        memset(roomInfo, 0, sizeof(roomInfo));
        memset(hasPassword, 0, sizeof(hasPassword));
    }
    PackType type;
    int zoneid;
    int roomInfo[DEF_ZONE_ROOM_COUNT];  // 房间人数
    bool hasPassword[DEF_ZONE_ROOM_COUNT];  // 新增：房间是否有密码
};

#define DEF_JOIN_ROOM_RQ (_DEF_PROTOCOL_BASE + 6 )
//加入房间
struct STRU_JOIN_ROOM_RQ //解决这是什么包, 谁加入哪个房间
{
    STRU_JOIN_ROOM_RQ():type(DEF_JOIN_ROOM_RQ), userid(0), roomid(0) {
    }
    PackType type;
    int userid;
    int roomid;
}; //发给服务器，服务器会同步房间成员信息

// 房间为了避免0出现歧义（房间号是0 或者未初始化） 所以让出来0， 1-120，121个元素

#define DEF_JOIN_ROOM_RS (_DEF_PROTOCOL_BASE + 7 )
enum ENUM_ROOM_STATUS{ _host, _player, _watcher }; //房主 玩家 观战者
// 加入房间回复
struct STRU_JOIN_ROOM_RS //解决这是什么包,谁加入哪个房间 是否成功 自己什么身份
{
    STRU_JOIN_ROOM_RS():type(DEF_JOIN_ROOM_RS), userid(0), roomid(0),
        status(_host), result(1){

    }
    PackType type;
    int userid;
    int roomid;
    int status;
    int result; //0 --fail 1--success
};

#define DEF_ROOM_MEMBER (_DEF_PROTOCOL_BASE + 8 )
// 房间成员
struct STRU_ROOM_MEMBER//解决这是什么包,谁,哪个房间,叫什么名字
{
    STRU_ROOM_MEMBER():type(DEF_ROOM_MEMBER), userid(0), status(_player), hasPassword(false) {
        memset(name, 0, sizeof(name));
    }
    PackType type;
    int userid;
    int status;
    bool hasPassword;  // 新增：房间是否有密码
    char name[_MAX_SIZE];
};

#define DEF_LEAVE_ROOM_RQ (_DEF_PROTOCOL_BASE + 9 )
// 退出房间
struct STRU_LEAVE_ROOM_RQ //解决这是什么包, 谁,退出了房间
{
    STRU_LEAVE_ROOM_RQ():type(DEF_LEAVE_ROOM_RQ), userid(0),
        roomid(0), status(_player){

    }
    PackType type;
    int userid;
    int roomid;
    int status;
};//-- 会被转发,如果自己不是房主,房主退出,自己也跟着退出


/*  五子棋游戏 相关   */
#define DEF_FIL_ROOM_READY      (_DEF_PROTOCOL_BASE + 12 )
#define DEF_FIL_GAME_START      (_DEF_PROTOCOL_BASE + 13 )
#define DEF_FIL_AI_BEGIN        (_DEF_PROTOCOL_BASE + 14 )
#define DEF_FIL_AI_END          (_DEF_PROTOCOL_BASE + 15 )
#define DEF_FIL_DISCARD_THIS    (_DEF_PROTOCOL_BASE + 16 )
#define DEF_FIL_SURREND         (_DEF_PROTOCOL_BASE + 17 )
#define DEF_FIL_PIECEDOWN       (_DEF_PROTOCOL_BASE + 18 )
#define DEF_FIL_WIN             (_DEF_PROTOCOL_BASE + 19 )

// 游戏的准备
// 准备 开始 胜利 托管 弃权（当前一次） 投降 落子（谁在什么位置下了一个什么子）
struct STRU_FIL_RQ{
    STRU_FIL_RQ(PackType _type):type(_type), userid(0), zoneid(0), roomid(0){

    }
    PackType type; // 准备 开始 胜利 托管 弃权（当前一次） 投降 复用
    int userid;
    int zoneid;
    int roomid;
}; //只有知道 什么专区 什么房间 才能找到相应的人

// 采用另一种方式 利用客户端知道准备的个数以及开放开始
struct STRU_FIL_RS{
    PackType type;
    int userid;
    int zoneid;
    int roomid;
};

// 落子
struct STRU_FIL_PIECEDOWN{ // 什么专区什么房间 谁 在什么位置放了一个子
    STRU_FIL_PIECEDOWN():type(DEF_FIL_PIECEDOWN), userid(0), zoneid(0), roomid(0)
    , color(0), x(-1), y(-1){

    }
    PackType type;
    int userid;
    int zoneid;
    int roomid;
    int color;
    int x;
    int y;
};



/////////////////////////////////////////////////
///---------------聊天功能---------------------------///
#define DEF_PACK_CHAT_MSG (_DEF_PROTOCOL_BASE + 20)

// 聊天消息结构
struct STRU_CHAT_MSG {
    STRU_CHAT_MSG():type(DEF_PACK_CHAT_MSG), userid(0), zoneid(0), roomid(0) {
        memset(msg, 0, sizeof(msg));
    }
    PackType type;
    int userid;
    int zoneid;
    int roomid;
    char username[_MAX_SIZE];
    char msg[_DEF_CONTENT_SIZE];
};



//----------------------------悔棋--------------------------------
#define DEF_FIL_UNDO (_DEF_PROTOCOL_BASE + 24)

// 悔棋请求
struct STRU_FIL_UNDO {
    STRU_FIL_UNDO():type(DEF_FIL_UNDO), userid(0), zoneid(0), roomid(0) {}
    PackType type;
    int userid;
    int zoneid;
    int roomid;
};
