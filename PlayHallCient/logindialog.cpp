#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QRegExp>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    setUI();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::setUI()
{
    this->setWindowTitle("登录&注册");
    // 加载图片 资源路径 .qrc文件下 :/ 根目录
    QPixmap pixmap(":/images/background.jpg");
    // 画板 添加背景
    QPalette pal(this->palette());
    pal.setBrush(QPalette::Background, pixmap);
    this->setPalette(pal);

    // 默认登录界面
    ui->tw_page->setCurrentIndex(0);
}

// 注册按钮响应
void LoginDialog::on_pb_register_clicked()
{
    // 注册信息的过滤
    QString name = ui->le_name_register->text();
    QString tel = ui->le_tel_register->text();
    QString password = ui->le_password_register->text();
    QString confirm = ui->le_confirme_register->text();

    // 不能为空
    if(name.isEmpty() || tel.isEmpty()
            || password.isEmpty() || confirm.isEmpty()){
        QMessageBox::about(this, "提示", "内容不能为空");
        return;
    }
    // 手机号合法--正则 --> 1 3~8 0~9 9位
    // 创建正则对象
    QRegExp exp("^1[345678][0-9]\{9\}$");
    if(!exp.exactMatch(tel)){
        QMessageBox::about(this, "提示", "手机号不合法");
        return;
    }
    // 密码不能过长
    if(password.length() > 20){
        QMessageBox::about(this, "提示", "密码过长，不能超过20");
        return;
    }
    // 昵称规范 不能全为空格，不能过长
    QString tmpName = name;
    tmpName = tmpName.remove(' '); // remove 操作会改变源字符串
    if(name.length() > 20 || tmpName.isEmpty()){
        QMessageBox::about(this, "提示", "昵称为空或过长(超过20)");
        return;
    }
    // 两次密码一致
    if(password != confirm){
        QMessageBox::about(this, "提示", "两次密码输入不一致");
        return;
    }
    // 发送信号
    Q_EMIT SIG_registerCommit(tel, password, name);
}

// 注册清空按钮响应
void LoginDialog::on_pb_clear_register_clicked()
{
    ui->le_name_register->setText("");
    ui->le_confirme_register->setText("");
    ui->le_password_register->setText("");
    ui->le_tel_register->setText("");
}

// 登录按钮响应
void LoginDialog::on_pb_login_clicked()
{
    // 登录信息的过滤
    QString tel = ui->le_tel->text();
    QString password = ui->le_password->text();

    // 不能为空
    if( tel.isEmpty() || password.isEmpty()){
        QMessageBox::about(this, "提示", "内容不能为空");
        return;
    }
    // 手机号合法--正则 --> 1 3~8 0~9 9位
    // 创建正则对象
    QRegExp exp("^1[2345678][0-9]\{9\}$");
    if(!exp.exactMatch(tel)){
        QMessageBox::about(this, "提示", "手机号不合法");
        return;
    }
    // 密码不能过长
    if(password.length() > 20){
        QMessageBox::about(this, "提示", "密码过长，不能超过20");
        return;
    }

    // 发送信号
    Q_EMIT SIG_loginCommit(tel, password);
}

// 清空按钮响应
void LoginDialog::on_pb_clear_clicked()
{
    ui->le_tel->setText("");
    ui->le_password->setText("");
}

void LoginDialog::closeEvent(QCloseEvent *e)
{
    if(QMessageBox::question(this, "退出", "是否退出")==QMessageBox::Yes){
        // 发信号
        Q_EMIT SIG_close();
        // 同意关闭事件
        e->accept();
    }else{
        // 忽略关闭事件
        e->ignore();
    }
}

