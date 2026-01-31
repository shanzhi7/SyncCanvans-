#include "paintscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPathItem>

PaintScene::PaintScene(QObject *parent)
    : QGraphicsScene{parent}
{
    m_currItem = nullptr;
}
void PaintScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 1. 鼠标按下：开始新的一笔
    m_currPath = QPainterPath(); // 重置路径
    m_lastPoint = event->scenePos(); // 【记录】起始点
    m_currPath.moveTo(event->scenePos()); // 移动画笔到鼠标点击的位置

    // 2. 创建一个图元，加到舞台上
    m_currItem = new QGraphicsPathItem();
    m_currItem->setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)); //设置圆角笔触，拐弯更圆润
    m_currItem->setPath(m_currPath);

    addItem(m_currItem); // 【关键】把这一笔加进舞台

}

void PaintScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

    if (!m_currItem)
        return;

    // 只有当按着鼠标移动时才画
    // if (m_currItem) {

    //     // 计算当前点和上一个点的距离
    //     QPointF newPos = event->scenePos();
    //     QPointF lastPos = m_currPath.currentPosition();

    //     qreal distance = QLineF(newPos, lastPos).length();

    //     // 如果移动距离小于 5 像素，直接忽略，不处理！
    //     // 这能大幅减少点的数量，解决“越画越卡”
    //     if (distance < 5.0) {
    //         return;
    //     }

    //     // 1. 路径连线到当前鼠标位置
    //     m_currPath.lineTo(event->scenePos());

    //     // 2. 更新图元的路径，界面会自动刷新
    //     m_currItem->setPath(m_currPath);
    // }


    QPointF currentPoint = event->scenePos();

    qreal dx = currentPoint.x() - m_lastPoint.x();
    qreal dy = currentPoint.y() - m_lastPoint.y();

    if ( (dx * dx + dy * dy) < 25.0 )
    {
        return; // 距离太近，忽略
    }
    // 贝塞尔平滑算法（防折线）
    // 原理：不直接连到 currentPoint，而是连到它和上一个点的“中点”
    // 使用上一个点作为“控制点”来拉扯曲线
    QPointF midPoint = (m_lastPoint + currentPoint) / 2.0;


    m_currPath.quadTo(m_lastPoint, midPoint);

    // 更新显示
    m_currItem->setPath(m_currPath);

    // 【新增】发射移动信号
    // 注意：我们要发给后端的其实是新的目标点。
    // 虽然本地用了贝塞尔优化，但发给后端最简单的做法是发原始点，
    // 让接收端（也就是别人的电脑）也运行一套一样的贝塞尔逻辑即可。
    //emit sigStrokeMove(currentPoint);

    // 更新上一个点
    m_lastPoint = currentPoint;
    m_pointCount++; // 计数

    // 【核心优化】如果当前这笔画已经超过 100 个点
    if (m_pointCount > 100) {
        // 1. 把当前的 Item "封存" (它以后再也不会变了，Qt 渲染它非常快)
        // (不需要做任何操作，只要不再去 setPath 它就是静态的)

        // 2. 马上开启一个新的 Item，无缝衔接
        // 新的路径起点必须是上一段的终点，保证视觉不断开
        QPointF newStart = m_currPath.currentPosition();

        m_currPath = QPainterPath(); // 清空路径
        m_currPath.moveTo(newStart); // 移到断点处

        m_currItem = new QGraphicsPathItem();
        m_currItem->setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        m_currItem->setPath(m_currPath);
        addItem(m_currItem);

        m_pointCount = 0; // 重置计数器
    }
}

void PaintScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // 把最后一段没画完的补上
    if (m_currItem)
    {
        m_currPath.lineTo(event->scenePos());
        m_currItem->setPath(m_currPath);
        m_currItem = nullptr;
    }
    // 【新增】发射结束信号
    //emit sigStrokeEnd();
}
