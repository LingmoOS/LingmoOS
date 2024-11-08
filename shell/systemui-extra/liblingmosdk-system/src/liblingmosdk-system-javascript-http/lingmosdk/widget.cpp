/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "widget.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

QObject *object;
static QString filepath = "";
Widget* Widget::w = NULL;

#define CONFIG_FILE_NAME "/etc/lingmosdk/lingmosdk.conf"

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
    // ui->setupUi(this);
    object = this;
    w=this;
    QSettings settings(CONFIG_FILE_NAME,QSettings::IniFormat);
    settings.beginGroup("lingmosdk");
    int port = settings.value("port",8888).toInt();
    qDebug()<<"localhoat port is "<<port;
    initializeHttpServer(port);

//    convert2Json();
//    readctl = new Readctl;
//    simreadctl = new Simreadctl;
//    screenctl = new Screenctl;
//    connect(this,&Widget::openSignSignal,this,&Widget::openSign);
}
Widget::~Widget()
{
    
}
//void Widget::convert2Json()
//{
//    QJsonObject json;
//    json.insert("result","success");
//    QJsonDocument document;
//    document.setObject(json);
//    QByteArray array = document.toJson(QJsonDocument::Compact);
//    convertFromJson(array);
//    QString jsonStr = QString(array);
//    qDebug()<<jsonStr;
//}
void Widget::convertFromJson(QByteArray data)
{
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(data,&jsonError);
    if(!document.isNull() && (jsonError.error == QJsonParseError::NoError))
    {
        if(document.isObject())
        {
            QJsonObject object = document.object();
            if(object.contains("result"))
            {
                QJsonValue value = object.value("result");
                if(value.isString())
                {
                    QString re = value.toString();
                    qDebug()<<"result:"<<re;
                }
            }
        }
    }
}

bool Widget::event(QEvent *event)
{
    if(event->type() == CustomerEvent::eventType())//过滤自定义的事件
    {
        CustomerEvent *customerEvent = dynamic_cast<CustomerEvent*>(event);
        qDebug() << customerEvent->geturlValueString();
        QString filepath = customerEvent->geturlValueString();
        qDebug()<<filepath;//控制设备，来处理
//        actByFilepath(filepath);
        QString type = customerEvent->getTypeValueString();
        qDebug()<<type;
    }
    return QWidget::event(event);
}

QJsonObject Widget::deal_filepath(QString &filepath)
{
    QJsonObject jsonpath;
    QJsonArray jarray;
    QStringList strList;

    int res = -1;
    QString readres = "";
    if (filepath == "/netLink/getNetState")
    {
        qDebug() << "网络连接";
        NetLink netlink;
        int netStatus = netlink.getNetState();
        qDebug()<<netStatus;
        res = netlink.dbusConnect;
        jsonpath.insert("Result",res);
        jsonpath.insert("ResultMessage",netStatus);
        return jsonpath;
    }

}

QJsonObject Widget::deal_filepath_para(QString &filepath, QVariant filepathpara)
{
    QJsonObject jsonpath;
    QJsonArray jarray;
    QStringList strList;

    int res = -1;
    QString readres = "";
    if(filepathpara.toString() == "")
    {
        res = -1;
        readres = "输入值为空，请重新输入";
        jsonpath.insert("Result",res);
        jsonpath.insert("ResultMessage",readres);
        return jsonpath;
    }
}

void Widget::onHttpAccepted(const QPointer<JQHttpServer::Session> &session)
{
    //有客户端调用接口时，会调用该函数
    w->mutex.lock();
    QString method = session->requestMethod();//get post
    QString requestUrl = session->requestUrl();//url
    QString srcIP = session->requestSourceIp();//ip
    QString sendString = "";
    QStringList filepathList = requestUrl.split("?");
//    int num = filepathList.size();
    QJsonObject json;
    QJsonObject jsonpara;

    if(method=="GET"&&requestUrl!="/favicon.ico")
    {

        CustomerEvent *customerEvent = new CustomerEvent(requestUrl,srcIP,method,sendString);
        QStringList urlList = requestUrl.split("/");
        if(urlList[1]=="disk")
        {
            Disk disk(requestUrl);
            json = disk.deal_disk();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="netcard")
        {
            NetCard netcard(requestUrl);
            json = netcard.deal_netcard();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="bios")
        {
            Bios bios(requestUrl);
            json = bios.deal_bios();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="mainboard")
        {
            Mainboard mainboard(requestUrl);
            json = mainboard.deal_mainboard();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="peripheralsenum")
        {
            Peripheralsenum peri(requestUrl);
            json = peri.deal_peripheralsenum();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="packageinfo")
        {
            Packageinfo  packageinfo(requestUrl);
            json = packageinfo.deal_packageinfo();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="resource")
        {
            Resource resource(requestUrl);
            json = resource.deal_resource();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="process")
        {
            Process process(requestUrl);
            json = process.deal_process();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="sysinfo")
        {
            Sysinfo sysinfo(requestUrl);
            json = sysinfo.deal_sysinfo();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="net")
        {
            Net net(requestUrl);
            json = net.deal_net();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="runinfo")
        {
            Runinfo runinfo(requestUrl);
            json = runinfo.deal_runinfo();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="gps")
        {
            Gps gps(requestUrl);
            json = gps.deal_gps();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="printer")
        {
            Printer pinter(requestUrl);
            json = pinter.deal_printer();
            session->replyJsonObject(json);
        }
        else if(urlList[1]=="cpuinfo")
        {
            Cpuinfo cpuinfo(requestUrl);
            json = cpuinfo.deal_cpuinfo();
            session->replyJsonObject(json);
        }

//        if(filepathList.size()==1)
//        {
//            //不需要传参数
//            filepath = filepathList[0];
//            qDebug()<<"filepath is "<<filepath;
//            json = deal_filepath(filepath);
//            session->replyJsonObject(json);
//        }else{
//            //需要传参数
//            filepath = filepathList[0];
//            QString para = filepathList[filepathList.size()-1];
//            QStringList paraList = para.split("=");
//            qDebug()<<"参数"<<paraList;
//            jsonpara = deal_filepath_para(filepath,paraList[paraList.size()-1]);
//            session->replyJsonObject(jsonpara);
//        }

        //定义Json数组对象
//        QJsonArray array;
//        array = deal_filepath(filepath);
//        session->replyJsonArray(array);

    }
    CustomerEvent *customerEvent = new CustomerEvent(requestUrl,srcIP,method,sendString);
    QCoreApplication::postEvent(object, customerEvent);
    w->mutex.unlock();
}


void Widget::initializeHttpServer(int port)
{
    static JQHttpServer::TcpServerManage tcpServerManage(50);
    tcpServerManage.setHttpAcceptedCallback(std::bind(Widget::onHttpAccepted,std::placeholders::_1));
    const auto listenSucceed = tcpServerManage.listen(QHostAddress::LocalHost, port);
    qDebug()<<"start Listen:"<<listenSucceed;
}

