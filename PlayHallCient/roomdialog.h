#ifndef ROOMDIALOG_H
#define ROOMDIALOG_H

#include <QDialog>
#include <QCloseEvent>
namespace Ui {
class RoomDialog;
}

class RoomDialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_close();
    // 准备和开局
    void SIG_gameReady(int zoneid, int roomid, int userid);
    void SIG_gameStart(int zoneid, int roomid);
    // 落子信号
    void SIG_pieceDown(int blackorwhite, int x, int y);
    void SIG_playerWin(int blackorwhite);
    // 电脑托管
    void SIG_playByCpuBegin(int zoneid, int roomid, int userid);
    void SIG_playByCpuEnd(int zoneid, int roomid, int userid);

    // 聊天功能
    void SIG_chat(int zoneid, int roomid, int userid);

public:
    explicit RoomDialog(QWidget *parent = nullptr);
    ~RoomDialog();

    void setInfo(int roomid);
    void setUserStatus(int status);
    void setHostInfo(int id, QString name);
    void setPlayerInfo(int id, QString name);
    // 用于kernel调用的
    void setHostPlayByCpu(bool yes);
    void setPlayerPlayByCpu(bool yes);
    void clearRoom();
    void resetAllPushButton();
    void playerLeave(int id);

    void closeEvent(QCloseEvent *event);

    void setPlayerReady(int id);
    // CKernel 调用
    void setGameStart();

    //设置聊天请求内容到窗口上
    void setChatCoTent(QString content);

private slots:
    void on_pb_player1_ready_clicked(bool checked);

    void on_pb_player2_ready_clicked(bool checked);

    void on_pb_start_clicked();

    void on_pb_player1_cpu_clicked(bool checked);

    void on_pb_player2_cpu_clicked(bool checked);

    void on_pb_send_clicked();

public  slots:
    void slot_pieceDoown(int blackorwhite, int x, int y);

private:
    Ui::RoomDialog *ui;
    int m_roomid;
    std::list<int> m_roomUserLisst;
    int m_status;
};

#endif // ROOMDIALOG_H
