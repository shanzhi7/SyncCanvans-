#include "tcpmgr.h"
#include <QNetworkProxy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

TcpMgr::TcpMgr():_host(""),_port(0),_b_recv_pending(false),_message_id(0),_message_len(0)
{
    //连接成功
    QObject::connect(&_socket,&QTcpSocket::connected,this,[this](){
        qDebug()<<"连接到 Server！";
        emit sig_con_success(true);
    });

    //准备读取数据
    QObject::connect(&_socket,&QTcpSocket::readyRead,this,[this](){
        //当有数据可读时，读取所有数据
        //读取数据追加到缓冲区buffer
        _buffer.append(_socket.readAll());


        // 循环处理_buffer 直到缓冲区无数据，避免粘包问题
        while(_buffer.size() > 0)
        {
            QDataStream stream(&_buffer,QIODevice::ReadOnly);
            stream.setByteOrder(QDataStream::BigEndian);        //设置大端字节序(网络字节序)
            stream.setVersion(QDataStream::Qt_6_5);             //设置版本
            //解析消息头(当前没有等待的消息)
            if(!_b_recv_pending)
            {
                //消息头不够(4字节),等待更多数据
                if(_buffer.size() < sizeof(quint16) * 2)
                {
                    break;
                }
                //读取消息id和长度
                stream >> _message_id >> _message_len;
                _buffer = _buffer.mid(sizeof(quint16) * 2);
                qDebug() << "Message ID:" << _message_id << ", Length:" << _message_len;

            }

            //解析消息体
            if(_buffer.size() < _message_len)   //长度不足
            {
                _b_recv_pending = true;         //当前有等待的消息
                break;
            }

            _b_recv_pending = false;            //当前无等待消息

            //消息体足够，解析处理
            QByteArray messageBody = _buffer.mid(0,_message_len);
            qDebug() << "Received body:" << messageBody;

            handleMsg(ReqId(_message_id),_message_len,messageBody);

            //移除已经解析的消息体
            _buffer = _buffer.mid(_message_len);    //如果执行完mid 长度不为 0，说明读取到了下一条消息
        }
    });

    //错误处理
    QObject::connect(&_socket,QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
                     [&](QAbstractSocket::SocketError socketError){
                         Q_UNUSED(socketError);

                         qDebug() << "Error:" << _socket.errorString() ;
                         switch (socketError)
                         {
                         case QTcpSocket::ConnectionRefusedError:     //服务端拒绝
                             qDebug() << "Connection Refused!";
                             emit sig_con_success(false);
                             break;
                         case QTcpSocket::RemoteHostClosedError:      //服务端关闭
                             qDebug() << "Remote Host Closed Connection!";
                             break;
                         case QTcpSocket::HostNotFoundError:          //未找到目标服务(host,port)
                             qDebug() << "Host Not Found!";
                             emit sig_con_success(false);
                             break;
                         case QTcpSocket::SocketTimeoutError:         //连接超时
                             qDebug() << "Connection Timeout!";
                             emit sig_con_success(false);
                             break;
                         case QTcpSocket::NetworkError:               //网络错误
                             qDebug() << "Network Error!";
                             break;
                         default:
                             qDebug() << "Other Error!";
                             break;
                         }
                     });

    initHandlers();

    //连接tcp发送信号与槽函数
    connect(this,&TcpMgr::sig_send_data,this,&TcpMgr::slot_send_data);
}

void TcpMgr::initHandlers()
{
    //注册登录回包函数
    _handlers.insert(ReqId::ID_CANVAS_LOGIN_RSP,[this](ReqId id,int len,QByteArray data){
        qDebug()<<"handle id is "<<id<<" data is "<<data;

        //将ByteArray转换成QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        //检测是否转换成功
        if(jsonDoc.isNull())
        {
            qDebug()<<"creat QJsonDocument 错误";
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        if(!jsonObj.contains("error"))      //没有error这个键
        {
            int err = ErrorCodes::ERR_JSON;
            qDebug()<<"登录失败,error json解析失败 "<<err;
            emit sig_login_failed(err);
        }
        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS)
        {
            qDebug()<<"登录失败,error is "<<err;
            emit sig_login_failed(err);
            return;
        }

        qDebug()<<"tcpmgr: recv error is "<<jsonObj["error"].toInt()<<" uid is "<<jsonObj["uid"].toInt();
        emit sig_switch_canvas();   //发送信号切换canvas界面
    });
}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto it = _handlers.find(id);
    if(it == _handlers.end())
    {
        qDebug()<<"not found id ["<<id<<"] to handle";
        return;
    }
    it.value()(id,len,data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug()<<"接收到 tcp connect singal";

    //尝试连接服务器
    qDebug()<<"Connnecting to Server......";
    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toUInt());

    _socket.setProxy(QNetworkProxy::NoProxy);           //禁用代理
    _socket.connectToHost(_host,_port);                 //连接服务器
}

void TcpMgr::slot_send_data(ReqId reqid, QByteArray data)
{
    quint16 id = reqid;
    quint16 len = static_cast<quint16>(data.size());

    //创建一个QByteArray用于存储准备发送的数据
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);       //设置网络字节序(大端字节序)

    //写入包头
    out<<id<<len;

    //写入body
    block.append(data);
    _socket.write(block);                           //发送数据
}
