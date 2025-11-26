#ifndef CLOGIC_H
#define CLOGIC_H

#include"TCPKernel.h"
#include <unordered_map>


class CLogic
{
public:
    CLogic( TcpKernel* pkernel ) : m_roomUserList( 121 )
    {
        m_pKernel = pkernel;
        m_sql = pkernel->m_sql;
        m_tcp = pkernel->m_tcp;
        pthread_mutex_init( &m_roomListMutex, NULL );
    }
public:
    //设置协议映射
    void setNetPackMap();
    /************** 发送数据*********************/
    void SendData( sock_fd clientfd, char*szbuf, int nlen )
    {
        m_pKernel->SendData( clientfd ,szbuf , nlen );
    }
    /************** 网络处理 *********************/
    //注册
    void RegisterRq(sock_fd clientfd, char*szbuf, int nlen);
    //登录
    void LoginRq(sock_fd clientfd, char*szbuf, int nlen);
    // 加入分区
    void JoinZoneRq(sock_fd clientfd, char*szbuf, int nlen);
    // 离开分区
    void LeaveZoneRq(sock_fd clientfd, char*szbuf, int nlen);
    // 加入房间
    void JoinRoomRq(sock_fd clientfd, char*szbuf, int nlen);
    // 离开房间
    void LeaveRoomRq(sock_fd clientfd, char*szbuf, int nlen);
    // 获取专区每个房间玩家个数
    void ZoneInfoRq(sock_fd clientfd, char*szbuf, int nlen);
    // 五子棋游戏命令转发
    void FIL_MsgSendRq(sock_fd clientfd, char*szbuf, int nlen);
    // 五子棋落子转发
    void FIL_PieceDownRq(sock_fd clientfd, char*szbuf, int nlen);
    // 聊天
    void ChatMsgRq(sock_fd clientfd, char *szbuf, int nlen);
    // 悔棋处理
    void FIL_UndoRq(sock_fd clientfd, char* szbuf, int nlen);
    /*******************************************/

private:
    TcpKernel* m_pKernel;
    CMysql * m_sql;
    Block_Epoll_Net * m_tcp;
    MyMap<int, UserInfo*> m_mapIdToUserInfo;
    vector<list<int>> m_roomUserList;
    pthread_mutex_t m_roomListMutex;

};

#endif // CLOGIC_H
