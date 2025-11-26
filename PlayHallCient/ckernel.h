 #ifndef CKERNEL_H
#define CKERNEL_H

#include <QObject>
#include "INetMediator.h"
#include "packdef.h"
#include <vector>

#include "maindialog.h"
#include "logindialog.h"
#include "fiveinlinezone.h"
#include "roomdialog.h"
#include <QTimer>

// 成员函数指针类型
class CKernel;
typedef void (CKernel::*PFUN)( unsigned int lSendIP, char* buf, int nlen);

//单例
class CKernel : public QObject
{
    Q_OBJECT
public:
    static CKernel* GetInstance(){
        static CKernel kernel;
        return &kernel;
    }
public slots:
    void DestroyInstance();
    void SendData(char* buf, int nlen);

    // 窗口处理
    void slot_loginCommit(QString tel, QString password);
    void slot_registerCommit(QString tel, QString password, QString name);
    void slot_joinZone(int zoneid);
    void slot_leaveZone();
    void slot_joinRoom(int roomid);
    void slot_leaveRoom();
    // 获取专区每个房间玩家数
    void slot_roomInfoInZone();
    // 五子棋准备和开局
    void slot_fil_gameReady(int zoneid, int roomid, int userid);
    void slot_fil_gameStart(int zoneid, int roomid);
    // 落子
    void slot_fil_pieceDown(int blackorwhite, int x, int y);
    // 输赢
    void slot_fil_win(int blackorwhite);
    // 电脑托管
    void slot_fil_playByCpuBegin(int zoneid, int roomid, int userid);
    void slot_fil_playByCpuEnd(int zoneid, int roomid, int userid);

    // 网络处理
    void slot_ReadyData( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealLoginRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealRegisterRs(unsigned int lSendIP, char *buf, int nlen);
    void slot_dealJoinRoomRs(unsigned int lSendIP, char *buf, int nlen);
    void slot_dealRoomMemberRq(unsigned int lSendIP, char *buf, int nlen);
    void slot_dealLeaveRoomRq(unsigned int lSendIP, char *buf, int nlen);
    void slot_dealZoneRoomInfo(unsigned int lSendIP, char *buf, int nlen);
    /* 游戏 */
    void slot_dealFilGameReadyRq(unsigned int lSendIP, char *buf, int nlen);
    void slot_dealFilGameStartRq(unsigned int lSendIP, char *buf, int nlen);
    void slot_dealFilPieceDownRq(unsigned int lSendIP, char *buf, int nlen);
    void slot_dealFilWinRq(unsigned int lSendIP, char *buf, int nlen);
    /* 托管 */
    void slot_dealFilPlayByCpuBegin(unsigned int lSendIP, char *buf, int nlen);
    void slot_dealFilPlayByCpuEnd(unsigned int lSendIP, char *buf, int nlen);

signals:

private:
    void setNEtPackFunMap();
    void ConfigSet();
    explicit CKernel(QObject *parent = nullptr);
    ~CKernel(){ /*DestroyInstance();*/ }
    CKernel(const CKernel& kernel){}
    CKernel& operator = ( const CKernel& kernel ){
        return *this;
    }
    // 成员属性 网络 ui类对象
    MainDialog *    m_mainDialog;
    LoginDialog*    m_loginDialog;
    FiveInLineZone* m_fiveInLineZone;
    RoomDialog*     m_roomDialog;

    INetMediator* m_client;
    // 协议映射表 协议头与处理函数的对应关系
    std::vector<PFUN> m_netPackFunMAp;

    // 个人信息
    int m_id;
    int m_roomid;
    int m_zoneid;
    bool m_isHost;
    QString m_userName;
    char m_serverIP[20];

    // 单位时间获取专区内玩家个数
    QTimer m_rqTimer;
};

#endif // CKERNEL_H
