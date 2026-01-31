#ifndef PAINTSCENE_H
#define PAINTSCENE_H

#include <QGraphicsScene>
#include <QObject>
#include <QPainterPath>
#include <QPointF>

class PaintScene : public QGraphicsScene
{
public:
    explicit PaintScene(QObject *parent = nullptr);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // 当前正在画的那一笔
    QGraphicsPathItem *m_currItem;

    // 当前笔画的路径数据
    QPainterPath m_currPath;

    QPointF m_lastPoint;    //上一个坐标

    // 在头文件中增加一个计数器
    int m_pointCount = 0;
};

#endif // PAINTSCENE_H
