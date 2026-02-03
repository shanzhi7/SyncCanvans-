#include "hoverwidget.h"
#include "global.h"
#include <QGraphicsColorizeEffect>
#include <QStyleOption>
#include <QPainter>
#include <QPropertyAnimation>
#include <QMouseEvent>

HoverWidget::HoverWidget(QWidget *parent)
    : QWidget(parent),_normal(""),_hover(""),_press("")
{
    setState("normal","hover","press");

    m_posInit = false;

    // 初始化动画对象，目标属性是 "pos" (坐标)
    m_animation = new QPropertyAnimation(this, "pos");

    // 动画时间 200ms，短一点会有轻快的感觉
    m_animation->setDuration(200);

    // 使用 OutCubic 曲线，这会让上浮动作有“磁悬浮”般的刹车感
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

HoverWidget::~HoverWidget()
{
    //手动停止动画
    if (m_animation)
    {
        m_animation->stop();
    }
}

void HoverWidget::setState(QString normal, QString hover, QString press)
{
    _normal = normal;
    _hover = hover;
    _press = press;

    setProperty("state",_normal);
    repolish(this);
}


void HoverWidget::enterEvent(QEnterEvent *event)
{
    //设置属性为悬浮
    setProperty("state", _hover);

    repolish(this);

    // 立即打断当前动画（防止之前的动画还没做完）
    m_animation->stop();

    // 起点,必须是【当前】位置，这样动画才连贯，不会闪烁
    m_animation->setStartValue(this->pos());

    // 终点：永远是【原始坐标 - 偏移量】
    // 这样无论你鼠标进进出出多少次，它最终都只会在这个高度
    QPoint targetPos(m_defaultPos.x(), m_defaultPos.y() - FLOAT_OFFSET);
    m_animation->setEndValue(targetPos);

    // 4. 开始动画
    m_animation->start();
    QWidget::enterEvent(event);
}

void HoverWidget::leaveEvent(QEvent *event)
{
    //设置属性为默认
    setProperty("state", _normal);

    repolish(this);

    //打断
    m_animation->stop();

    //起点：当前位置
    m_animation->setStartValue(this->pos());

    //终点：永远是【原始坐标】（回家）
    // 这就是解决“乱飘”的核心，不管它现在在哪，最后必须回到 m_defaultPos
    m_animation->setEndValue(m_defaultPos);

    // 4. 开始
    m_animation->start();

    QWidget::leaveEvent(event);
}

void HoverWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 第一次显示时，记录下布局管理器分配给它的位置
    if (!m_posInit)
    {
        m_defaultPos = this->pos();
        m_posInit = true;
    }
}

void HoverWidget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);

    // 关键逻辑：防止动画自己触发 moveEvent 导致坐标记录错误
    // 只有当动画【停止】时，发生的移动才是因为窗口缩放或布局改变引起的
    // 这时候我们需要更新“老家”的坐标
    if (m_animation->state() == QAbstractAnimation::Stopped)
    {
        m_defaultPos = this->pos();
    }
}

void HoverWidget::mousePressEvent(QMouseEvent *event)
{
    // 只响应左键
    if (event->button() == Qt::LeftButton)
    {
        //setProperty("state", _press); // 切换为按下样式
        style()->unpolish(this);
        style()->polish(this);
    }

    QWidget::mousePressEvent(event);
}

void HoverWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        //点击窗口发送点击信号
        if (rect().contains(event->pos()))
        {
            emit clicked(); // 发送信号！

            // 松开后，如果鼠标还在控件里，应该变回 hover 状态
            setProperty("state", _hover);
        }
        else
        {
            // 如果鼠标移出去了才松开，变回 normal 状态
            setProperty("state", _normal);
        }

        style()->unpolish(this);
        style()->polish(this);
    }

    QWidget::mouseReleaseEvent(event);
}

void HoverWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //开启抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);  //开启抗锯齿
    //让样式表以及子控件的样式表生效
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
