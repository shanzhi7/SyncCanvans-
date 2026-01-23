#include "loginwidget.h"
#include "ui_loginwidget.h"

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
    ui->email_edit->setPlaceholderText("请输入您的邮箱");
    ui->password_edit->setPlaceholderText("请输入您的密码");
    ui->password_edit->setEchoMode(QLineEdit::Password);

    //点击返回按钮发送切换欢迎界面信号
    connect(ui->cannel_btn,&QPushButton::clicked,this,&LoginWidget::switchWelcome);

    //点击注册按钮发送切换注册页面信号
    connect(ui->register_btn,&QPushButton::clicked,this,&LoginWidget::switchRegister);

    //点击忘记密码按钮发送切换注册页面信号
    connect(ui->reset_btn,&QPushButton::clicked,this,&LoginWidget::switchReset);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

QSize LoginWidget::sizeHint() const
{
    return QSize(this->rect().size());
}
