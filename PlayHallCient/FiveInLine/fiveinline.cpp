#include "fiveinline.h"
#include "ui_fiveinline.h"
#include <QPainter>
#include <QMessageBox>

FiveInLine::FiveInLine(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FiveInLine), m_board(FIL_COLS, std::vector<int>(FIL_COLS, 0))
    , m_movePoint(0, 0), m_moveFlag(false), m_blackOrWhite(Black)
    , m_isOver(false), m_status(Black), m_colorCounter(DEFAULT_COUNTER)
    //vector 初始化 有参构造 参数： 多长 初值多少
{
    ui->setupUi(this);

    InitAiVector();

    m_pieceColor[None] = QBrush(QColor(0, 0, 0, 0)); // 最后一个是0 代表全透明
    m_pieceColor[Black] = QBrush(QColor(0, 0, 0));
    m_pieceColor[White] = QBrush(QColor(255, 255, 255));

    // 测试样例
//    m_board[1][0] = Black;
//    m_board[2][0] = White;
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(repaint()));
    // repain() 会触发重绘
    m_timer.start(1000/60);

    // 连接落子和落子处理 通过kernel 转发了
//    connect(this, SIGNAL(SIG_pieceDown(int, int, int))
//            , this, SLOT(slot_pieceDown(int, int, int)));

    clear();
    // slot_startGame();

    connect(&m_countTimer, SIGNAL(timeout()),
            this, SLOT(slot_countTimer()));
}

FiveInLine::~FiveInLine()
{
    m_timer.stop();
    delete ui;
}

// 重绘事件： 绘图背景 棋盘 棋子等
void FiveInLine::paintEvent(QPaintEvent *event)
{
    // 绘制棋盘
    QPainter painer(this); // QPainter 用于绘图，使用有参构造，传入当前对象，得到当前控件的画布
    painer.setBrush(QBrush(QColor(225, 160, 0))); // 通过画刷设置颜色
    painer.drawRect(FIL_MARGIN_WIDTH - FIL_DISTANCE,
                    FIL_MARGIN_HEIGHT - FIL_DISTANCE,
                    (FIL_COLS + 1) * FIL_SPACE + 2 * FIL_DISTANCE,
                    (FIL_ROWS + 1) * FIL_SPACE + 2 * FIL_DISTANCE);

    // 绘制网格线
    // | 竖线
    for(int i = 1; i <= FIL_COLS; ++i){
        painer.setBrush(QBrush(QColor(0, 0, 0))); // 通过画刷设置颜色
        QPoint p1(FIL_MARGIN_WIDTH + FIL_SPACE * i, FIL_MARGIN_HEIGHT + FIL_SPACE);
        QPoint p2(FIL_MARGIN_WIDTH + FIL_SPACE * i, FIL_MARGIN_HEIGHT + (FIL_ROWS)*FIL_SPACE);
        painer.drawLine(p1, p2);

    }
    // -- 横线
    for(int i = 1; i <= FIL_COLS; ++i){
        painer.setBrush(QBrush(QColor(0, 0, 0))); // 通过画刷设置颜色
        QPoint p1(FIL_MARGIN_WIDTH + FIL_SPACE, FIL_MARGIN_HEIGHT + FIL_SPACE * i);
        QPoint p2(FIL_MARGIN_WIDTH + (FIL_COLS)*FIL_SPACE, FIL_MARGIN_HEIGHT + FIL_SPACE * i);
        painer.drawLine(p1, p2);

    }

    // 画中心点
    painer.setBrush(QBrush(QColor(0, 0, 0)));
    painer.drawEllipse(
    QPoint(290, 290),
                6/2, 6/2
                );// 画圆 中心点坐标 半径x,y

    // 绘制棋子
    // x,y点对应的颜色 --> m_pieceColor[m_board[x][y]]
    for(int x = 0; x < FIL_COLS; x++){
        for(int y = 0; y < FIL_ROWS; y++){
            if(m_board[x][y] != None){
                painer.setBrush(m_pieceColor[m_board[x][y]]);
                painer.drawEllipse(
                QPoint(FIL_MARGIN_WIDTH + FIL_SPACE + FIL_SPACE * x,
                       FIL_MARGIN_HEIGHT + FIL_SPACE + FIL_SPACE * y),
                            FIL_PIECE_SIZE/2, FIL_PIECE_SIZE/2
                            );// 画圆 中心点坐标 半径x,y
            }
        }
    }

    // 显示移动的棋子 当前的回合是谁 就画谁
    if(m_moveFlag){
        //m_movePoint
        painer.setBrush(m_pieceColor[getblackOrWhite()]);
        painer.drawEllipse(
        QPoint(m_movePoint.x(),m_movePoint.y()),
                    FIL_PIECE_SIZE/2, FIL_PIECE_SIZE/2
                    );// 画圆 中心点坐标 半径x,y
    }



    event->accept();
}

// 鼠标点击事件 出现棋子
void FiveInLine::mousePressEvent(QMouseEvent * event)
{
    // 捕捉点 --> 相对坐标
    m_movePoint = event->pos();
    // 位置坐标
    // 相对坐标 （相对窗口的）
    // QMouseEvent::pos();
    // 绝对坐标 （相对windows桌面的）
    // QCursor::pos(); 鼠标位置
    // QMouseEvent::globalPos();

    // 加入结束判断
    if(m_isOver) goto quit;
    if(m_status != getblackOrWhite()) goto quit;
    // 点击状态
    m_moveFlag = true;

quit:
    event->accept();
}
// 鼠标移动事件 棋子跟着移动
void FiveInLine::mouseMoveEvent(QMouseEvent * event)
{
    // 捕捉点 --> 相对坐标
    if(m_moveFlag)
        m_movePoint = event->pos();

    event->accept();
}
// 鼠标释放事件 棋子落子
void FiveInLine::mouseReleaseEvent(QMouseEvent * event)
{
    m_moveFlag = false;

    // 落子 出界 要放弃  合法 发送信号
    // 将相对坐标换算为棋盘坐标
    // 会涉及到取整和四舍五入的问题
    // 落子是四舍五入而不是取整
    int x = 0;
    int y = 0;
    float fx = (float)event->pos().x();
    float fy = (float)event->pos().y();

    if(m_isOver) goto quit;
    if(m_status != getblackOrWhite()) goto quit;

    fx = (fx - FIL_MARGIN_WIDTH - FIL_SPACE) / FIL_SPACE;
    fy = (fy - FIL_MARGIN_HEIGHT - FIL_SPACE) / FIL_SPACE;
    // 类似于 4.4 4.5 4.6 根据 <0.5 和 >0.5 分为两个情况
    x = (fx - (int)fx) > 0.5 ? ((int)fx + 1) : (int)fx;
    y = (fy - (int)fy) > 0.5 ? ((int)fy + 1) : (int)fy;

    // 是否越界
    if(isCrossLine(x, y)) return;
    Q_EMIT SIG_pieceDown(getblackOrWhite(), x, y);

quit:
    event->accept();
}

int FiveInLine::getblackOrWhite() const
{
    return m_blackOrWhite;
}

void FiveInLine::changeBlackAndWhite()
{
    // 0, 1, 2
    m_blackOrWhite = m_blackOrWhite + 1;
    if(m_blackOrWhite == 3){
        m_blackOrWhite = Black;
    }
    if(m_blackOrWhite == Black){
        ui->lb_color->setText("黑子回合");
    }
    else{
        ui->lb_color->setText("白子回合");
    }
}

// 判断是否出界
bool FiveInLine::isCrossLine(int x, int y)
{
    if(x < 0 || x >= 15 || y < 0 || y >= 15) return true;
    return false;
}

bool FiveInLine::isWin(int x, int y)
{
    // 看四条直线 八个方向上同色棋子个数 只要有一个数到5 就结束
    int count = 1;
    // 循环看四条线
    for(int dr = d_z; dr < d_count; dr += 2){
        // 先看一条线 <- ->
        // <-
        for(int i = 1; i <= 4; i++){
            // 获取偏移后棋子坐标
            int ix = dx[dr]*i + x;
            int iy = dy[dr]*i + y;
            // 判断是否出界
            if(isCrossLine(ix, iy)) break;
            // 看是否同色
            if(m_board[ix][iy] == m_board[x][y]){
                count++;
            }else break;
        }

        // ->
        for(int i = 1; i <= 4; i++){
            // 获取偏移后棋子坐标
            int ix = dx[dr + 1]*i + x;
            int iy = dy[dr + 1]*i + y;
            // 判断是否出界
            if(isCrossLine(ix, iy)) break;
            // 看是否同色
            if(m_board[ix][iy] == m_board[x][y]){
                count++;
            }else break;
        }
        if(count >= 5) break; // 不再看其它直线
        else count = 1;  // 不够，要看其它直线
    }

    if(count >= 5){
        // 结束
        m_isOver = true;
        return true;
    }
    return false;

}

void FiveInLine::clear()
{
    // 默认不开AI 开的话Black or White
    setCpuColor(None);
    // 清空赢法棋子个数统计
    for(int i = 0; i < m_vecWin.size(); ++i){
        m_vecWin[i].clear();
    }
    // 状态位
    m_isOver = true;
    m_blackOrWhite = Black;
    m_moveFlag = false;

    // 界面
    for(int x = 0; x < FIL_COLS; ++x){
        for(int y = 0; y < FIL_ROWS; ++y){
            m_board[x][y] = None;
        }
    }
    ui->lb_winner->setText("");
    ui->lb_color->setText("黑子回合");

    ui->lb_timer->hide();
}

void FiveInLine::setSelfStatus(int _status)
{
    m_status = _status;
}

void FiveInLine::slot_pieceDown(int blackorwhite, int x, int y)
{
    // 更新数组 切换回合
    if(blackorwhite != getblackOrWhite()) return;
    // 落子 没有子才能落子
    if(m_board[x][y] == None){
        m_board[x][y] = blackorwhite;

        // 更新棋子的颜色 判断输赢
        if(isWin(x, y)){
            QString str = getblackOrWhite() == Black? "黑方胜" : "白方胜";
            ui->lb_winner->setText(str);

            m_countTimer.stop();
            ui->lb_timer->hide();

            Q_EMIT SIG_playerWin(getblackOrWhite());
            //QMessageBox::about(this, "结束", str);
        }else{
            if(m_cpuColor != getblackOrWhite()){
                // 更新玩家的每种赢法 玩家的棋子个数
                for(int i = 0; i < m_vecWin.size(); i ++){
                    if(m_vecWin[i].board[x][y] == 1){
                        m_vecWin[i].playerCount += 1;
                        m_vecWin[i].cpuCount = 100; // 无效的值
                    }
                }
            }else{
                // 统计个数要更新
                for(int k = 0; k < m_vecWin.size(); ++k){
                    if( m_vecWin[k].board[x][y] == 1 ){
                        m_vecWin[k].cpuCount += 1;
                        m_vecWin[k].playerCount = 100;
                    }
                }
            }
            m_colorCounter = DEFAULT_COUNTER;
            // 更换回合
            changeBlackAndWhite();
            // 判断是否是电脑的回合 电脑下棋
            if(m_cpuColor == getblackOrWhite())
                pieceDownByCpu();
        }

    }
}

void FiveInLine::slot_startGame()
{
    clear();
    m_isOver = false;

    m_colorCounter = DEFAULT_COUNTER;
    ui->lb_timer->show();
    m_countTimer.start(1000);
}

/*  电脑AI   */
// 初始化所有赢法棋子布局
void FiveInLine::InitAiVector()
{
    // - 赢法
    for(int y = 0; y < FIL_ROWS; y++){
        for(int x = 0; x < FIL_COLS - 4; x++){// 起点位置
            stru_win w;
            for(int k = 0; k < 5; k++){
                // ( (x+k) , y )
                w.board[x + k][y] = 1;
            }
            m_vecWin.push_back(w);
        }
    }
    // | 赢法
    for(int x = 0; x < FIL_COLS; x++){
        for(int y = 0; y < FIL_ROWS - 4; y++){// 起点位置
            stru_win w;
            for(int k = 0; k < 5; k++){
                // ( (x+k) , y )
                w.board[x][y + k] = 1;
            }
            m_vecWin.push_back(w);
        }
    }
    // / 赢法
    for( int x = FIL_COLS - 1; x >= 4; x-- ){
        for(int y = 0; y < FIL_ROWS - 4; y++){// 起点位置
            stru_win w;
            for(int k = 0; k < 5; k++){
                // ( (x+k) , y )
                w.board[x - k][y + k] = 1;
            }
            m_vecWin.push_back(w);
        }
    }
    // \ 赢法
    for(int x = 0 ; x < FIL_COLS - 4; x++){
        for(int y = 0; y < FIL_ROWS - 4; y++){// 起点位置
            stru_win w;
            for(int k = 0; k < 5; k++){
                // ( (x+k) , y )
                w.board[x + k][y + k] = 1;
            }
            m_vecWin.push_back(w);
        }
    }
}

// 电脑落子：根据每种赢法棋子的个数 给每个无子的位置估分 得到所有的最优值 然后得到一个坐标
void FiveInLine::pieceDownByCpu()
{
    if(m_isOver) return;

    if(m_cpuColor != getblackOrWhite()) return;

    int x = 0, y = 0, k = 0;
    int MyScore[FIL_COLS][FIL_ROWS] = {};// 记录玩家所有位置的分数
    int CpuScore[FIL_COLS][FIL_ROWS] = {};// 记录电脑所有位置的分数

    int u = 0, v = 0; // 最优目标坐标
    int max = 0; // 最高分



    // 估分 找到没有子的点，看每种赢法对应的棋子个数 进行这个点的分数计算
    for(x = 0; x < FIL_COLS; ++x){
        for(y = 0; y < FIL_ROWS; ++y){
            if(m_board[x][y] == None){
                // 遍历所有赢法
                for(k = 0; k < m_vecWin.size(); ++k){
                    // 评估玩家在 x, y 点分数
                    if(m_vecWin[k].board[x][y] == 1){
                        // 根据该赢法 棋子个数
                        switch (m_vecWin[k].playerCount) {
                        case 1:
                            MyScore[x][y] += 200;
                            break;
                        case 2:
                            MyScore[x][y] += 400;
                            break;
                        case 3:
                            MyScore[x][y] += 2000;
                            break;
                        case 4:
                            MyScore[x][y] += 10000;
                            break;
                        }
                    }
                    // 评估电脑在 x, y 点分数
                    if(m_vecWin[k].board[x][y] == 1){
                        // 根据该赢法 棋子个数
                        switch (m_vecWin[k].cpuCount) {
                        case 1:
                            MyScore[x][y] += 220;
                            break;
                        case 2:
                            MyScore[x][y] += 420;
                            break;
                        case 3:
                            MyScore[x][y] += 4100;
                            break;
                        case 4:
                            MyScore[x][y] += 200000;
                            break;
                        }
                    }
                }
            }
        }
    }

    int a = 0, b = 0;
    int maxCpu = 0 ;

    // 估分之后找最优点 一定是没有子的点
    for(x = 0; x < FIL_COLS; ++x){
        for(y = 0; y < FIL_ROWS; ++y){
            if(m_board[x][y] == None){
//                // 先看玩家 MyScore看作是防守 CpuScore看作是进攻
//                if(MyScore[x][y] > max){
//                    max = MyScore[x][y];
//                    u = x;
//                    v = y;
//                }else if(MyScore[x][y] == max){ // 最高值可能不止一个，看攻击最高的
//                    if(CpuScore[x][y] > CpuScore[u][v]){
//                        u = x;
//                        v = y;
//                    }
//                }
//                // 再看电脑
//                if(CpuScore[x][y] > max){
//                    max = CpuScore[x][y];
//                    u = x;
//                    v = y;
//                }else if(CpuScore[x][y] == max){
//                    if(MyScore[x][y] > MyScore[u][v]){
//                        u = x;
//                        v = y;
//                    }
//                }

               if(MyScore[x][y] > max){
                   max = MyScore[x][y];
                   u = x;
                   v = y;
               }
               if(CpuScore[x][y] > max){
                   maxCpu = CpuScore[x][y];
                   a = x;
                   b = y;
               }
            }
        }
    }
    if(max < maxCpu){
         u = a;
         v = b;
    }

//    // 统计个数要更新
//    for( k = 0; k < m_vecWin.size(); ++k){
//        if( m_vecWin[k].board[u][v] == 1 ){
//            m_vecWin[k].cpuCount += 1;
//            m_vecWin[k].playerCount = 100;
//        }
//    }

    // 得到最优目标坐标


    Q_EMIT SIG_pieceDown(getblackOrWhite(), u, v);
}

// 每秒更新一次
void FiveInLine::slot_countTimer()
{
    m_colorCounter--;
    if(m_colorCounter <= 0){
        // 界面文本更新
        ui->lb_timer->setText(QString("%1秒").arg(m_colorCounter, 2, 10, QChar('0')));
        // 切换回合
        changeBlackAndWhite();
        m_colorCounter = DEFAULT_COUNTER;
        return;
    }

    // 界面文本更新
    ui->lb_timer->setText(QString("%1秒").arg(m_colorCounter, 2, 10, QChar('0')));
}

void FiveInLine::setCpuColor(int newCpuColor)
{
    m_cpuColor = newCpuColor;
}

