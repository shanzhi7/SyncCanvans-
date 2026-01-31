#ifndef NEWROOMDIALOG_H
#define NEWROOMDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class NewRoomDialog;
}

class NewRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewRoomDialog(QWidget *parent = nullptr);
    ~NewRoomDialog();

    QString getRoomName();
    QSize getSelectedSize();

protected:
    void paintEvent(QPaintEvent* event) override;   //重绘事件，绘制背景

private:
    Ui::NewRoomDialog *ui;

    void initBubbles(int count);                    //初始化气泡


    QTimer* bubble_timer;                           //气泡定时器
    std::vector<std::shared_ptr<Bubble>> bubbles;    //气泡数组

private slots:
    void updateBubbles();                           //更新气泡
    void on_creat_btn_clicked();
    void on_cannel_btn_clicked();
};

#endif // NEWROOMDIALOG_H
