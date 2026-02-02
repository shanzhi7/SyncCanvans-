#ifndef LOBBYWIDGET_H
#define LOBBYWIDGET_H

#include <QWidget>
#include "global.h"

namespace Ui {
class LobbyWidget;
}

class LobbyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LobbyWidget(QWidget *parent = nullptr);
    ~LobbyWidget();

private:
    Ui::LobbyWidget *ui;

signals:
    void sig_switchCanvas(std::shared_ptr<RoomInfo> room_info);             //发送切换画布页面

private slots:
    void slot_create_clicked();                                             //点击创建房间窗口
    void slot_join_clicked();                                               //点击加入房间窗口
    void slot_create_room_finish(std::shared_ptr<RoomInfo> room_info);      //创建房间完成槽函数
    void slot_join_room_finish(std::shared_ptr<RoomInfo> room_info);        //加入房间完成槽函数
};

#endif // LOBBYWIDGET_H
