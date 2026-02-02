#include "lobbywidget.h"
#include "ui_lobbywidget.h"
#include "joinroomdialog.h"
#include "hoverwidget.h"
#include "newroomdialog.h"
#include "usermgr.h"
#include "tcpmgr.h"
#include "tipwidget.h"
#include "message.pb.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

LobbyWidget::LobbyWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LobbyWidget)
{
    ui->setupUi(this);
    //连接点击，弹出创建房间对话框
    connect(ui->create_widget,&HoverWidget::clicked,this,&LobbyWidget::slot_create_clicked);

    //连接点击加入房间，弹出加入房间对话框
    connect(ui->join_widget,&HoverWidget::clicked,this,&LobbyWidget::slot_join_clicked);

    //连接创建房间成功，切换canvas页面
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_create_room_finish,this,&LobbyWidget::slot_create_room_finish);

    //连接加入房间成功，切换canvas页面
    connect(TcpMgr::getInstance().get(),&TcpMgr::sig_join_room_finish,this,&LobbyWidget::slot_join_room_finish);

}

LobbyWidget::~LobbyWidget()
{
    delete ui;
}

void LobbyWidget::slot_create_clicked()
{
    NewRoomDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted) //点击的是创建按钮
    {
        QString roomName = dlg.getRoomName();       //获取房间名字
        QSize canvasSize = dlg.getSelectedSize();   //获取用户选择的画布尺寸

        //todo... 发送网络请求
        QJsonObject json_obj;
        json_obj["room_name"] = roomName;                   //房间名
        auto info = UserMgr::getInstance()->getMyInfo();    //房主id
        json_obj["width"] = canvasSize.width();   //画布尺寸
        json_obj["height"] = canvasSize.height();

        if (info)
        {
            json_obj["owner_uid"] = info->_id;
        } else
        {
            qDebug() << "Error: UserInfo is null!";
            return;
        }
        QJsonDocument jsonDoc(json_obj);
        QByteArray jsonString = jsonDoc.toJson();
        TcpMgr::getInstance()->sig_send_data(ReqId::ID_CREAT_ROOM_REQ,jsonString);      //发送TCP包

    }

}

void LobbyWidget::slot_join_clicked()
{
    JoinRoomDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted)
    {
        QString room_id = dlg.getRoomId();

        TipWidget::showTip(ui->draw_label,"正在加入房间...");
        message::JoinRoomReq req;
        req.set_room_id(room_id.toStdString());        //加入的房间号
        req.set_uid(UserMgr::getInstance()->getMyInfo()->_id);  //加入者(自己)

        std::string binaryData;
        if(req.SerializeToString(&binaryData))
        {
            // 将 std::string 转回 Qt 用的 QByteArray
            QByteArray sendData = QByteArray::fromStdString(binaryData);
            emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_JOIN_ROOM_REQ, sendData);
        }
        else
        {
            TipWidget::showTip(ui->draw_label,"加入房间失败");
        }
    }
}

void LobbyWidget::slot_create_room_finish(std::shared_ptr<RoomInfo> room_info)
{
    emit sig_switchCanvas(room_info);
}

void LobbyWidget::slot_join_room_finish(std::shared_ptr<RoomInfo> room_info)
{
    emit sig_switchCanvas(room_info);
}
