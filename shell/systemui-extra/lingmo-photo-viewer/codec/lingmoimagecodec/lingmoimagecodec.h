#ifndef LINGMO_IMAGE_CODEC_H
#define LINGMO_IMAGE_CODEC_H

#include "lingmoimagecodec_global.h"
#include "savemovie.h"
#include "loadmovie.h"
struct LingmoImageCodecResultBase
{
    int fps = 0;
    QFileInfo info;
    int maxFrame = 0;
    bool openSuccess = true; //判断打开是否成功，默认为true，打开成功
};
struct MatResult : public LingmoImageCodecResultBase
{
    Mat mat;
    QList<Mat> *matList = nullptr;
};

class LingmoImageCodecSignals : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void loadMovieFinish(const QString &path);
    void saveMovieFinish(const QString &path);
};


class LINGMOIMAGECODEC_EXPORT LingmoImageCodec : public LingmoImageCodecSignals
{
    Q_OBJECT

public:
    static LingmoImageCodecSignals *getSignalObj();
    static MatResult loadImageToMat(QString path);
    static MatResult loadThumbnailToMat(QString path, QString realFormat = "",
                                        ImreadModes modes = IMREAD_REDUCED_COLOR_8, QSize size = QSize(94, 56));
    static QStringList getSupportFormats();
    static QString getOpenFileFormats();
    static QPixmap converFormat(const Mat &mat);

    static bool saveImage(const Mat &mat, const QString &savepath, const QString &realFormat, bool replace = true);
    static bool saveImage(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat,
                          bool replace = true);
    static bool isSaving(const QString &path);
    static bool allSaveFinish();
    static QString savePath();
    //获取文件格式
    static FREE_IMAGE_FORMAT get_real_format(const QString &path);
    static QHash<QString, int> m_formats;
    static QStringList g_jpegList;
    static QStringList g_bmpList;
    static QStringList g_koaList;
    static QStringList g_lbmList;
    static QStringList g_tgaList;
    static QStringList g_tiffList;
    static QStringList g_pictList;
    static QStringList g_faxList;
    static QStringList g_pngList;
    static QStringList g_pbmList;
    static QStringList g_allFormat;
    static QStringList g_otherFormatList;
    static bool g_needSaveAs;
    static const QStringList m_supportFormats;          //支持的格式列表
    static QString saveWay(const QString &savepath, bool replace);
    static bool save(const Mat &mat, const QString &savepath, const QString &type);
    static bool save(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat);

private:
    static MatResult loadImageToMat(QString path, ImreadModes modes, QString suffix, QSize size);
    static MatResult loadMovieToMat(QString path, ImreadModes modes, QString suffix);
    static int getDelay(const QString &path, const QString &suffix);
    static int gifDelay(const QString &path);
    static Mat loadFreeimageFormat(const char *filename); // freeimage库加载图片
    static Mat FI2MAT(FIBITMAP *src);                     //转mat
//    static QString saveWay(const QString &savepath, bool replace);
//    static bool save(const Mat &mat, const QString &savepath, const QString &type);
//    static bool save(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat);
    static bool saveMovie(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat);
    static bool saveFreeImage(const QString &path, Mat mat, FREE_IMAGE_FORMAT); //保存freeimage格式图片
    static bool mat2fibitmap(Mat mat, FREE_IMAGE_FORMAT, QString);
    static fipImage mat2RgbFipImage(Mat mat); //转为
    static QHash<QString, int> creatFormats();

    static LingmoImageCodecSignals *m_signalObj;
    static QStringList *m_list;                         //用来保存正在保存的动图队列
    static const QString m_supportFrmatsClassify;       //打开文件格式列表
    static const QStringList m_opencvSupportFormats;    // opencv支持的格式列表
    static const QStringList m_freeimageSupportFormats; // libfreeimage支持的格式列表
    static const QStringList m_otherSupportFormats;     //其他支持格式列表
    static QString m_savePath;                          //图片保存路径

    // mime-type
    static const QStringList m_supportFormatsMT; // opencv支持的格式列表
};

#endif // LINGMO_IMAGE_CODEC_H
