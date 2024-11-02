#ifndef __EXTERNAL_H__
#define __EXTERNAL_H__

#include <QObject>
#include <QGSettings>
#include <QString>
#include <QList>
#include <QProcess>

class External : public QObject
{
    Q_OBJECT

public:
    External();
    ~External();
    QList<QString> getCollectFont(void);
    bool addCollectFont(QString path);
    bool removeCollectFont(QString path);

    bool applyFont(QString path);
    QStringList getFontFromControl();
    bool setFontIntoList(QString font);
    QString isUsingFont();
    bool removeFontFormContro(QString font);
    bool getFontTipsData();           /* 获取gsetting的显示提示窗口的状态 */
    bool setFontIntoList(bool state); /* 设置gsetting的显示提示窗口的状态 */
private:
    QGSettings *m_gsettings = nullptr;
    QGSettings *m_themeGsettings = nullptr;
    QGSettings *m_contolGsettings = nullptr;
    QGSettings *m_gtkGsettings = nullptr;
    // QProcess *m_process = nullptr;
};

#endif
