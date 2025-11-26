#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_close();
    void SIG_joinZone(int zoneid);

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

    void closeEvent(QCloseEvent *e);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainDialog *ui;
};
#endif // MAINDIALOG_H
