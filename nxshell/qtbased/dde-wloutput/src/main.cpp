// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: GPL-3.0-or-later

#include <string.h>
#include <stdlib.h>

#include <iostream>

#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QMap>
#include <QMapIterator>

#include <DWayland/Client/outputdevice_v2.h>
#include <DWayland/Client/registry.h>
#include <DWayland/Client/connection_thread.h>
#include <DWayland/Client/outputconfiguration_v2.h>
#include <DWayland/Client/outputmanagement_v2.h>

using namespace KWayland::Client;

enum modeFlag {
    None,
    Current = 1 << 0,
    Preferred = 1 << 1
};

class command_set {
public:
    bool enabled;
    QString uuid;
    int x, y;
    int width, height;
    int refresh, transform;
    bool used;
};

class command_argument {
    public:
        bool cmd_get;
        bool cmd_monitor;
        QMap<QString,command_set> cmd_set;
};

static command_argument *cmd_args = nullptr;
static bool beConnect = false;


static OutputManagementV2 *manager = nullptr;
static OutputConfigurationV2 *conf = nullptr;

#define parse_int_arg(dev,item,str) do { \
    dev.item = (int32_t)QString(str).toInt(); \
    } while(0);

//./dde_wloutput set e56ac6dfa7 1 0 0 1600 1200 60 f457c545a9 1 0 0 1600 1200 59.869
void parse_arguments(int argc,char **argv)
{
    cmd_args = new command_argument;
    if (QString(argv[1]).compare("get") == 0) {
        cmd_args->cmd_get = true;
        return;
    }
    if (QString(argv[1]).compare("monitor") == 0) {
        cmd_args->cmd_monitor = true;
        return;
    }

    //do set parser
    //dde_wloutput set <uuid> <enable> <x> <y> <width> <height> <refresh> <transform>
    //添加多组屏幕同时设置的需求
    if(argc%8 != 2){
        return;
    }
    int cnt = (argc-2)/8;
    qDebug()<< "cnt" << cnt;
    cmd_args->cmd_set.clear();
    for(int i=0;i<cnt;i++){
        command_set cs;
        int startIndex = i*8;
        cs.uuid = QString(argv[2 + startIndex]);

        qDebug()<< "do here:"<<cs.uuid;
        if (QString(argv[3+startIndex]).toInt() == 1) {
            cs.enabled = true;
        } else {
            cs.enabled = false;
        }

        parse_int_arg(cs, x, argv[4+startIndex]);
        parse_int_arg(cs, y, argv[5+startIndex]);
        parse_int_arg(cs, width, argv[6+startIndex]);
        parse_int_arg(cs, height, argv[7+startIndex]);
        parse_int_arg(cs, refresh, argv[8+startIndex]);
        parse_int_arg(cs, transform, argv[9+startIndex]);
        qDebug()<<"parse_int_arg" << cs.uuid<<"x:"<<cs.x<<"y:"<<cs.y<<"w:"<<cs.width<<"h:"<<cs.height;
        cs.used = false;
        cmd_args->cmd_set.insert(cs.uuid,cs);
    }
}

QString get_output_name(QString model, QString make)
{
    QString tmode(model);
    QString uuid = tmode.left(tmode.indexOf(' '));
    QStringList names = uuid.split(make);
    if (names[0] == uuid) {
        // TODO(jouyouyun): improve in future
        QStringList tmpList = names[0].split("-");
        if (tmpList.length() <= 2) {
            uuid = names[0];
        } else {
            uuid = tmpList[0] + "-" + tmpList[1];
        }
    } else {
        uuid = names[0];
    }

    return uuid;
}

void free_command_argument(struct command_argument *manager)
{
    if (!manager) {
        return;
    }
    free(manager);
}

void dump_outputs(Registry *reg) {
    QObject::connect(reg, &Registry::outputDeviceV2Announced, [reg](quint32 name, quint32 version){
        auto dev = reg->createOutputDeviceV2(name, version);
        if (!dev) {
            qDebug() << "get dev error!";
            return;
        }
        QObject::connect(dev, &OutputDeviceV2::changed, [dev]{
            std::cout << dev->model().split(" ").first().toStdString();
            std::cout<<" "<<((dev->enabled() == OutputDeviceV2::Enablement::Enabled)?"enabled":"disabled");
            std::cout<<" "<<dev->pixelSize().width()<<"x"<<dev->pixelSize().height();
            std::cout<<"+"<<dev->globalPosition().x()<<"+"<<dev->globalPosition().y()<<" "<<(dev->refreshRate()/1000.0);
            std::cout<<" "<<int(dev->transform())<<" "<<dev->scaleF()<<" "<<dev->physicalSize().width()<<"x"<<dev->physicalSize().height();
            std::cout<<" "<<dev->uuid().toStdString()<<" "<<dev->manufacturer().toStdString()<<std::endl;
            //            qDebug()<< dev->model().split(" ").first()<<((dev->enabled() == OutputDeviceV2::Enablement::Enabled)?"enabled":"disabled")
            //                   <<dev->pixelSize().width()<<"x"<<dev->pixelSize().height()
            //                  <<"+"<<dev->globalPosition().x()<<"+"<<dev->globalPosition().y()<<(dev->refreshRate()/1000.0)
            //                 <<int(dev->transform())<<dev->scaleF()<<dev->physicalSize().width()<<"x"<<dev->physicalSize().height()
            //                <<dev->uuid()<<dev->manufacturer();

            auto modes = dev->modes();
            int index = 0;
            for (auto oIter = modes.begin(); oIter != modes.end(); ++oIter)
            {
                std::cout<<"\t" << index++ <<"\t" << (*oIter)->size().width()<<"x"<<(*oIter)->size().height() << "\t" << (*oIter)->refreshRate()/1000.0;
                std::cout<<(((*oIter)->preferred() & modeFlag::Current)?"\tcurrent":"");
                std::cout<<(((*oIter)->preferred() & modeFlag::Preferred)?"\tpreferred":"")<<std::endl;
            }
            std::cout<<std::endl;
            beConnect = true;
        });
        beConnect = true;
    });
}

void set_output(Registry *reg)
{
    QObject::connect(reg, &Registry::outputManagementV2Announced, [reg](quint32 name, quint32 version) {
        qDebug() << "output management announced with name :" << name << " & version :" << version;
        qDebug() << "reg pt :" << reg << "\treg is valid :" << reg->isValid();
        manager = reg->createOutputManagementV2(name, version);
        if (!manager || !manager->isValid()) {
            qDebug() << "create manager is nullptr or not valid!";
            return;
        }
        conf = manager->createConfiguration();
        if (!conf || !conf->isValid()) {
            qDebug() << "create output configure is null or is not vaild";
            return;
        }
        QObject::connect(reg, &Registry::outputDeviceV2Announced, [reg](quint32 name, quint32 version) {
            beConnect = true;

            auto dev = reg->createOutputDeviceV2(name, version);
            if (!dev || !dev->isValid()) {
                qDebug() << "get dev is null or not valid!";
                return;
            }

            QObject::connect(dev, &OutputDeviceV2::changed, [dev] {
                beConnect = true;

                QString uuid = dev->uuid();
                //if (cmd_args->cmd_set.uuid != uuid) {
                if(!cmd_args->cmd_set.contains(uuid) || cmd_args->cmd_set[QString(uuid)].used) {
                    qDebug() << "skip output:" << uuid<<"---"<<uuid;
                    return;
                }

                qDebug() << "start set output " << uuid;
                auto modes = dev->modes();
                int index = 0;
                for (auto oIter = modes.begin(); oIter != modes.end(); ++oIter, ++index) {
                    if ((*oIter)->size().width() == cmd_args->cmd_set[uuid].width
                            && (*oIter)->size().height() == cmd_args->cmd_set[uuid].height
                            && (*oIter)->refreshRate() == cmd_args->cmd_set[uuid].refresh) {
                        qDebug() << "set output mode :" << cmd_args->cmd_set[uuid].width << "x" << cmd_args->cmd_set[uuid].height
                                 << "and refreshRate :" << cmd_args->cmd_set[uuid].refresh;
                        conf->setMode(dev, index);
                        break;
                    }
                }
                conf->setPosition(dev, QPoint(cmd_args->cmd_set[uuid].x, cmd_args->cmd_set[uuid].y));
                conf->setEnabled(dev, OutputDeviceV2::Enablement(cmd_args->cmd_set[uuid].enabled));
                qDebug() << "set output transform to " << cmd_args->cmd_set[uuid].transform;
                conf->setTransform(dev, OutputDeviceV2::Transform(cmd_args->cmd_set[uuid].transform));
                cmd_args->cmd_set[QString(uuid)].used = true;

                qDebug()<<cmd_args->cmd_set[uuid].uuid<<cmd_args->cmd_set[uuid].x;

                bool setEnd = true;
                QMapIterator<QString, command_set> i(cmd_args->cmd_set);
                while (i.hasNext()) {
                    i.next();
                    if (i.value().used == false) {
                        qDebug()<<"do this";
                        qDebug()<<"used "<<i.value().uuid;
                        setEnd = false;
                        qDebug()<<"aaa";
                        break;
                    }
                    qDebug()<<i.key();
                    qDebug()<<i.value().used<<i.value().x<<i.value().y<<i.value().uuid;
                }
                if(setEnd){
                    qDebug()<<"do set";
                    conf->apply();
                }

                if (dev)
                    dev->deleteLater();
            });
        });
        QObject::connect(conf, &OutputConfigurationV2::applied, []() {
            qDebug() << "Configuration applied!";
            conf->deleteLater();
            beConnect = true;
        });
        QObject::connect(conf, &OutputConfigurationV2::failed, []() {
            qDebug() << "Configuration failed!";
            conf->deleteLater();
            beConnect = true;
        });
        beConnect = true;
    });
}


void output_monitor(Registry *reg)
{
    QObject::connect(reg, &Registry::outputManagementV2Announced, [reg](quint32 name, quint32 version) {
        qDebug() << "[Output] management announced with name :" << name << " & version :" << version;
        qDebug() << "\treg pt :" << reg << "\treg is valid :" << reg->isValid();
        manager = reg->createOutputManagementV2(name, version);
        if (!manager || !manager->isValid()) {
            qDebug() << "create manager is nullptr or not valid!";
            return;
        }
        conf = manager->createConfiguration();
        if (!conf || !conf->isValid()) {
            qDebug() << "create output configure is null or is not vaild";
            return;
        }
        QObject::connect(reg, &Registry::outputDeviceV2Announced, [reg](quint32 name, quint32 version) {
            auto dev = reg->createOutputDeviceV2(name, version);
            if (!dev || !dev->isValid()) {
                qDebug() << "get dev is null or not valid!";
                return;
            }

            QObject::connect(dev, &OutputDeviceV2::changed, [dev] {
                qDebug()<<"[Output] [Change] "<<dev->manufacturer()<<dev->model()<<dev->uuid()<<dev->globalPosition()<<dev->pixelSize();
            });
            QObject::connect(dev, &OutputDeviceV2::removed, [dev]{
                qDebug()<<"[Output] [Change] "<<dev->manufacturer()<<dev->model()<<dev->uuid()<<dev->globalPosition()<<dev->pixelSize();
            });
        });
    });
}

void print_usage(const char *prog) {
    qDebug()<<"Usage: "<<prog<<"<get>/<monitor>/<set <uuid> <enable> <x> <y> <width> <height> <refresh> <transform>>";
}

// command:
//          get
//          monitor
//          set output eanble x y width height refresh transform
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    auto conn = new ConnectionThread;
    auto thread = new QThread;
    Registry *reg = nullptr;

    if (argc < 2 || (QString(argv[1]).compare("set") && QString(argv[1]).compare("get") && QString(argv[1]).compare("monitor")) ||
            (QString(argv[1]).compare("set") == 0 && argc % 8 != 2) ||
            (QString(argv[1]).compare("monitor") == 0 && argc != 2) ||
            (QString(argv[1]).compare("get") == 0 && argc != 2)) {
        goto usage;
    }

    parse_arguments(argc,argv);
    conn->moveToThread(thread);
    thread->start();

    QObject::connect(conn, &ConnectionThread::connected, [ & ] {

        reg = new Registry;

        bool tmp = false;
        if (cmd_args->cmd_get) {
            dump_outputs(reg);
        } else if (cmd_args->cmd_monitor) {
            tmp = true;
            output_monitor(reg);
        } else {
            set_output(reg);
        }

        QObject::connect(reg, &Registry::outputDeviceV2Removed, [](quint32 name) {
            qDebug() << "output device removed with name: " << name;
            beConnect = true;
        });

        reg->create(conn);
        reg->setup();

        do {
            beConnect = tmp;
            conn->roundtrip();
        } while (beConnect);

        if (conn) {
            conn->deleteLater();
            thread->quit();
            thread->wait();
        }
        if (reg)
            reg->deleteLater();
        if (conf)
            conf->deleteLater();


        exit(0);
    });
    QObject::connect(conn, &ConnectionThread::failed, [conn] {
        qDebug() << "connect failed to wayland at socket:" << conn->socketName();
        beConnect = true;
    });
    QObject::connect(conn, &ConnectionThread::failed, [conn] {
        qDebug() << "connect failed to wayland at socket:" << conn->socketName();
        beConnect = true;
    });
    QObject::connect(conn, &ConnectionThread::connectionDied, [ & ] {
        qDebug() << "connect failed to wayland at socket:" << conn->socketName();
        delete cmd_args;

        if (reg)
            reg->deleteLater();
        if (conf)
            conf->deleteLater();
        if (conf)
            conf->deleteLater();

        exit(-1);
    });
    conn->initConnection();
    return app.exec();

usage:
    print_usage(argv[0]);
    return -1;
}
