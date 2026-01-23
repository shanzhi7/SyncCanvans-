#ifndef RESETWIDGET_H
#define RESETWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include "tipwidget.h"
#include "global.h"

namespace Ui {
class ResetWidget;
}

class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResetWidget(QWidget *parent = nullptr);
    ~ResetWidget();

private:
    Ui::ResetWidget *ui;

    void bindValidator(QLineEdit *input, const QString &pattern, const QString &errMsg);
    void initValidator();       //初始化验证器，验证输入合法性
    void initHandersMap();      //初始化服务器回包函数
    void initToolButton();      //初始化ui的两个toolBtn

    // 定义一个列表，存储所有返回 bool 的校验函数
    QList<std::function<bool()>> _validators;
    QMap<ReqId,std::function<void(QJsonObject)>> _handlers_map;        //服务器回包处理函数存储

signals:
    void switchLogin();     //返回登录界面信号
private slots:
    void on_get_verify_btn_clicked();       //获取验证码按钮槽函数
    void on_reset_btn_clicked();            //重置密码按钮槽函数

    void slot_reset_mod_finish(ReqId reqid, QString res, ErrorCodes err);           //重置密码回调函数
};

#endif // RESETWIDGET_H
