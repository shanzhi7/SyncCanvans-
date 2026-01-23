#include "resetwidget.h"
#include "ui_resetwidget.h"
#include "global.h"
#include "httpmgr.h"
#include <QRegularExpression>
#include <QJsonObject>

ResetWidget::ResetWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ResetWidget)
{
    ui->setupUi(this);

    ui->email_edit->setPlaceholderText("请输入您的邮箱 (例如: name@example.com)");
    ui->password_edit->setPlaceholderText("请输入您的密码");
    ui->confirm_edit->setPlaceholderText("请再次输入您的密码");
    ui->verify_edit->setPlaceholderText("请输入您的验证码");
    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    //连接返回登录按钮
    connect(ui->cannel_btn,&QPushButton::clicked,this,&ResetWidget::switchLogin);

    //连接http请求成功信号槽函数
    connect(HttpMgr::getInstance().get(),&HttpMgr::sig_reset_mod_finish,this,&ResetWidget::slot_reset_mod_finish);

    initValidator();        //注册验证器
    initHandersMap();       //初始化回包处理函数
    initToolButton();       //初始化两个toolbtn
}

ResetWidget::~ResetWidget()
{
    delete ui;
}

//初始化输入验证器
void ResetWidget::initValidator()
{
    //邮箱
    bindValidator(ui->email_edit,
                  "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$",
                  "邮箱格式不正确");

    //密码
    bindValidator(ui->password_edit,
                  "^[a-zA-Z0-9!@#$%^&*?]{6,15}$",
                  "密码长度需为6-15位");

    //验证码
    bindValidator(ui->verify_edit,
                  "^[a-zA-Z0-9]{4,6}$",
                  "验证码错误 (请输入4-6位字符)");

    // ==========================================
    // 【特殊】确认密码 (Confirm Password)
    // 规则：必须等于 password_edit 的内容
    // 这里的逻辑特殊，我们需要手动连接信号，不能直接用 bindValidator 传正则
    // ==========================================
    auto confirmValidator = [=]() -> bool
    {
        // 检查空
        if (ui->confirm_edit->text().isEmpty())
        {
            TipWidget::showTip(ui->confirm_edit, "请确认密码");
            return false;
        }

        // 检查一致性
        if (ui->confirm_edit->text() != ui->password_edit->text()) {
            TipWidget::showTip(ui->confirm_edit, "两次密码输入不一致");
            return false;
        }

        return true;
    };
    _validators.append(confirmValidator);
    connect(ui->confirm_edit, &QLineEdit::editingFinished, confirmValidator);
}

void ResetWidget::initHandersMap()
{
    //注册用户回包逻辑
    _handlers_map.insert(ReqId::ID_RESET_PWD,[this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS)    //错误
        {
            TipWidget::showTip(ui->title_lbl,"错误！请检查输入的内容");
            return;
        }
        TipWidget::showTip(ui->title_lbl,"密码重置成功");
        auto email = jsonObj["email"].toString();
        QString fromServer = jsonObj["server"].toString();
        qDebug()<<"email is "<<email<<"  from server: "<<fromServer<<"reqid is "<<"password reset";

        //页面跳转到登录页面
        emit switchLogin();
    });

    //注册获取验证码回包逻辑
    _handlers_map.insert(ReqId::ID_GET_VERIFY_CODE,[this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS)    //错误
        {
            TipWidget::showTip(ui->title_lbl,"错误！请检查输入的内容");
            return;
        }
        TipWidget::showTip(ui->title_lbl,"验证码已发送到邮箱,注意查收");
        auto email = jsonObj["email"].toString();
        QString fromServer = jsonObj["server"].toString();
        qDebug()<<"email is "<<email<<"  from server: "<<fromServer<<"reqid is "<<"get verifycode";

    });
}

void ResetWidget::initToolButton()
{
    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    ui->password_tool->setIcon(QIcon(":/res/unvisible.png"));
    ui->confirm_tool->setIcon(QIcon(":/res/unvisible.png"));

    connect(ui->password_tool,&QToolButton::clicked,this,[=](){
        if (ui->password_edit->echoMode() == QLineEdit::Password)
        {
            // 切换为明文
            ui->password_edit->setEchoMode(QLineEdit::Normal);
            ui->password_tool->setIcon(QIcon(":/res/visible.png")); // 换成睁眼
        }
        else
        {
            // 切换为密文
            ui->password_edit->setEchoMode(QLineEdit::Password);
            ui->password_tool->setIcon(QIcon(":/res/unvisible.png")); // 换成闭眼
        }
    });

    connect(ui->confirm_tool,&QToolButton::clicked,this,[=](){

        if (ui->confirm_edit->echoMode() == QLineEdit::Password)
        {
            // 切换为明文
            ui->confirm_edit->setEchoMode(QLineEdit::Normal);
            ui->confirm_tool->setIcon(QIcon(":/res/visible.png")); // 换成睁眼
        }
        else
        {
            // 切换为密文
            ui->confirm_edit->setEchoMode(QLineEdit::Password);
            ui->confirm_tool->setIcon(QIcon(":/res/unvisible.png")); // 换成闭眼
        }
    });
}

void ResetWidget::bindValidator(QLineEdit *input, const QString &pattern, const QString &errMsg)
{
    // 1. 定义校验逻辑 (Lambda)
    auto validator = [=]() -> bool {
        QString text = input->text();

        // 空值检查 (如果必填)
        if (text.isEmpty()) {
            TipWidget::showTip(input, "输入不能为空");
            return false;
        }

        // 正则匹配
        QRegularExpression rx(pattern);
        if (!rx.match(text).hasMatch()) {
            // 只弹窗，不改样式
            TipWidget::showTip(input, errMsg);
            return false;
        }

        return true;
    };

    // 2. 存入列表 (供注册按钮统查)
    _validators.append(validator);

    // 3. 绑定信号 (失焦时自查)
    connect(input, &QLineEdit::editingFinished, this, [=](){
        validator();
    });
}


void ResetWidget::on_get_verify_btn_clicked()
{
    //邮箱正则表达式
    static const QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    QString email = ui->email_edit->text();         //获取用户输入的邮箱
    bool match = regex.match(email).hasMatch();     //判断是否匹配

    if(match)//格式正确
    {
        TipWidget::showTip(ui->title_lbl, "尝试获取验证码...");
        //发送http请求
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::getInstance()->postHttpRequest(QUrl(gate_url_prefix + "/get_verifycode"),json_obj,
                                                ReqId::ID_GET_VERIFY_CODE,Modules::MOD_RESET);
    }
    else
    {
        //邮箱格式不正确
        TipWidget::showTip( ui->email_edit,tr("邮箱格式不正确"));
    }
}


void ResetWidget::on_reset_btn_clicked()
{
    // 遍历检查所有规则
    for (const auto &validator : _validators)
    {
        if (!validator())
        {
            // 如果有一个失败，validator() 内部已经弹窗了
            // 直接中断，不发送请求
            return;
        }
    }

    //密码加密
    QByteArray passwordByte;
    QString pas= ui->password_edit->text();
    passwordByte.append(pas.toUtf8());
    QByteArray hash = QCryptographicHash::hash(passwordByte,QCryptographicHash::Sha512);
    QString password_sha512 = hash.toHex();

    //确认密码
    QByteArray confirmByte;
    QString con= ui->confirm_edit->text();
    confirmByte.append(con.toUtf8());
    QByteArray hash_con = QCryptographicHash::hash(confirmByte,QCryptographicHash::Sha512);
    QString confirm_sha512 = hash_con.toHex();

    //发送http请求，注册账号
    QJsonObject json_obj;
    json_obj["email"] = ui->email_edit->text();
    json_obj["password"] = password_sha512;
    json_obj["confirm"] = confirm_sha512;
    json_obj["verifycode"] = ui->verify_edit->text();
    HttpMgr::getInstance()->postHttpRequest(QUrl(gate_url_prefix + "/reset_password"),json_obj,
                                            ReqId::ID_RESET_PWD,Modules::MOD_RESET);
}

void ResetWidget::slot_reset_mod_finish(ReqId reqid, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS)
    {
        TipWidget::showTip(ui->title_lbl,tr("网络请求失败"));
        return;
    }

    //解析json字符串，res转换成QByteArray
    QByteArray data = res.toUtf8();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    //json解析错误
    if(jsonDoc.isNull())
    {
        TipWidget::showTip(ui->title_lbl,tr("json解析错误"));
        return;
    }
    QJsonObject jsonObject = jsonDoc.object();
    //调用对应的处理函数
    _handlers_map[reqid](jsonObject);
}

