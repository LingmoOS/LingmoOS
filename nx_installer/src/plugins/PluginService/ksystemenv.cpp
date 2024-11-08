#include "ksystemenv.h"

#include <QFile>
#include <QProcessEnvironment>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QTextCodec>

namespace KServer {

KSystemENV* KSystemENV::m_envInstance = nullptr;

KSystemENV::KSystemENV(QObject* parent ):QObject(parent)
{
  //  Q_ASSERT(m_envInstance);
    m_envInstance = this;
    readENVToFile();
}

KSystemENV::~KSystemENV()
{

}

KSystemENV* KSystemENV::init()
{
    if(!m_envInstance) {
        qDebug() << "创建系统环境变量读取对象";
        return new KSystemENV();
    }
    return m_envInstance;
}

KSystemENV* KSystemENV::getInstance()
{
    return m_envInstance;
}

void KSystemENV::readENVToFile()
{
    QFile file("/tmp/myEVN");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_envStrs = QProcess::systemEnvironment();
        for(int i = 0; i < m_envStrs.size(); i++) {
            file.write(QString(m_envStrs.at(i)).toUtf8());
            file.write("\n");
        }
    }
    file.close();
}

QString KSystemENV::getENVValue(QString name)
{
    QString envStr = "";
    QString value = "";
    QFile file("/tmp/myENV");
    if(file.open(QIODevice::ReadOnly)) {
        while (!file.atEnd()) {
            QTextCodec *codec = QTextCodec::codecForName("UTF8");
            envStr = codec->toUnicode(file.readLine());
          //  qDebug() << evnStr << "环境变量";
            if(envStr.startsWith(name)) {
                value = envStr.split('=').at(1);
//                qDebug() << value;
                break;
            }
        }
    }
    file.close();
    if(value.isEmpty()){
        qDebug() << "文件不存在或者该环境变量没有";
    }
    return value;
}

}
