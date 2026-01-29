#include "loginwidget.h"
#include "ui_loginwidget.h"
#include "httpmgr.h"
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QJsonObject>


LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    ui->email_edit->setPlaceholderText("请输入您的邮箱");
    ui->password_edit->setPlaceholderText("请输入您的密码");
    ui->password_edit->setEchoMode(QLineEdit::Password);

    // 在构造函数中初始化验证器
    initValidator();
    initHandlerMap();       //初始化回包处理函数

    //点击返回按钮发送切换欢迎界面信号
    connect(ui->cannel_btn,&QPushButton::clicked,this,&LoginWidget::switchWelcome);

    //点击注册按钮发送切换注册页面信号
    connect(ui->register_btn,&QPushButton::clicked,this,&LoginWidget::switchRegister);

    //点击忘记密码按钮发送切换注册页面信号
    connect(ui->reset_btn,&QPushButton::clicked,this,&LoginWidget::switchReset);

    //连接http请求完成信号
    connect(HttpMgr::getInstance().get(),&HttpMgr::sig_login_mod_finish,this,&LoginWidget::slot_login_mod_finish);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

QSize LoginWidget::sizeHint() const
{
    return QSize(this->rect().size());
}

void LoginWidget::on_login_btn_clicked()    //登录按钮槽函数
{
    // 遍历所有验证器
    for(auto &validator : _validators)
    {
        // 只要有一个验证不通过，直接返回，不再继续执行登录逻辑
        // 因为 validator() 内部已经调用了 TipWidget::showTip，所以这里不需要再弹窗
        if(!validator())
        {
            return;
        }
    }

    //密码加密
    QByteArray passwordByte;
    QString pas= ui->password_edit->text();
    passwordByte.append(pas.toUtf8());
    QByteArray hash = QCryptographicHash::hash(passwordByte,QCryptographicHash::Sha512);
    QString password_sha512 = hash.toHex();

    QJsonObject json_obj;
    json_obj["email"] = ui->email_edit->text();
    json_obj["password"] = password_sha512;
    HttpMgr::getInstance()->postHttpRequest(gate_url_prefix + "/user_login",
                                        json_obj,ReqId::ID_LOGIN,Modules::MOD_LOGIN);
}

void LoginWidget::slot_login_mod_finish(ReqId reqid, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS)
    {
        TipWidget::showTip(ui->draw_label,tr("网络请求失败"));
        return;
    }

    //解析json字符串，res转换成QByteArray
    QByteArray data = res.toUtf8();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    //json解析错误
    if(jsonDoc.isNull())
    {
        TipWidget::showTip(ui->draw_label,tr("json解析错误"));
        return;
    }
    QJsonObject jsonObject = jsonDoc.object();
    //调用对应的处理函数
    _handlers_map[reqid](jsonObject);
}

void LoginWidget::bindValidator(QLineEdit *input, const QString &pattern, const QString &errMsg)
{
    // 定义校验逻辑 (Lambda)
    auto validator = [=]() -> bool
    {
        QString text = input->text();

        // 空值检查
        if (text.isEmpty())
        {
            TipWidget::showTip(input, "输入不能为空");
            return false;
        }

        // 正则匹配
        QRegularExpression rx(pattern);
        if (!rx.match(text).hasMatch())
        {
            TipWidget::showTip(input, errMsg);
            return false;
        }

        return true;
    };

    // 存入列表 (供登录按钮统查)
    _validators.append(validator);

    // 绑定信号 (失焦时自查，提升用户体验)
    connect(input, &QLineEdit::editingFinished, this, [=](){
        validator();
    });
}

void LoginWidget::initValidator()
{
    // 清空旧的验证器列表（防止重复初始化导致累积）
    _validators.clear();

    //邮箱验证
    bindValidator(ui->email_edit,
                  "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$",
                  "邮箱格式不正确");

    //密码验证
    bindValidator(ui->password_edit,
                  "^[a-zA-Z0-9!@#$%^&*?]{6,15}$",
                  "密码格式错误 (需6-15位)");

}

void LoginWidget::initHandlerMap()
{
    //注册用户回包逻辑
    _handlers_map.insert(ReqId::ID_LOGIN,[this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS)    //错误
        {
            TipWidget::showTip(ui->draw_label,"错误！请检查输入的内容");
            return;
        }
        TipWidget::showTip(ui->draw_label,"登录成功");
        auto email = jsonObj["email"].toString();
        auto uid = jsonObj["uid"].toInt();
        auto name = jsonObj["name"].toString();
        auto avator = jsonObj["avator"].toString();
        auto host = jsonObj["host"].toString();
        auto port = jsonObj["port"].toString();

        QString fromServer = jsonObj["server"].toString();
        qDebug()<<"email is "<<email<<"  from server: "<<fromServer<<"reqid is "<<"user login";
        qDebug()<<"uid is "<<uid<<" name is "<<name<<"avator is "<<avator<<"host is "<<host<<"port is "<<port;

        emit switchCanvas();

        //todo... 连接canvasServer,跳转页面

    });
}

