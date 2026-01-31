#include "newroomdialog.h"
#include "ui_newroomdialog.h"
#include <QPainter>
#include <QPaintEvent>
#include <QRandomGenerator>
#include <QTimer>

NewRoomDialog::NewRoomDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewRoomDialog)
{
    ui->setupUi(this);

    //初始化背景 (begin)
    bubble_timer = new QTimer(this);

    // 去掉窗口标题栏和边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    // 设置窗口背景透明，配合圆角
    setAttribute(Qt::WA_TranslucentBackground);

    //连接一定时间间隔更新背景
    connect(bubble_timer, &QTimer::timeout, this, &NewRoomDialog::updateBubbles);

    bubble_timer->start(16);   //帧率
    initBubbles(50);           //初始化气泡
    setWindowTitle("创建房间");
    //初始化背景 (end)

    connect(this,&NewRoomDialog::finished,bubble_timer,&QTimer::stop);  //窗口关闭停止定时器

    //初始化 QComboBox的Item (begin)
    ui->canvasSize_cbx->setItemData(0,QSize(1920, 1080));   //默认
    ui->canvasSize_cbx->setItemData(1,QSize(2480, 3508));   //A4
    ui->canvasSize_cbx->setItemData(2,QSize(5000,5000));    //超大画布
    //初始化 QComboBox的Item (end)
}

NewRoomDialog::~NewRoomDialog()
{
    if(bubble_timer != nullptr)
        bubble_timer->stop();
    delete ui;
}

QString NewRoomDialog::getRoomName()        //获取房间名字
{
    return ui->roomName_edit->text();
}

QSize NewRoomDialog::getSelectedSize()      //获取画布大小
{
    return ui->canvasSize_cbx->currentData().toSize();
}

void NewRoomDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QColor background(9, 20, 40);
    // 变暗
    QColor fixedColor = background.darker(150);
    painter.fillRect(this->rect(),fixedColor);

    // 开启抗锯齿，圆形边缘更平滑 (稍吃性能)
    painter.setRenderHint(QPainter::Antialiasing);

    //去除边框
    painter.setPen(Qt::NoPen);

    //绘制气泡
    for(const auto& bubble : bubbles)
    {
        painter.setBrush(bubble->color);
        painter.drawEllipse(QPointF(bubble->x,bubble->y),bubble->radius,bubble->radius);
    }
}

void NewRoomDialog::initBubbles(int count)
{
    bubbles.clear();        //清除
    bubbles.reserve(count);   //预分配容量

    //初始化100个气泡
    for(int i = 0;i < count;i++)
    {
        std::shared_ptr<Bubble> bubble = std::make_shared<Bubble>();

        // 随机初始化位置
        bubble->x = QRandomGenerator::global()->bounded(width());
        bubble->y = QRandomGenerator::global()->bounded(height());

        // 随机大小 (5 ~ 25)
        bubble->radius = QRandomGenerator::global()->bounded(5, 25);

        //随机速度
        bubble->speed = 0.5 + QRandomGenerator::global()->bounded(2.0);

        bubble->h_speed = -0.5 + QRandomGenerator::global()->bounded(2.0);

        // 随机颜色 (带透明度看起来更像气泡/背景)
        int red = QRandomGenerator::global()->bounded(256);
        int green = QRandomGenerator::global()->bounded(256);
        int blue = QRandomGenerator::global()->bounded(256);
        bubble->color = QColor(red, green, blue, 50); //透明度 (0-255)

        //插入数组
        bubbles.push_back(bubble);
    }
}

void NewRoomDialog::updateBubbles()
{
    int w = width();        //窗口宽度
    int h = height();       //窗口高度

    for (auto &bubble : bubbles)
    {
        // 移动位置
        bubble->y -= bubble->speed;
        bubble->x += bubble->h_speed;

        // 边界检测: 如果完全飘出屏幕上方
        if (bubble->y + bubble->radius < 0) {
            // 重置到底部下面一点点
            bubble->y = h + bubble->radius;
            // X轴随机位置
            bubble->x = QRandomGenerator::global()->bounded(w);
        }
    }

    // 触发重绘
    update();
}

void NewRoomDialog::on_creat_btn_clicked()
{
    QDialog::accept();
}


void NewRoomDialog::on_cannel_btn_clicked()
{
    QDialog::reject();
}

