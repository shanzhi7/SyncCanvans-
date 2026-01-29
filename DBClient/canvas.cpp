#include "canvas.h"
#include "ui_canvas.h"
#include <QMouseEvent>
#include <QApplication>

Canvas::Canvas(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Canvas)
{
    ui->setupUi(this);
    ui->tool_dock->setWindowTitle("工具栏");
    ui->chat_dock->setWindowTitle("聊天室");
    ui->user_dock->setWindowTitle("在线用户");

    this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);    //强制所有DockWidget标签页显示在顶部

    //使用空widget替换tool_dock的标题栏
    QWidget* emptyTitleTool = new QWidget();
    ui->tool_dock->setTitleBarWidget(emptyTitleTool);

    this->tabifyDockWidget(ui->chat_dock,ui->user_dock);                    // 两个dock叠在一起

    //菜单栏
    ui->input_img->setIcon(style()->standardIcon(QStyle::SP_FileIcon));     // 设置action图标
    ui->menubar->setVisible(false);                                         //将菜单栏设置为不可见
    ui->file_btn->setMenu(ui->menu_F); // 直接把原来的菜单对象赋给按钮！

    //状态栏
    QStatusBar *bar = this->statusBar();    //获取状态栏
    // 左侧：坐标信息 (新建一个 Label)
    QLabel *posLabel = new QLabel("X: 0, Y: 0", this);
    posLabel->setStyleSheet("color: #666; font-size: 12px; padding-left: 10px;");
    bar->addWidget(posLabel); // addWidget 加在左边

    // 中间/右侧：缩放信息
    QLabel *zoomLabel = new QLabel("缩放：100%", this);
    zoomLabel->setStyleSheet("color: #333; font-weight: bold; font-size: 12px;");
    bar->addPermanentWidget(zoomLabel); // addPermanentWidget 加在最右边

    // 右侧：连接状态
    QLabel *statusDot = new QLabel("● 未连接", this);
    statusDot->setStyleSheet("color: #ff4d4d; font-size: 12px; padding-right: 10px;"); // 红色圆点
    bar->addPermanentWidget(statusDot);

    // 为整个程序安装事件过滤器
    qApp->installEventFilter(this);
}

Canvas::~Canvas()
{
    qApp->removeEventFilter(this);  // 移除事件过滤器
    delete ui;
}

bool Canvas::eventFilter(QObject *watched, QEvent *event)
{
    // 只关心鼠标按下
    if (event->type() == QEvent::MouseButtonPress)
    {

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalPos = mouseEvent->globalPos();

        // 计算 TreeWidget 的区域
        QRect treeRect(ui->treeWidget->mapToGlobal(QPoint(0, 0)), ui->treeWidget->size());

        // 判断点击位置

        if (treeRect.contains(globalPos))   //点击在treeWidget内部
        {
            QPoint localPos = ui->treeWidget->mapFromGlobal(globalPos);
            QTreeWidgetItem *item = ui->treeWidget->itemAt(localPos);

            // 点了内部空白 -> 清除选中
            if (item == nullptr)
            {
                ui->treeWidget->clearSelection();
                return true; // 拦截，因为处理了空白点击，不需要默认处理了
            }

            // 点了已选中的 Item -> 反选
            if (item->isSelected())
            {
                item->setSelected(false);
                return true; // 拦截！防止默认行为又把它选上
            }

            return false; // 放行，让 QTreeWidget 默认逻辑去选中它
        }
        else
        {
            ui->treeWidget->clearSelection();
            return false;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
