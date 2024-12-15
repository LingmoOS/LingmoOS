#include "application.h"
#include <QFileSystemWatcher>

Application::Application(QObject *parent)
    : QObject{parent}
{
    initSetting();
    QFileSystemWatcher *m_FileWatcher = new QFileSystemWatcher(this);
    m_FileWatcher->addPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/lingmoglobalshortcutsrc");
    connect(m_FileWatcher, &QFileSystemWatcher::fileChanged, this, &Application::initSetting);
}

void Application::initSetting()
{
    cleanSetting();
    QFile file(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/lingmoglobalshortcutsrc");
    QSettings setting(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/lingmoglobalshortcutsrc", QSettings::IniFormat);
    if(!file.exists())
    {
        setting.beginGroup("Ctrl+Alt+A");
        setting.setValue("Comment","Screenshot");
        setting.setValue("Exec","lingmo-screenshot");
        setting.endGroup();

        setting.beginGroup("Meta+L");
        setting.setValue("Comment","Screenlocker");
        setting.setValue("Exec","lingmo-screenlocker");
        setting.endGroup();

        setting.beginGroup("Ctrl+Alt+T");
        setting.setValue("Comment","Terminal");
        setting.setValue("Exec","lingmo-terminal");
        setting.endGroup();

        setting.beginGroup("Meta");
        setting.setValue("Comment","Launcher");
        setting.setValue("Exec","lingmo-launcher");
        setting.endGroup();

        setting.beginGroup("Ctrl+Alt+F");
        setting.setValue("Comment","Filemanager");
        setting.setValue("Exec","lingmo-filemanager");
        setting.endGroup();

        setting.beginGroup("Ctrl+Alt+I");
        setting.setValue("Comment","Debinstaller");
        setting.setValue("Exec","lingmo-debinstaller");
        setting.endGroup();

    }
    
    all = setting.childGroups();
    for (int i = 0; i < all.size() ; ++i)
    {
        setting.beginGroup(all.at(i));
        allexec<<setting.value("Exec").toString();
        allkey << new QHotkey(QKeySequence(all.at(i)), true, this);
        qDebug() << "Is segistered:" << allkey[i]->isRegistered();
        QObject::connect(allkey.at(i), &QHotkey::activated,[=](){
            qDebug() << i;
            qDebug() << allexec.at(i);
            QProcess::startDetached(allexec.at(i));
        });
        setting.endGroup();
    }
}
void Application::cleanSetting()
{
    all.clear();
    allexec.clear();
    foreach(QHotkey *tmp,allkey)
    {
        if(tmp)
        {
            allkey.removeOne(tmp);
            tmp->disconnect();
            delete tmp;
            tmp = nullptr;
        }
    }
}
