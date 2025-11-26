#include "ckernel.h"
#include <QDebug>
#include "TcpClientMediator.h"
#include <QMessageBox>
#include "md5.h"

// 获得MD5函数
#define MD5_KEY 1234
static std::string getMD5(QString val){
    QString tmp = QString("%1_%2").arg(val).arg(MD5_KEY);
    MD5 md(tmp.toStdString());
    return md.toString();
}
// 1_1234 -> ea135e06cd37ab7e304e1dc440c93ea2

// 宏定义简化 映射偏移
#define NetPackMap(a) m_netPackFunMAp[(a) - _DEF_PROTOCOL_BASE]

// 协议对应关系
void CKernel::setNEtPackFunMap()
{
    //m_netPackFunMAp[_DEF_PACK_LOGIN_RS - _DEF_PROTOCOL_BASE] = &CKernel::slot_dealloginRs;
    NetPackMap(_DEF_PACK_LOGIN_RS)    = &CKernel::slot_dealLoginRs;
    NetPackMap(_DEF_PACK_REGISTER_RS) = &CKernel::slot_dealRegisterRs;
    NetPackMap(DEF_JOIN_ROOM_RS) = &CKernel::slot_dealJoinRoomRs;
    NetPackMap(DEF_ROOM_MEMBER) = &CKernel::slot_dealRoomMemberRq;
    NetPackMap(DEF_LEAVE_ROOM_RQ) = &CKernel::slot_dealLeaveRoomRq;
    NetPackMap(DEF_ZONE_ROOM_INFO) = &CKernel::slot_dealZoneRoomInfo;

    /* 游戏相关 */
    NetPackMap(DEF_FIL_ROOM_READY  )   = &CKernel::slot_dealFilGameReadyRq;
    NetPackMap(DEF_FIL_GAME_START  )   = &CKernel::slot_dealFilGameStartRq;
    NetPackMap(DEF_FIL_AI_BEGIN    )   = &CKernel::slot_dealFilPlayByCpuBegin;
    NetPackMap(DEF_FIL_AI_END      )   = &CKernel::slot_dealFilPlayByCpuEnd;
//    NetPackMap(DEF_FIL_DISCARD_THIS)   = &CKernel::slot_deal
//    NetPackMap(DEF_FIL_SURREND     )   = &CKernel::slot_deal

    NetPackMap(DEF_FIL_PIECEDOWN   )   = &CKernel::slot_dealFilPieceDownRq;
    NetPackMap(DEF_FIL_WIN         )   = &CKernel::slot_dealFilWinRq;

}

#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
void CKernel::ConfigSet()
{
    // 获取配置文件里面的信息以及设置
    // .ini 配置文件
    // [net] 组名 GroupName
    // key = value

    // 1. ip默认
    strcpy(m_serverIP, _DEF_SERVER_IP);
    // 2. 设置和获取配置文件 有还是没有 配置文件在哪里？ 设置和exe同一级的目录
    // exe的目录
    // 路径
    QString path = QCoreApplication::applicationDirPath() + "/config.ini";
    // 查看是否存在
    QFileInfo info(path);
    if(info.exists()){
        // 存在
        QSettings setting(path, QSettings::IniFormat, nullptr);
        // [net]组写入值
        setting.beginGroup("net");
        QVariant var = setting.value("ip");
        QString strip = var.toString();
        if(!strip.isEmpty()){
            strcpy(m_serverIP, strip.toStdString().c_str());
        }
        setting.endGroup();
    }else{
        // 不存在
        QSettings setting(path, QSettings::IniFormat, nullptr);
        // [net]组写入值
        setting.beginGroup("net");
        setting.setValue("ip", QString::fromStdString(m_serverIP));
        setting.endGroup();
    }
    qDebug() << "ip: " << m_serverIP;
}

void CKernel::DestroyInstance()
{
    qDebug() << __func__;
    if(m_client){
        m_client->CloseNet();
        delete m_client;
        m_client = nullptr;
    }
    delete m_mainDialog;
    delete m_loginDialog;
}

void CKernel::slot_loginCommit(QString tel, QString password)
{
    // 应该加密
    // 封包
    STRU_LOGIN_RQ rq;
    strcpy(rq.tel, tel.toStdString().c_str());
    qDebug() << password << "MD5: " << getMD5(password).c_str();
    strcpy(rq.password, getMD5(password).c_str());

    // 发送
    SendData((char*)&rq, sizeof(rq));
}

void CKernel::slot_registerCommit(QString tel, QString password, QString name)
{
    // 应该加密
    // 封包
    STRU_REGISTER_RQ rq;
    strcpy(rq.tel, tel.toStdString().c_str());
    // 通过MD5加密
    strcpy(rq.password, getMD5(password).c_str());
    // 兼容中文
    std::string strName = name.toStdString();
    strcpy(rq.name, strName.c_str());
    // 发送
    SendData((char*)&rq, sizeof(rq));
}

// 提交加入分区
void CKernel::slot_joinZone(int zoneid)
{
    m_rqTimer.start(1000);
    // 成员改变属性
    m_zoneid = zoneid;

    // 请求包
    STRU_JOIN_ZONE rq;
    rq.userid = m_id;
    rq.zoneid = zoneid;

    SendData((char*)&rq, sizeof(rq));

    // ui跳转
    // 显示专区
    switch (zoneid) {
    case Five_In_Line:
        m_fiveInLineZone->show();
        break;
    }
}

void CKernel::slot_leaveZone()
{
    m_rqTimer.stop();
    // 成员实行修改
    m_zoneid = 0;
    // 请求
    STRU_LEAVE_ZONE rq;
    rq.userid = m_id;
    SendData((char*)&rq, sizeof(rq));
    // ui跳转
    m_mainDialog->show();
}
// 提交加入房间
void CKernel::slot_joinRoom(int roomid)
{
    if(m_roomid != 0){
        QMessageBox::about(nullptr, "提示", "已经在房间中，无法加入。");
        return;
    }
    // 在加入房间成功后隐藏
    //m_fiveInLineZone->hide();
    // 提交请求
    STRU_JOIN_ROOM_RQ rq;
    rq.userid = m_id;
    rq.roomid = roomid;

    SendData((char*)&rq, sizeof(rq));
}

// 离开房间
void CKernel::slot_leaveRoom()
{
    // 玩家主动离开
    STRU_LEAVE_ROOM_RQ rq;
    rq.status = m_isHost ? _host : _player;
    rq.userid = m_id;
    rq.roomid = m_roomid;
    SendData((char*)&rq, sizeof(rq));

    // 界面
    m_roomDialog->clearRoom();
    m_roomDialog->hide();
    m_fiveInLineZone->show();

    // 后台数据
    m_roomid = 0;
    m_isHost = false;
}

// 获取专区内每个房间玩家数
void CKernel::slot_roomInfoInZone()
{
    STRU_ZONE_INFO_RQ rq;
    rq.zoneid = m_zoneid;

    SendData((char*)&rq, sizeof(rq));
}

// 五子棋准备
void CKernel::slot_fil_gameReady(int zoneid, int roomid, int userid)
{
    // 自己的准备

    STRU_FIL_RQ rq(DEF_FIL_ROOM_READY);
    rq.roomid = roomid;
    rq.zoneid = zoneid;
    rq.userid = m_id;

    SendData((char*)&rq, sizeof(rq));
}
// 五子棋开局
void CKernel::slot_fil_gameStart(int zoneid, int roomid)
{
    STRU_FIL_RQ rq(DEF_FIL_GAME_START);
    rq.roomid = roomid;
    rq.zoneid = zoneid;
    rq.userid = m_id;

    SendData((char*)&rq, sizeof(rq));
}

void CKernel::slot_fil_pieceDown(int blackorwhite, int x, int y)
{
    // 封包
    STRU_FIL_PIECEDOWN rq;
    rq.color = blackorwhite;
    rq.x = x;
    rq.y = y;
    rq.userid = m_id;
    rq.roomid = m_roomid;
    rq.zoneid = m_zoneid;

    SendData((char*)&rq, sizeof(rq));
}

#include "fiveinline.h"
void CKernel::slot_fil_win(int blackorwhite)
{
    // 弹窗
    QString res;
    if(m_isHost){
        if(blackorwhite == FiveInLine::Black){
            res = QString("获胜");
        }else{
            res = QString("失败");
        }
    }else{
        if(blackorwhite == FiveInLine::White){
            res = QString("获胜");
        }else{
            res = QString("失败");
        }
    }


    QMessageBox::about(m_roomDialog, "提示", res);
    // 可以点准备和开局
    m_roomDialog->resetAllPushButton();
}

void CKernel::slot_fil_playByCpuBegin(int zoneid, int roomid, int userid)
{
    // 发包
    STRU_FIL_RQ rq(DEF_FIL_AI_BEGIN);
    rq.roomid = roomid;
    rq.userid = userid;
    rq.zoneid = zoneid;

    SendData((char*)&rq, sizeof(rq));
}

void CKernel::slot_fil_playByCpuEnd(int zoneid, int roomid, int userid)
{
    // 发包
    STRU_FIL_RQ rq(DEF_FIL_AI_END);
    rq.roomid = roomid;
    rq.userid = userid;
    rq.zoneid = zoneid;

    SendData((char*)&rq, sizeof(rq));
}

// 接收处理
void CKernel::slot_ReadyData(unsigned int lSendIP, char *buf, int nlen)
{
    // 协议映射表
    PackType type = *(int*)buf; // *(int*) 按照整型取数据
    if(type >= _DEF_PROTOCOL_BASE
            && type < _DEF_PROTOCOL_BASE + _DEF_PROTOCOL_COUNT)
    {
        // 根据协议头跳转到对应函数
        // 协议映射处理
        PFUN pf = NetPackMap(type);
        // 执行函数
        (this->*pf)(lSendIP, buf, nlen);
    }

    // buf要回收
    delete[] buf;
}

void CKernel::slot_dealLoginRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug() << __func__;
    // 拆包
    STRU_LOGIN_RS * rs = (STRU_LOGIN_RS*)buf;
    // 根据不同结果显示
    switch(rs->result){
    case user_not_exist:
        QMessageBox::about(m_loginDialog, "提示", "登陆失败，用户不存在！");
        break;
    case password_error:
        QMessageBox::about(m_loginDialog, "提示", "登陆失败，密码错误！");
        break;
    case login_success:
        //ui 切换
        m_loginDialog->hide();
        m_mainDialog->show();
        // 存储
        m_id = rs->userid;
        m_userName = QString::fromStdString(rs->name);
        break;
    default:
        QMessageBox::about(m_loginDialog, "提示", "登陆异常");
        break;
    }
}

void CKernel::slot_dealRegisterRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug() << __func__;
    // 解析数据包
    STRU_REGISTER_RS * rs = (STRU_REGISTER_RS * )buf;
    // 根据结果弹窗
    switch(rs->result){
    case tel_is_exist:
        QMessageBox::about(this->m_loginDialog, "注册失败", "手机号已存在");
        break;
    case name_is_exist:
        QMessageBox::about(this->m_loginDialog, "注册失败", "昵称已存在");
        break;
    case register_success:
        QMessageBox::about(this->m_loginDialog, "注册成功", "恭喜你！可以继续游玩！");
        break;
    default:
        QMessageBox::about(this->m_loginDialog, "注册失败", "注册异常");
        break;
    }
}
// 加入房间回复处理
void CKernel::slot_dealJoinRoomRs(unsigned int lSendIP, char *buf, int nlen)
{
    // 拆包
    STRU_JOIN_ROOM_RS * rs = (STRU_JOIN_ROOM_RS*)buf;
    if(rs->result == 0){
        QMessageBox::about(m_fiveInLineZone, "提示", "加入房间失败"); //todo：具体专区
        return;;
    }
    if(rs->status == _host){
        m_isHost = true;
    }
    m_roomid = rs->roomid;

    // 成功 跳转 成员赋值
    m_fiveInLineZone->hide();
    m_roomDialog->setInfo(m_roomid);
    m_roomDialog->show();


}

void CKernel::slot_dealRoomMemberRq(unsigned int lSendIP, char *buf, int nlen)
{
    // 拆包 别人给你发 自己给自己发
    STRU_ROOM_MEMBER * rq = (STRU_ROOM_MEMBER * )buf;
    if(rq->status == _host){
        m_roomDialog->setHostInfo(rq->userid,
                                  QString::fromStdString( rq->name));
    }
    if(rq->status == _player){
        m_roomDialog->setPlayerInfo(rq->userid,
                                  QString::fromStdString( rq->name));
    }

    m_roomDialog->setUserStatus(m_isHost ? _host : _player);

}

void CKernel::slot_dealLeaveRoomRq(unsigned int lSendIP, char *buf, int nlen)
{
    // 拆包
    STRU_LEAVE_ROOM_RQ * rq = (STRU_LEAVE_ROOM_RQ * )buf;
    if(rq->status == _host){
        // 离开的对方是房主，自己也要退出
        // 界面
        m_roomDialog->clearRoom();
        m_roomDialog->hide();
        m_fiveInLineZone->show();

        // 后台数据
        m_roomid = 0;
        m_isHost = false;
    }else{
        m_roomDialog->playerLeave(rq->userid);
    }

}

void CKernel::slot_dealZoneRoomInfo(unsigned int lSendIP, char *buf, int nlen)
{
    // 拆包
    STRU_ZONE_ROOM_INFO * rq = (STRU_ZONE_ROOM_INFO *)buf;
    // 根据专区 找到对应窗口

    // 根据数组 更新ui
    std::vector<RoomItem*>& vec = m_fiveInLineZone->getVecRoomItem();
    for(int i = 1; i < vec.size(); ++i){
        vec[i]->setRoomItem(rq->roomInfo[i]);
    }
}

void CKernel::slot_dealFilGameReadyRq(unsigned int lSendIP, char *buf, int nlen)
{
    // 拆包
    STRU_FIL_RQ * rq = (STRU_FIL_RQ *)buf;
    // 什么专区 房间 谁 做什么事
    if(rq->roomid == m_roomid){
        m_roomDialog->setPlayerReady(rq->userid);
    }
}

void CKernel::slot_dealFilGameStartRq(unsigned int lSendIP, char *buf, int nlen)
{
    // 拆包
    STRU_FIL_RQ * rq = (STRU_FIL_RQ *)buf;
    // 什么专区 房间 谁 做什么事
    if(rq->roomid == m_roomid){
        m_roomDialog->setGameStart();
    }
}

// 处理落子
void CKernel::slot_dealFilPieceDownRq(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_FIL_PIECEDOWN* rq = (STRU_FIL_PIECEDOWN*)buf;
    m_roomDialog->slot_pieceDoown(rq->color, rq->x, rq->y);
}

void CKernel::slot_dealFilWinRq(unsigned int lSendIP, char *buf, int nlen)
{

}

void CKernel::slot_dealFilPlayByCpuBegin(unsigned int lSendIP, char *buf, int nlen)
{
    // 拆包
    STRU_FIL_RQ * rq = (STRU_FIL_RQ * )buf;
    // 查看身份
    rq->zoneid; // 根据专区看房间
    rq->roomid; // 根据房间看ui
    if(m_id == rq->userid){
        if(m_isHost){
            m_roomDialog->setHostPlayByCpu(true);
        }else{
            m_roomDialog->setPlayerPlayByCpu(true);
        }
    }else{
        if(m_isHost){
            m_roomDialog->setPlayerPlayByCpu(true);
        }else{
            m_roomDialog->setHostPlayByCpu(true);
        }
    }
}

void CKernel::slot_dealFilPlayByCpuEnd(unsigned int lSendIP, char *buf, int nlen)
{
    // 拆包
    STRU_FIL_RQ * rq = (STRU_FIL_RQ * )buf;
    // 查看身份
    rq->zoneid; // 根据专区看房间
    rq->roomid; // 根据房间看ui
    if(m_id == rq->userid){
        if(m_isHost){
            m_roomDialog->setHostPlayByCpu(false);
        }else{
            m_roomDialog->setPlayerPlayByCpu(false);
        }
    }else{
        if(m_isHost){
            m_roomDialog->setPlayerPlayByCpu(false);
        }else{
            m_roomDialog->setHostPlayByCpu(false);
        }
    }
}

void CKernel::SendData(char *buf, int nlen)
{
    m_client->SendData(0, buf, nlen);
}

CKernel::CKernel(QObject *parent) : QObject(parent)
  , m_netPackFunMAp( _DEF_PROTOCOL_COUNT, 0 )
  , m_id(0), m_roomid(0), m_zoneid(0), m_isHost(false)
{
    ConfigSet();
    setNEtPackFunMap();

    m_mainDialog = new MainDialog;
    connect( m_mainDialog, SIGNAL(SIG_close()),
             this, SLOT(DestroyInstance()));
    connect( m_mainDialog, SIGNAL(SIG_joinZone(int)),
             this, SLOT(slot_joinZone(int)));
    //m_mainDialog->show();

    m_loginDialog=new LoginDialog;
    connect(m_loginDialog, SIGNAL(SIG_close()), this, SLOT(DestroyInstance));
    connect(m_loginDialog, SIGNAL(SIG_loginCommit(QString, QString)),
            this, SLOT(slot_loginCommit(QString, QString)));
    connect(m_loginDialog, SIGNAL(SIG_registerCommit(QString, QString, QString)),
            this, SLOT(slot_registerCommit(QString, QString, QString)));
    m_loginDialog->show();

    m_fiveInLineZone = new FiveInLineZone;
    connect(m_fiveInLineZone, SIGNAL(SIG_joinRoom(int)),
            this, SLOT(slot_joinRoom(int)));
    connect(m_fiveInLineZone, SIGNAL(SIG_close()),
            this, SLOT(slot_leaveZone()));

    m_roomDialog = new RoomDialog;
    connect(m_roomDialog, SIGNAL(SIG_close()),
            this, SLOT(slot_leaveRoom()));
    connect(m_roomDialog, SIGNAL(SIG_gameReady(int,int,int)),
            this, SLOT(slot_fil_gameReady(int,int,int)));
    connect(m_roomDialog, SIGNAL(SIG_gameStart(int,int)),
            this, SLOT(slot_fil_gameStart(int,int)));
    connect(m_roomDialog, SIGNAL(SIG_pieceDown(int,int,int)),
            this, SLOT(slot_fil_pieceDown(int,int,int)));
    connect(m_roomDialog, SIGNAL(SIG_playerWin(int)),
            this, SLOT(slot_fil_win(int)));
    connect(m_roomDialog, SIGNAL(SIG_playByCpuBegin(int,int,int)),
            this, SLOT(slot_fil_playByCpuBegin(int,int,int)));
    connect(m_roomDialog, SIGNAL(SIG_playByCpuEnd(int,int,int)),
            this, SLOT(slot_fil_playByCpuEnd(int,int,int)));
    //m_roomDialog->show();

    m_client = new TcpClientMediator;
    m_client->OpenNet(m_serverIP, _DEF_TCP_PORT);
    connect( m_client, SIGNAL(SIG_ReadyData(uint, char*, int)),
             this, SLOT(slot_ReadyData(uint, char*, int)) );

    // 模拟连接服务器 发送数据包
//    STRU_LOGIN_RQ rq;
//    m_client->SendData(0,(char*)&rq,sizeof(rq));

    connect(&m_rqTimer, SIGNAL(timeout()),
            this, SLOT(slot_roomInfoInZone()));
}
