#include "roomitem.h"
#include "ui_roomitem.h"

RoomItem::RoomItem(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoomItem), m_roomid(0)
{
    ui->setupUi(this);

    //setUI();
}

RoomItem::~RoomItem()
{
    delete ui;
}

void RoomItem::setInfo(int roomid)
{
    m_roomid = roomid;

   QString txt = QString("五子棋-%1房").arg(roomid, 2, 10, QChar('0'));
   ui->lb_title->setText(txt);
}

//void RoomItem::setUI()
//{
//    this->setWindowTitle("登录&注册");
//    // 加载图片 资源路径 .qrc文件下 :/ 根目录
//    QPixmap pixmap(":/images/background.jpg");
//    // 画板 添加背景
//    QPalette pal(this->palette());
//    pal.setBrush(QPalette::Background, pixmap);
//    this->setPalette(pal);
//}

void RoomItem::on_pb_join_clicked()
{
    Q_EMIT SIG_JoinRoom(m_roomid);
}

void RoomItem::setRoomItem(int num)
{
    QPixmap ready = QPixmap(":/icon/avatar_10.png").copy(0, 0, 230, 280);
    QPixmap wait = QPixmap(":/icon/slotwait.png");

    switch (num) {
    case 0:
        ui->lb_player1->setPixmap(wait);
        ui->lb_player2->setPixmap(wait);
        break;
    case 1:
        ui->lb_player1->setPixmap(ready);
        ui->lb_player2->setPixmap(wait);
        break;
    case 2:
        ui->lb_player1->setPixmap(ready);
        ui->lb_player2->setPixmap(ready);
        break;
    }
}

