#include "clogic.h"
#include <list>
using namespace std;
void CLogic::setNetPackMap()
{
    NetPackMap(_DEF_PACK_REGISTER_RQ)    = &CLogic::RegisterRq;
    NetPackMap(_DEF_PACK_LOGIN_RQ)       = &CLogic::LoginRq;
    NetPackMap(DEF_PACK_JOIN_ZONE)       = &CLogic::JoinZoneRq;
    NetPackMap(DEF_PACK_LEAVE_ZONE)       = &CLogic::LeaveZoneRq;
    NetPackMap(DEF_JOIN_ROOM_RQ)       = &CLogic::JoinRoomRq;
    NetPackMap(DEF_LEAVE_ROOM_RQ)       = &CLogic::LeaveRoomRq;
    NetPackMap(DEF_ZONE_INFO_RQ)       = &CLogic::ZoneInfoRq;

    NetPackMap(DEF_FIL_ROOM_READY   ) =   &CLogic::FIL_MsgSendRq;
    NetPackMap(DEF_FIL_GAME_START   ) =   &CLogic::FIL_MsgSendRq;
    NetPackMap(DEF_FIL_AI_BEGIN     ) =   &CLogic::FIL_MsgSendRq;
    NetPackMap(DEF_FIL_AI_END       ) =   &CLogic::FIL_MsgSendRq;
    NetPackMap(DEF_FIL_DISCARD_THIS ) =   &CLogic::FIL_MsgSendRq;
    NetPackMap(DEF_FIL_SURREND      ) =   &CLogic::FIL_MsgSendRq;
    NetPackMap(DEF_FIL_PIECEDOWN    ) =   &CLogic::FIL_MsgSendRq;
    NetPackMap(DEF_FIL_WIN          ) =   &CLogic::FIL_MsgSendRq;

    NetPackMap(DEF_FIL_PIECEDOWN    ) =   &CLogic::FIL_PieceDownRq;

    NetPackMap(DEF_PACK_CHAT_MSG) = &CLogic::ChatMsgRq;

    NetPackMap(DEF_FIL_UNDO) = &CLogic::FIL_UndoRq;

}

#define _DEF_COUT_FUNC_    cout << "clientfd:"<< clientfd << __func__ << endl;

//注册
void CLogic::RegisterRq(sock_fd clientfd,char* szbuf,int nlen)
{
    //cout << "clientfd:"<< clientfd << __func__ << endl;
    _DEF_COUT_FUNC_
    // 解析数据包 获取 tel password name
    STRU_REGISTER_RQ * rq = (STRU_REGISTER_RQ*)szbuf;
    STRU_REGISTER_RS rs;
    // 根据tel查询数据库看有没有
    list<string> lstRes;
    char sqlstr[1000] = "";
    sprintf(sqlstr, "select tel from t_user where tel = '%s';", rq->tel);
    m_sql->SelectMysql(sqlstr, 1, lstRes);
    // 有 返回结果
    if(lstRes.size() > 0){
        rs.result = tel_is_exist;
    }
    else{
        // 没有 接下来看昵称有没有
        lstRes.clear();
        sprintf(sqlstr, "select name from t_user where name = '%s';", rq->name);
        m_sql->SelectMysql(sqlstr, 1, lstRes);
        // 有 返回结果
        if(lstRes.size() > 0){
            rs.result = name_is_exist;
        }
        else{
            // 没有 注册成功  更新数据库  写表
            rs.result = register_success;
            sprintf(sqlstr, "insert into t_user (tel, password, name) values('%s', '%s', '%s');"
                    , rq->tel, rq->password, rq->name);
            bool res = m_sql->UpdataMysql(sqlstr);
            if(!res){
                printf("update fail:%s\n", sqlstr);
            }
        }
        // 返回结果
        SendData(clientfd, (char*)&rs, sizeof(rs));
    }

   
}

//登录
void CLogic::LoginRq(sock_fd clientfd ,char* szbuf,int nlen)
{
//    cout << "clientfd:"<< clientfd << __func__ << endl;
    _DEF_COUT_FUNC_

//    STRU_LOGIN_RS rs;
//    rs.result = password_error;
//    SendData( clientfd , (char*)&rs , sizeof rs );

      // 析包 获取 tel password
      STRU_LOGIN_RQ *rq = (STRU_LOGIN_RQ*)szbuf;
    STRU_LOGIN_RS rs;
      // 根据 tel 查 id password name
      char sqlstr[1000] = "";
      list<string> lstRes;
      sprintf(sqlstr, "select id, password, name from t_user where tel = '%s';", rq->tel);
      m_sql->SelectMysql(sqlstr, 3, lstRes);
      if(lstRes.size() == 0){
          // 没有 返回结果
          rs.result = user_not_exist;
      }
      else {
          int id = atoi(lstRes.front().c_str());
          lstRes.pop_front();
          string strPassword = lstRes.front();
          lstRes.pop_front();
          string strName = lstRes.front();
          lstRes.pop_front();
      // 有 看密码是否一致
          if(strcmp(rq->password, strPassword.c_str()) != 0){
              // 不一致 返回结果
              rs.result = password_error;
          }
          else{
              // 一致
              rs.result = login_success;
              UserInfo* info = nullptr;
              // 如果之前有用户信息 强制下线 回收
              if(m_mapIdToUserInfo.find(id, info)){
                  // 强制下线

                  // 回收
                  m_mapIdToUserInfo.erase(id);
                  delete info;
              }

              // 保存用户信息
              info = new UserInfo;
              info->m_id = id;
              info->m_sockfd = clientfd;
              strcpy(info->m_userName, strName.c_str());
              strcpy(rs.name, strName.c_str());
              rs.userid = id;
              // 把 id 和套接字捆绑在一起 **
              m_mapIdToUserInfo.insert(id, info);

              // 返回结果 id name result
              SendData(clientfd, (char*)&rs, sizeof(rs));
              // 离线信息

              // 推送信息
              return;
          }
      }
      SendData(clientfd, (char*)&rs, sizeof(rs));
}

void CLogic::JoinZoneRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d JoinZoneRq\n", clientfd);
    // 拆包，更新信息
    STRU_JOIN_ZONE *rq = (STRU_JOIN_ZONE*)szbuf;
    UserInfo* info = nullptr;
    if(!m_mapIdToUserInfo.find(rq->userid, info)){
        return;
    }
    info->m_zoneid = rq->zoneid;
}

void CLogic::LeaveZoneRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d LeaveZoneRq\n", clientfd);
    // 拆包，更新信息
    STRU_LEAVE_ZONE *rq = (STRU_LEAVE_ZONE*)szbuf;
    UserInfo* info = nullptr;
    if(!m_mapIdToUserInfo.find(rq->userid, info)){
        return;
    }
    info->m_zoneid = 0;
}

// 加入房间请求 加入时可能多个线程同时有客户端请求，房间列表应该加锁处理
void CLogic::JoinRoomRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d JoinRoomRq\n", clientfd);
    // 拆包
    STRU_JOIN_ROOM_RQ* rq = (STRU_JOIN_ROOM_RQ*)szbuf;
    STRU_JOIN_ROOM_RS rs;
    list<int> tmplist;
    pthread_mutex_lock(&m_roomListMutex);

    list<int> & userlst = m_roomUserList[rq->roomid];
    switch(userlst.size()){

    // 首先 0-120 数组 看房间里人数
    // 0 加入房间是放逐 返回
    // 1 玩家 返回加入成功 需要同步信息 玩家给加入的人发 加入的人给玩家发 加入房间列表
    // 2 加入失败

    case 0:
        rs.result = 1;
        rs.roomid = rq->roomid;
        rs.status = _host;
        rs.userid = rq->userid;
        userlst.push_back(rq->userid);
        break;
    case 1:
        rs.result = 1;
        rs.roomid = rq->roomid;
        rs.status = _player;
        rs.userid = rq->userid;
        userlst.push_back(rq->userid);
        break;
    case 2:
        rs.status = 0;
        break;
    default:
        rs.status = 0;
        break;
    }
    tmplist = userlst;
    pthread_mutex_unlock(&m_roomListMutex);

    SendData(clientfd, (char*)&rs, sizeof(rs));

    // 玩家给加入的人发 加入的人给玩家发
    // size 为 1 自己给自己发
    if(tmplist.size() > 0){
        int joinid = rq->userid;
        // 根据id拿到用户信息
        UserInfo* joinInfo = nullptr;
        if(!m_mapIdToUserInfo.find(joinid, joinInfo)) return;
        // 写成员信息的请求
        STRU_ROOM_MEMBER joinrq;
        joinrq.userid = joinid;
        joinrq.status = rs.status;
        strcpy(joinrq.name, joinInfo->m_userName);

        bool flag = false;
        for(auto ite = tmplist.begin(); ite != tmplist.end(); ++ite){
            int status = _player;
            if(!flag){
                status = _host;
                flag = true;
            }
            int roomMemid = *ite;
            if(roomMemid != joinid){
                // 根据id拿到用户信息
                UserInfo* MemInfo = nullptr;
                if(!m_mapIdToUserInfo.find(roomMemid, MemInfo)) continue;
                // 写成员信息的请求
                STRU_ROOM_MEMBER Memrq;
                Memrq.userid = roomMemid;
                Memrq.status = status;
                strcpy(Memrq.name, MemInfo->m_userName);

                // 互相发送
                SendData(joinInfo->m_sockfd, (char*)&Memrq, sizeof(Memrq));
                SendData(MemInfo->m_sockfd, (char*)&joinrq, sizeof(joinrq));
            }else{
                // 自己给自己发
                SendData(joinInfo->m_sockfd, (char*)&joinrq, sizeof(joinrq));
            }
        }
    }



}

// 离开房间
void CLogic::LeaveRoomRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d LeaveRoomRq\n", clientfd);
    // 拆包
    STRU_LEAVE_ROOM_RQ * rq = (STRU_LEAVE_ROOM_RQ *)szbuf;
    // 谁 什么身份 离开哪个房间
//    rq->roomid;
//    rq->status;
    int leaveid = rq->userid;
    // 当前的这个人的离开要发给房间里所有人
    list<int>& lst = m_roomUserList[rq->roomid];
    // 给除了这个离开的人之外的所有人转发离开信息
    for(auto ite = lst.begin(); ite != lst.end(); ++ite){
        int memid = *ite;
        if(leaveid != memid){
            UserInfo* memInfo = nullptr;
            if(!m_mapIdToUserInfo.find(memid, memInfo)) continue;
            SendData(memInfo->m_sockfd, szbuf, nlen);
        }
    }
    pthread_mutex_lock(&m_roomListMutex);
    // 根据身份不同 host player 操作链表 房间信息
    if(rq->status == _host){
        lst.clear();
    }else if(rq->status == _player){
        // 找到离开的人 清掉
        for(auto ite = lst.begin(); ite != lst.end(); ++ite){
            if(leaveid == *ite){
                ite = lst.erase(ite);
                break;
            }
        }
    }
    pthread_mutex_unlock(&m_roomListMutex);
}

void CLogic::ZoneInfoRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d ZoneInfoRq\n", clientfd);
    // 拆包
    STRU_ZONE_INFO_RQ * rq = (STRU_ZONE_INFO_RQ *)szbuf;
    STRU_ZONE_ROOM_INFO rs;
    rs.zoneid = rq->zoneid;
    // 根据专区拿到房间列表
    for(int i = 1; i < m_roomUserList.size(); ++i){
        list<int>& lst = m_roomUserList[i];
        rs.roomInfo[i] = lst.size();
    }
    SendData(clientfd, (char*)&rs, sizeof(rs));

}

// 服务器转发五子棋游戏命令给房间内其他成员
void CLogic::FIL_MsgSendRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d FIL_MsgSendRq\n", clientfd);
    // 拆包
    STRU_FIL_RQ * rq = (STRU_FIL_RQ *)szbuf;

    // 什么专区 什么房间 谁 发了什么
    // 根据专区 拿到房间列表 根据房间 拿到房间内成员 转发给房间里所有人
    //rq->zoneid;
    list<int>& lstRes = m_roomUserList[rq->roomid];
    for(auto ite = lstRes.begin(); ite != lstRes.end(); ++ite){
        int userid = *ite;
        UserInfo* info = nullptr;
        if(!m_mapIdToUserInfo.find(userid, info)) continue;
        SendData(info->m_sockfd, szbuf, nlen);
    }
}

void CLogic::FIL_PieceDownRq(sock_fd clientfd, char *szbuf, int nlen)
{
    printf("clientfd:%d FIL_PieceDownRq\n", clientfd);
    // 拆包
    STRU_FIL_PIECEDOWN * rq = (STRU_FIL_PIECEDOWN *)szbuf;
    // 什么专区 什么房间 谁 发了什么
    // 根据专区 拿到房间列表 根据房间 拿到房间内成员 转发给房间里所有人
    //rq->zoneid;
    list<int>& lstRes = m_roomUserList[rq->roomid];
    for(auto ite = lstRes.begin(); ite != lstRes.end(); ++ite){
        int userid = *ite;
        UserInfo* info = nullptr;
        if(!m_mapIdToUserInfo.find(userid, info)) continue;
        SendData(info->m_sockfd, szbuf, nlen);
    }
}

void CLogic::ChatMsgRq(sock_fd clientfd, char *szbuf, int nlen) {
    (void)clientfd;
    (void)nlen;

    STRU_CHAT_MSG *rq = (STRU_CHAT_MSG*)szbuf;
    UserInfo* info = nullptr;

    if(!m_mapIdToUserInfo.find(rq->userid, info)) {
        return;
    }

    // 安全拷贝用户名
    strncpy(rq->username, info->m_userName, _MAX_SIZE - 1);
    rq->username[_MAX_SIZE - 1] = '\0';

    // 转发给房间成员
    list<int>& lstRes = m_roomUserList[rq->roomid];
    for(auto ite = lstRes.begin(); ite != lstRes.end(); ++ite) {
        int userid = *ite;
        UserInfo* targetInfo = nullptr;
        if(!m_mapIdToUserInfo.find(userid, targetInfo)) continue;
        SendData(targetInfo->m_sockfd, szbuf, sizeof(STRU_CHAT_MSG));
    }
}



// 悔棋处理函数
void CLogic::FIL_UndoRq(sock_fd clientfd, char* szbuf, int nlen) {
    printf("clientfd:%d FIL_UndoRq\n", clientfd);
    // 拆包
    STRU_FIL_UNDO* rq = (STRU_FIL_UNDO*)szbuf;

    // 转发给房间内其他成员
    list<int>& lstRes = m_roomUserList[rq->roomid];
    for(auto ite = lstRes.begin(); ite != lstRes.end(); ++ite) {
        int userid = *ite;
        UserInfo* info = nullptr;
        if(!m_mapIdToUserInfo.find(userid, info)) continue;
        SendData(info->m_sockfd, szbuf, nlen);
    }
}

