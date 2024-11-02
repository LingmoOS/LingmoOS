#ifndef FILE_H
#define FILE_H
#define THEMEDARK "lingmo-dark"
#define THEMEBLACK "lingmo-black"
#include <QObject>
#include <QFileInfo>
#include <QVariant>
#include "model/processing/processing.h"
#include "global/computingtime.h"
#include "gio/gio.h"
#include <gio/gfileinfo.h>
#include "global/variable.h"
#include <QUrl>
typedef MatResult MatAndFileinfo;
Q_DECLARE_METATYPE(MatAndFileinfo) // QVarant 注册自定义类型
class File : public QObject
{
    Q_OBJECT

public:
    static MatAndFileinfo loadImage(QString path, QString realFormat = "", ImreadModes modes = IMREAD_UNCHANGED);
    static bool saveImage(const Mat &mat, const QString &savepath, const QString &realFormat,
                          bool replace = true); //静态图
    static bool saveImage(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat,
                          bool replace = true); //动态图
    static void deleteImage(const QString &savepath);
    static bool isSaving(const QString &path);
    static bool allSaveFinish();
    static bool canDel(QString path); //判断是否可删除
    static QString savePath();        //返回保存路径--针对裁剪和另存为
    static void changeSaveSign(bool needSave);
    static QString realFormat(const QString &path); //返回图片真正的格式

private:
    static void processStart(const QString &cmd, QStringList arguments = QStringList());
    static QProcess *m_process; //操作文件
    static QString m_imageRealFormat;
};

#endif // FILE_H
