/***********************************************************************************
*
* @file         hoverwidget.h
* @brief        欢迎页面的子widegt
*
* @author       shanzhi
* @date         2026/01/20
* @history
***********************************************************************************/
#ifndef HOVERWIDGET_H
#define HOVERWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

class HoverWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HoverWidget(QWidget *parent = nullptr);
    ~HoverWidget();
    void setState(QString normal,QString hover,QString _press);     //初始化成员变量


protected:
    virtual void enterEvent(QEnterEvent* event) override;           //鼠标进入事件
    virtual void leaveEvent(QEvent* event) override;                //鼠标离开事件
    virtual void mousePressEvent(QMouseEvent *event) override;      //鼠标点击事件
    virtual void mouseReleaseEvent(QMouseEvent *event) override;    //鼠标释放事件
    virtual void paintEvent(QPaintEvent *event) override;           //重绘事件

    // 控件显示事件（用于获取初始坐标）
    virtual void showEvent(QShowEvent *event) override;
    // 控件移动事件（用于处理窗口缩放时的坐标更新）
    void moveEvent(QMoveEvent *event) override;


private:

    QString _normal;
    QString _hover;
    QString _press;

    // 属性动画对象
    QPropertyAnimation *m_animation;
    // 记录控件的“老家”（原始坐标）
    QPoint m_defaultPos;
    // 标记是否已经初始化了坐标
    bool m_posInit;
    // 上浮的距离（像素）
    const int FLOAT_OFFSET = 10;

signals:
    void clicked();          //点击信号
};

#endif // HOVERWIDGET_H
