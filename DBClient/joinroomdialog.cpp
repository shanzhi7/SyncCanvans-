#include "joinroomdialog.h"
#include "ui_joinroomdialog.h"
#include "tipwidget.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>

JoinRoomDialog::JoinRoomDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JoinRoomDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    this->setWindowTitle("加入协作");

    // 2. 限制只能输入 1-8 位数字
    QRegularExpression rx("^[0-9]{1,8}$");
    QValidator *validator = new QRegularExpressionValidator(rx, this);  //设置验证器限制输入
    ui->roomId_edit->setValidator(validator);
}

JoinRoomDialog::~JoinRoomDialog()
{
    delete ui;
}

QString JoinRoomDialog::getRoomId() const
{
    return ui->roomId_edit->text().trimmed();
}

void JoinRoomDialog::on_cannel_btn_clicked()
{
    this->reject();
}


void JoinRoomDialog::on_join_btn_clicked()
{
    QString room_id = getRoomId();
    if(room_id.isEmpty())
    {
        TipWidget::showTip(ui->roomId_edit,"请输入房间号！");
        return;
    }
    this->accept();
}

