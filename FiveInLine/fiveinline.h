#ifndef FIVEINLINE_H
#define FIVEINLINE_H

#include <QWidget>
#include<packdef.h>
#include<QStack>


QT_BEGIN_NAMESPACE
namespace Ui { class FiveInLine; }
QT_END_NAMESPACE

// 界面是 580 * 580 固定

// 外边距
#define FIL_MARGIN_HEIGHT   (50)
#define FIL_MARGIN_WIDTH    (50)

// 行列数 五子棋的规范位15*15
#define FIL_COLS            (15)
#define FIL_ROWS            (15)

// 边和边之间的边距
#define FIL_SPACE           (30)

// 棋子大小
#define FIL_PIECE_SIZE      (28)

// 棋盘边缘缩进的距离
#define FIL_DISTANCE        (10)

// 1. 首先绘制棋盘 网格线 棋子
// 2. 鼠标点击，出现棋子，然后鼠标移动棋子跟着移动，鼠标释放，棋子落子
//    需要涉及到鼠标点击 鼠标移动 鼠标释放事件
// 3. 定时刷新界面 1秒25帧  重绘25次
// 4.1落子 出界 要放弃  合法 发送信号
// 4.2落子的槽函数 更新数组 切换回合
// 5. 判断输赢

// 关于输赢
//1.根据当前点,依次查看 左右,上下,左上右下，左下右上,四条直线上的棋子个数
//2.初始棋子个数为1,然后根据方向换算新的坐标,查看是否出界,出界,break,不然看是否和当前棋子同色,如果是继续看,个数+1,不然break
//   如果count到5,结束，没有到5 则继续看其他直线
//3.所有都看完没到5 则游戏没结束

#include <vector>
#include <QPaintEvent>
#include <QTimer>
#include <vector>
using namespace std;

#define DEFAULT_COUNTER 30

// 赢法结构体
struct stru_win{
    stru_win(): board(FIL_COLS, vector<int>(FIL_ROWS, 0))
      , playerCount(0), cpuCount(0){

    }
    void clear(){
        playerCount = 0;
        cpuCount = 0;
    }
    vector< vector<int> >board; //的棋子布局
    int playerCount; //玩家在该赢法的个数统计 一旦玩家落子 该赢法 cpu无法获胜
    int cpuCount; // 计算机在该赢法的个数统计 一旦cpu落子 该赢法 玩家无法获胜
};

class FiveInLine : public QWidget
{
    Q_OBJECT
signals:
    // 落子信号：鼠标释放捕捉落子位置
    void SIG_pieceDown(int blackorwhite, int x, int y); // 什么颜色的棋子 落在什么位置
    void SIG_playerWin(int blackorwhite);

public:
    FiveInLine(QWidget *parent = nullptr);
    ~FiveInLine();

    // 黑白棋子枚举
    enum ENUM_BLSCK_ORWRITE{ None = 0, Black, White };

    // 重绘事件： 绘图背景 棋盘 棋子等
    void paintEvent(QPaintEvent *event);
    // 鼠标点击事件 出现棋子
    void mousePressEvent(QMouseEvent * event);
    // 鼠标移动事件 棋子跟着移动
    void mouseMoveEvent(QMouseEvent * event);
    // 鼠标释放事件 棋子落子
    void mouseReleaseEvent(QMouseEvent * event);
    // 获取当前是谁的回合
    int getblackOrWhite() const;
    // 切换回合
    void changeBlackAndWhite();
    // 判断是否出界
    bool isCrossLine(int x, int y);
    // 判断是否胜利
    bool isWin(int x, int y);
    // 清空
    void clear();

    /* 五子棋 AI */
    // 初始化所有赢法棋子布局
    void InitAiVector();

    // 电脑落子
    void pieceDownByCpu();
    // 设置电脑身份
    void setCpuColor(int newCpuColor);

    // 设置自己身份
    void setSelfStatus(int _status);

    // 悔棋功能
    void undoLastMove(int color, int x, int y);


public slots:
    void slot_pieceDown(int blackorwhite, int x, int y); // 什么颜色的棋子 落在什么位置
    void slot_startGame();
    void slot_countTimer();


private:
    Ui::FiveInLine *ui;
    // 记录当前是黑子白字的回合
    int m_blackOrWhite;
    // 鼠标移动中  鼠标所在位置
    QPoint m_movePoint;
    // 移动标志 标志棋子是否随着鼠标移动，也就是鼠标是不是一直在点击的状态
    bool m_moveFlag;


    // 判断输赢时需要的方向
    enum enum_direction{d_z,d_y,d_s,d_x,d_zs,d_yx,d_zx,d_ys,d_count}; //在一条线的一组相邻 方便写为循环

    // 存棋盘上棋子的二维数组
    std::vector< std::vector<int> > m_board;
    // 棋子的颜色的数组
    QBrush m_pieceColor[3];
    // 重绘定时器
    QTimer m_timer;
    // 结束标志
    bool m_isOver;

    //根据方向对坐标的偏移 每次是一个单位
    int dx[ d_count ] ={-1, 1, 0, 0, -1, 1, -1, 1};
    int dy[ d_count ] = {0, 0, -1, 1, -1, 1, 1, -1};

    // 网络版本 玩家身份 不是自己的回合不能动
    int m_status;

    /* 五子棋 AI */
    // 赢法数组
    vector< stru_win > m_vecWin;

    // 电脑的回合颜色
    int m_cpuColor;

    /* 回合定时 */
    // 每秒的定时
    QTimer m_countTimer;
    // 剩余时间计数器
    int m_colorCounter; // 初始值 DEFAULT_COUNTER 30



};
#endif // FIVEINLINE_H
