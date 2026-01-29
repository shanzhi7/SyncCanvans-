#ifndef CANVAS_H
#define CANVAS_H

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

private:
    Ui::Canvas *ui;
};

#endif // CANVAS_H
