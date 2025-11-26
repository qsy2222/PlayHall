#ifndef ROOMITEM_H
#define ROOMITEM_H

#include <QDialog>

namespace Ui {
class RoomItem;
}

class CKernel;
class RoomItem : public QDialog
{
    Q_OBJECT

public:
    explicit RoomItem(QWidget *parent = nullptr);
    ~RoomItem();

    void setInfo(int roomid);

signals:
    void SIG_JoinRoom(int id);

//    void setUI();
private slots:
    void on_pb_join_clicked();

    void setRoomItem(int num);

private:
    Ui::RoomItem *ui;
    int m_roomid;

    friend class CKernel;
};

#endif // ROOMITEM_H
