#ifndef JOINROOMDIALOG_H
#define JOINROOMDIALOG_H

#include <QDialog>

namespace Ui {
class JoinRoomDialog;
}

class JoinRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JoinRoomDialog(QWidget *parent = nullptr);
    ~JoinRoomDialog();

    QString getRoomId() const;

private:
    Ui::JoinRoomDialog *ui;

private slots:

    void on_cannel_btn_clicked();
    void on_join_btn_clicked();
};

#endif // JOINROOMDIALOG_H
