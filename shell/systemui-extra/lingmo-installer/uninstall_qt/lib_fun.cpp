#include <stdio.h>

#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include "lib_fun.h"
#include <QProcess>
#include <QDir>

#define XML_APPLICATIONS_PATH "/opt/kre/config/applications_v2.xml"
#define CMD_DPKG "dpkg -S %s | awk -F':' '{print $1}'"

/*
 * compatible environment analysis pkgname and runtime by desktop field
 * p_desktop_path_in --> desktop ptah -> in , p_pkg_name_out --> analysis pkg_name -> out , analysis runtime -> out
 * return 0->success , -1->fail
 */
int snow_analysis_xml(char *p_desktop_path_in , char *p_pkg_name_out , char *p_runtime_out)
{
    if(!strlen(p_desktop_path_in) || p_pkg_name_out == NULL || p_runtime_out == NULL) {
        printf("Error : args error\n");
        return -1;
    }

    QString pkg_name;
    QStringList runtime_list;
    pkg_name.clear();
    runtime_list.clear();

    QString desktop_path(p_desktop_path_in);
    QString desktop_name = desktop_path.split('/').last();

    std::string std_desktop_name = desktop_name.toStdString();
    const char *p_desktop_name = std_desktop_name.c_str();

    char p_buf[1024];

    const char *p_front = p_desktop_name;
    const char *p_after = p_desktop_name + strlen(p_desktop_name);
    const char *p_field_front = strstr(p_desktop_name , "lingmo-v10");
    if (p_field_front == NULL) {
        printf("Error : desktop name error\n");
        return -1;
    }
    const char *p_field_after = p_field_front + 8;

    p_field_front -= 1;
    p_field_after += 1;

    char *p_tmp = p_buf;
    while(p_front != p_field_front) {
        *p_tmp = *p_front;
        p_tmp++;
        p_front++;
    }

    while(p_field_after != p_after) {
        *p_tmp = *p_field_after;
        p_tmp++;
        p_field_after++;
    }

    *p_tmp = '\0';

    printf("Info : handle desktop name is [%s]\n" , p_buf);

    desktop_name = p_buf;

    QDomDocument doc;
    QFile file(XML_APPLICATIONS_PATH);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        printf("Error : open xml file fail\n");
        return -1;
    }

    if (!doc.setContent(&file)) {
        printf("Error : load xml file fail\n");
        return -1;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomNode first = root.firstChild();

    int i_flag = 0;
    while (!first.isNull()) {
        if (first.nodeName() == "appPkgName") {
            pkg_name = first.toElement().attribute("name");
            QDomNode second = first.firstChild();
            while (!second.isNull()) {
                if (second.nodeName() == "runtimes") {
                    QDomNode third = second.firstChild();
                    while (!third.isNull()) {
                        if (third.nodeName() == "runtime") {
                            runtime_list << third.toElement().text();
                        }
                        third = third.nextSibling();
                    }
                }
                if (second.nodeName() == "appEntrys" && second.toElement().attribute("desktop") == desktop_name) {
                    i_flag = 1;
                    goto FLAG;
                }
                second = second.nextSibling();
            }
        }
        first = first.nextSibling();
    }

FLAG:
    if(i_flag == 1) {
        std::string std_pkg_name = pkg_name.toStdString();
        const char *p_pkg_name = std_pkg_name.c_str();
        strcpy(p_pkg_name_out , p_pkg_name);
        if (runtime_list.count() > 0) {
            std::string std_runtime = runtime_list.at(0).toStdString();
            const char *p_runtime = std_runtime.c_str();
            strcpy(p_runtime_out , p_runtime);
        }
        return 0;
    }

    printf("Error : not analysis desktop\n");
    return -1;
}


QString get_zhname(QString p_desktop_path_in)
{
    /*
    QString Nazh = "Name[zh_CN]";
    QFile file(p_desktop_path_in);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        while (!file.atEnd()){
            QByteArray line = file.readLine();
            QString str(line);
            if(str.contains(Nazh,Qt::CaseSensitive)){
                QStringList list = str.split("=");
                if(list[0].length() == Nazh.length())
                    return list[1].simplified();
                else
                    continue;
            }
        }
    }
    */

    QSettings setting(p_desktop_path_in, QSettings::IniFormat);
    QString locale = getenv("LANG");
    setting.beginGroup("Desktop Entry");
    if(locale.contains("zh_CN")){
        if(setting.allKeys().contains("Name[zh_CN]")){
            return QString(setting.value("Name[zh_CN]").toString().toLatin1());
        }else {
            return QString(setting.value("Name").toString().toLatin1());
        }
    }else {
        return QString(setting.value("Name").toString().toLatin1());
    }

    return "";
}

bool isOsTree()
{

    QFile osTreeFile("/etc/ostree-release");
    QDir ostreeDir("/ostree");
    QDir sysRootDir("/sysroot");
    if (osTreeFile.exists() && sysRootDir.exists() && ostreeDir.exists()){
        return true;
    }
    return false;


}
/*
 * get local pkgname by desktop
 * return 0->success , -1->fail
 */
QString snow_get_local_pkg_name(QString p_desktop_path_in)
{

    if (isOsTree()){
        if (p_desktop_path_in.length() <=0) {
            printf("Error : arg error\n");
        }
        QFileInfo filein(p_desktop_path_in);
        QString fileffsufix = filein.suffix();
        if(fileffsufix.contains("desktop",Qt::CaseSensitive))
            ;
        else
            return 0;
        if(p_desktop_path_in.contains("TopSAP"))
        {
            return "topsap";
        }
        QString cmd;
        cmd.append("kare -S ").append(p_desktop_path_in).append(" | awk -F':' '{print $1}'");
        QString pkgname  = SelectPackgeName(cmd);
        if (pkgname.isEmpty()){
            //寻找包名，只按desktop文件寻找并不完全准确。
            QFile file(p_desktop_path_in);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
                while (!file.atEnd()){
                    QByteArray line = file.readLine();
                    QString str(line);
                    if(str.contains("Exec",Qt::CaseSensitive)){
                        QStringList list = str.split("=");
                        if (list[1].contains("/usr/bin/runWithCompatibility.sh") || list[1].contains("kare run")){
                            pkgname = "kare";
                            return  pkgname;
                        }
                        if(list[1].contains(" ",Qt::CaseSensitive))
                        {
                            QStringList desk_one = list[1].split(" ");
                            cmd.clear();
                            cmd.append("kare -S ").append(desk_one[0]).append(" | awk -F':' '{print $1}'");
                            pkgname  = SelectPackgeName(cmd);
                            return pkgname;
                        }
                        else
                        {
                            cmd.clear();
                            cmd.append("kare -S ").append(list[1].simplified()).append(" | awk -F':' '{print $1}'");
                            pkgname  = SelectPackgeName(cmd);
                            return pkgname;
                        }
                    }
                }
            }
        }
        else
            return pkgname;
    }else {
        if (p_desktop_path_in.length() <=0) {
            printf("Error : arg error\n");
        }
        QFileInfo filein(p_desktop_path_in);
        QString fileffsufix = filein.suffix();
        if(fileffsufix.contains("desktop",Qt::CaseSensitive))
            ;
        else
            return 0;
        if(p_desktop_path_in.contains("TopSAP"))
        {
            return "topsap";
        }
        QString cmd;
        cmd.append("dpkg -S ").append(p_desktop_path_in).append(" | awk -F':' '{print $1}'");
        QString pkgname  = SelectPackgeName(cmd);
        if (pkgname.isEmpty()){
            //寻找包名，只按desktop文件寻找并不完全准确。
            QFile file(p_desktop_path_in);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
                while (!file.atEnd()){
                    QByteArray line = file.readLine();
                    QString str(line);
                    if(str.contains("Exec",Qt::CaseSensitive)){
                        QStringList list = str.split("=");
                        if (list[1].contains("/usr/bin/runWithCompatibility.sh") || list[1].contains("kare run")){
                            pkgname = "kare";
                            return  pkgname;
                        }
                        if(list[1].contains(" ",Qt::CaseSensitive))
                        {
                            QStringList desk_one = list[1].split(" ");
                            cmd.clear();
                            cmd.append("dpkg -S ").append(desk_one[0]).append(" | awk -F':' '{print $1}'");
                            pkgname  = SelectPackgeName(cmd);
                            return pkgname;

                        }
                        else
                        {
                            cmd.clear();
                            cmd.append("dpkg -S ").append(list[1].simplified()).append(" | awk -F':' '{print $1}'");
                            pkgname  = SelectPackgeName(cmd);
                            return pkgname;
                        }
                    }
                }
            }
        }
        else
            return pkgname;
    }
}

QString SelectPackgeName(QString cmd)
{
    QProcess terminal;
    QStringList cmdList;
    cmdList << "-c" << cmd;
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString debInfo = terminal.readAll();
    return debInfo.simplified();
}



QString getAndroidPackageName(QString desktopFilePath)
{
    if (desktopFilePath == NULL) {
        return NULL;
    }
    if(desktopFilePath.contains("TopSAP"))
    {
        return  "topsap";
    }
    QString androidFilePath(desktopFilePath);
    QString desktopFileName = androidFilePath.split('/').last();
    return QString(desktopFileName.mid(0, desktopFileName.count() - 8).toLatin1().data());

}

/*
 * remove line last ernter
 */
void remove_last_line_break(char *p_src_in_out)
{
    char *p_tmp = p_src_in_out + strlen(p_src_in_out) - 1;
    if (p_tmp != NULL) {
        if (*p_tmp == '\n')  {
            *p_tmp = '\0';
        }
    }
    return;
}

/*
 * get max six chinese characters from char array
 */
QString intercept_chinese_characters(QString src_in)
{
    QString dst;
    dst.clear();
    int i_len = src_in.length();
    qDebug() << "==========" << i_len;

    if (i_len > 6) {
        dst = src_in.mid(0 , 6);
    } else {
        dst = src_in;
    }

    qDebug() << "---------->" << dst;

    return dst;
}
