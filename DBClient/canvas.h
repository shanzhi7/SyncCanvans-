#ifndef CANVAS_H
#define CANVAS_H

#include "paintscene.h"
#include <QMainWindow>

namespace Ui {
class Canvas;
}

class Canvas : public QMainWindow
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);
    ~Canvas();
protected:
    virtual bool eventFilter(QObject* watched,QEvent* event) override;       //事件过滤器

private slots:
    void on_creatRoom_action_triggered();

private:
    Ui::Canvas *ui;
    PaintScene* _paintScene;     //Scene

    void initCanvasUi();
};

#endif // CANVAS_H
