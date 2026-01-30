/***********************************************************************************
*
* @file         loginwidget.h
* @brief        登录窗口
*
* @author       shanzhi
* @date         2026/01/20
* @history
***********************************************************************************/
#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include "TipWidget.h"
#include "global.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

protected:
    virtual QSize sizeHint() const override;

private:
    Ui::LoginWidget *ui;
    QMap<ReqId,std::function<void(QJsonObject)>> _handlers_map;        //服务器回包处理函数存储


    QList<std::function<bool()>> _validators;           // 存储所有的验证函数

    // 通用绑定函数
    void bindValidator(QLineEdit *input, const QString &pattern, const QString &errMsg);

    // 初始化函数
    void initValidator();
    void initHandlerMap();                                              //初始化回包处理函数


signals:
    void switchWelcome();   //发送切换欢迎页面信号
    void switchRegister();  //发送切换注册页面信号
    void switchReset();     //发送切换重置密码页面信号
    void switchCanvas();    //发送切换Canvas页面信号

    void sig_connect_tcp(ServerInfo si);    //发送连接CanvasServer信号
private slots:
    void on_login_btn_clicked();

    void slot_login_mod_finish(ReqId reqid,QString res,ErrorCodes err); // 登录http请求完成，获取token
    void slot_tcp_con_finish(bool bsuccess);                            // tcp连接成功槽函数，正式尝试登录
};

#endif // LOGINWIDGET_H
