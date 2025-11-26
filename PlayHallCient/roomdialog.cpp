#include "roomdialog.h"
#include "ui_roomdialog.h"
#include <QMessageBox>
#include "packdef.h"

RoomDialog::RoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoomDialog), m_roomid(0), m_status(_player)
{
    ui->setupUi(this);

    ui->pb_start->setEnabled(false);
    // 关于开局的按键 可以是kernel判断 然后给room发送 可以点击开局

    // 点击开局 返回开局的信号

    // 五子棋类 发送的信号 转发给kernel
    connect(ui->wdg_play, SIGNAL(SIG_pieceDown(int,int,int)),
            this, SIGNAL(SIG_pieceDown(int,int,int)));
    connect(ui->wdg_play, SIGNAL(SIG_playerWin(int)),
            this, SIGNAL(SIG_playerWin(int)));
}

RoomDialog::~RoomDialog()
{
    delete ui;
}

void RoomDialog::setInfo(int roomid)
{
    m_roomid = roomid;

    QString txt = QString("五子棋-%1房").arg(roomid, 2, 10, QChar('0'));
    this->setWindowTitle(txt);
}

void RoomDialog::setUserStatus(int status)
{
    m_status = status;

    ui->wdg_play->setSelfStatus(m_status ==
                                _host ? FiveInLine::Black : FiveInLine::White);
}

// 设置房主信息
void RoomDialog::setHostInfo(int id, QString name)
{
    ui->lb_player1_name->setText(name);
    ui->lb_icon_player1->setPixmap(QPixmap(":/icon/avatar_10.png")
                                   .copy(0, 0, 230, 280));
    m_roomUserLisst.push_back(id);
}

// 设置玩家信息
void RoomDialog::setPlayerInfo(int id, QString name)
{
    ui->lb_player2_name->setText(name);
    ui->lb_icon_player2->setPixmap(QPixmap(":/icon/avatar_10.png")
                                   .copy(0, 0, 230, 280));
    m_roomUserLisst.push_back(id);
}

void RoomDialog::setHostPlayByCpu(bool yes)
{
    if(yes){
        ui->pb_player1_cpu->setText("托管中");
    }else{
        ui->pb_player1_cpu->setText("托管");
    }
}

void RoomDialog::setPlayerPlayByCpu(bool yes)
{
    if(yes){
        ui->pb_player2_cpu->setText("托管中");
    }else{
        ui->pb_player2_cpu->setText("托管");
    }
}

void RoomDialog::clearRoom()
{
    // ui
    ui->lb_player1_name->setText("");
    ui->lb_player2_name->setText("");
    ui->lb_icon_player1->setPixmap(QPixmap(":/icon/slotwait.png"));
    ui->lb_icon_player2->setPixmap(QPixmap(":/icon/slotwait.png"));

    // 游戏界面清空
    ui->pb_start->setEnabled(false);
    ui->pb_player1_ready->setEnabled(true);
    ui->pb_player1_ready->setChecked(false);
    ui->pb_player2_ready->setEnabled(true);
    ui->pb_player2_ready->setChecked(false);
    ui->pb_player1_ready->setText("准备");
    ui->pb_player2_ready->setText("准备");

    ui->pb_player1_cpu->setChecked(false);
    ui->pb_player2_cpu->setChecked(false);
    ui->pb_player1_cpu->setText("托管");
    ui->pb_player2_cpu->setText("托管");

    // 聊天窗口清空

    // 后台数据
    m_roomid = 0;
    m_roomUserLisst.clear();
    m_status = _player;
}

void RoomDialog::resetAllPushButton()
{
    ui->pb_start->setEnabled(false);
    ui->pb_player1_ready->setEnabled(true);
    ui->pb_player1_ready->setChecked(false);
    ui->pb_player2_ready->setEnabled(true);
    ui->pb_player2_ready->setChecked(false);
    ui->pb_player1_ready->setText("准备");
    ui->pb_player2_ready->setText("准备");

    ui->pb_player1_cpu->setChecked(false);
    ui->pb_player2_cpu->setChecked(false);
    ui->pb_player1_cpu->setText("托管");
    ui->pb_player2_cpu->setText("托管");

}

void RoomDialog::playerLeave(int id)
{
    // ui
    ui->lb_player2_name->setText("");
    ui->lb_icon_player2->setPixmap(QPixmap(":/icon/slotwait.png"));
    // data
    for(auto ite = m_roomUserLisst.begin(); ite != m_roomUserLisst.end(); ++ite){
        if(id == *ite){
            ite = m_roomUserLisst.erase(ite);
            break;
        }
    }
}

// 离开房间
void RoomDialog::closeEvent(QCloseEvent *event)
{
    if( QMessageBox::question( this, "退出房间", "是否退出房间？")
                           == QMessageBox::Yes ){
        Q_EMIT SIG_close();
        event->accept();
    }else{
        event->ignore();
    }
}

void RoomDialog:: setPlayerReady(int id)
{
    if(m_roomUserLisst.front() == id){
        ui->pb_player1_ready->setChecked(true);
        ui->pb_player1_ready->setText("已准备");
    }
    if(m_roomUserLisst.back() == id && m_roomUserLisst.size() == 2){
        ui->pb_player2_ready->setChecked(true);
        ui->pb_player2_ready->setText("已准备");
    }
    if(ui->pb_player1_ready->isChecked() && ui->pb_player2_ready->isChecked()){
        // 都准备了
        if(m_status == _host){
            ui->pb_start->setEnabled(true);
        }

    }
}

// 设置开局
void RoomDialog::setGameStart()
{
    // 开始 准备 都不能点
    ui->pb_player1_ready->setEnabled(false);
    ui->pb_player2_ready->setEnabled(false);
    ui->pb_start->setEnabled(false);
    // 五子棋开始操作
    ui->wdg_play->slot_startGame();
}

void RoomDialog::setChatCoTent(QString content)
{

}


void RoomDialog::on_pb_player1_ready_clicked(bool checked)
{
    // 点击准备按钮
    // 验证是不是自己点击
    if(m_status != _host) return;
    // 是 切换状态
    if(ui->pb_player1_ready->isChecked()){
        //ui->pb_player1_ready->setText("已准备");
        // 发送信号
        Q_EMIT SIG_gameReady(0x10, m_roomid, m_roomUserLisst.front());
    }else{
        ui->pb_player1_ready->setText("准备");
    }

}


void RoomDialog::on_pb_player2_ready_clicked(bool checked)
{
    // 验证是不是自己点击
    if(m_status != _player) return;
    // 是 切换状态
    if(ui->pb_player2_ready->isChecked()){
        //ui->pb_player2_ready->setText("已准备");
        // 发送信号
        Q_EMIT SIG_gameReady(0x10, m_roomid, m_roomUserLisst.back());
    }else{
        ui->pb_player2_ready->setText("准备");
    }
}

// 点击开局
void RoomDialog::on_pb_start_clicked()
{
    if(m_status != _host) return;

    Q_EMIT SIG_gameStart(0x10, m_roomid);
}

void RoomDialog::slot_pieceDoown(int blackorwhite, int x, int y)
{
    ui->wdg_play->slot_pieceDown(blackorwhite, x, y);
}

// 托管 第一次点击是托管 第二次点击是取消托管
void RoomDialog::on_pb_player1_cpu_clicked(bool checked)
{
    // 判断是否为自己 然后再执行
    if(m_status == _host){
        if(ui->pb_player1_cpu->isChecked()){
            // 首先 托管通过网络发送 需要一个信号
            Q_EMIT SIG_playByCpuBegin(0x10, m_roomid, m_roomUserLisst.front());
            // 托管信息发送到服务器 再转回来 然后改变文字 托管->托管中 -> 不在这里完成

            // 电脑自动下棋
            ui->wdg_play->setCpuColor(FiveInLine::Black);
            // 调用一次电脑下棋
            ui->wdg_play->pieceDownByCpu();
        }else{
            // 首先 托管通过网络发送 需要一个信号
            Q_EMIT SIG_playByCpuBegin(0x10, m_roomid, m_roomUserLisst.front());
            // 托管信息发送到服务器 再转回来 然后改变文字 托管->托管中

            // 电脑停止下棋
            ui->wdg_play->setCpuColor(FiveInLine::None);
        }
    }
}


void RoomDialog::on_pb_player2_cpu_clicked(bool checked)
{
    if(m_roomUserLisst.size() != 2) return;
    // 判断是否为自己 然后再执行
    if(m_status == _player){
        if(ui->pb_player2_cpu->isChecked()){
            // 首先 托管通过网络发送 需要一个信号
            Q_EMIT SIG_playByCpuBegin(0x10, m_roomid, m_roomUserLisst.back());
            // 托管信息发送到服务器 再转回来 然后改变文字 托管->托管中 -> 不在这里完成

            // 电脑自动下棋
            ui->wdg_play->setCpuColor(FiveInLine::White);
            // 调用一次电脑下棋
            ui->wdg_play->pieceDownByCpu();
        }else{
            // 首先 托管通过网络发送 需要一个信号
            Q_EMIT SIG_playByCpuBegin(0x10, m_roomid, m_roomUserLisst.back());
            // 托管信息发送到服务器 再转回来 然后改变文字 托管->托管中

            // 电脑停止下棋
            ui->wdg_play->setCpuColor(FiveInLine::None);
        }
    }
}


void RoomDialog::on_pb_send_clicked()
{

}

