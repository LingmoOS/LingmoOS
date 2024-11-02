#include "file.h"

QProcess *File::m_process = nullptr;
QString File::m_imageRealFormat = "";
MatAndFileinfo File::loadImage(QString path, QString realFormat, ImreadModes modes)
{
    MatAndFileinfo maf = LingmoImageCodec::loadThumbnailToMat(path, realFormat, modes);
    //判断图像是否有效，无效设置为默认失败图片提示
    if (!maf.openSuccess) {
        //打开失败，将图片路径存起来，便于切换主题时，更改相册对应的图片icon
        if (!Variable::g_damagedImage.contains(path)) {
            Variable::g_damagedImage.append(path);
        }
        if (THEMEDARK == Variable::g_themeStyle || THEMEBLACK == Variable::g_themeStyle) {
            path = Variable::DAMAGED_IMAGE_BLACK;
        } else {
            path = Variable::DAMAGED_IMAGE_WHITE;
        }
        QImage img(path, "apng");
        maf.mat = Mat(img.height(), img.width(), CV_8UC4, const_cast<uchar *>(img.bits()),
                      static_cast<size_t>(img.bytesPerLine()))
                      .clone();
    }
    return maf;
}

bool File::saveImage(const Mat &mat, const QString &savepath, const QString &realFormat, bool replace)
{
    return LingmoImageCodec::saveImage(mat, savepath, realFormat, replace);
}

bool File::saveImage(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat, bool replace)
{
    return LingmoImageCodec::saveImage(list, fps, savepath, realFormat, replace);
}


void File::deleteImage(const QString &savepath)
{
    if (!QFile::exists(savepath)) {
        return;
    }
    processStart("gio", QStringList() << "trash" << savepath);
}

bool File::isSaving(const QString &path)
{
    return LingmoImageCodec::isSaving(path);
}

bool File::allSaveFinish()
{
    return LingmoImageCodec::allSaveFinish();
}

bool File::canDel(QString path)
{
    // gio获取fileinfo -- char
    //对于含有特殊字符的路径，url特殊字符需转义：%#等。g_file_new_for_uri是never fail，所以不用判空
    GFile *file = g_file_new_for_uri(QUrl::fromLocalFile(path).toString().toUtf8().constData());
    //参数：1.--file；2.--文件属性 ""代表获取所有属性，access::* --
    //代表可获得的所有boolean属性；3.先设置默认属性；4.和5.nullptr即可
    GFileInfo *fileInfo =
        g_file_query_info(file, "access::*," G_FILE_ATTRIBUTE_ID_FILE, G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    //判空-返回false
    if (!fileInfo) {
        return false;
    }
    bool canwrite = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
    bool candel = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH);

    if (canwrite && candel) {
        return true;
    } else {
        return false;
    }
}

QString File::savePath()
{
    return LingmoImageCodec::savePath();
}

void File::changeSaveSign(bool needSave)
{
    LingmoImageCodec::g_needSaveAs = needSave;
}

QString File::realFormat(const QString &path)
{
    m_imageRealFormat = "";
    m_imageRealFormat = LingmoImageCodec::m_formats.key(LingmoImageCodec::get_real_format(path));
    if (m_imageRealFormat == "") {
        processStart("file", QStringList() << "-i" << path);
        if (m_imageRealFormat != "") {
            if (m_imageRealFormat.contains("image/")) {
                m_imageRealFormat = m_imageRealFormat.mid(m_imageRealFormat.indexOf(":") + 2);
                m_imageRealFormat = m_imageRealFormat.left(m_imageRealFormat.indexOf(";"));
            } else {
                m_imageRealFormat = "";
            }
        }
    }
    if (m_imageRealFormat.startsWith("image/")) {
        if ("image/svg+xml" == m_imageRealFormat) {
            m_imageRealFormat = "svg";
        } else if ("image/x-sun-raster" == m_imageRealFormat) {
            m_imageRealFormat = "sr";
        } else if ("image/x-portable-anymap" == m_imageRealFormat) {
            m_imageRealFormat = "pnm";
        } else {
            qDebug() << "不支持的format";
        }
    } else {
        m_imageRealFormat = m_imageRealFormat.toLower();
    }
    //校准
    if ("" != m_imageRealFormat) {
        QFileInfo file(path);
        QString fileFormat = file.suffix().toLower();
        // jpeg
        if (LingmoImageCodec::g_jpegList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_jpegList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "jpeg";
            }
            goto ends;
        }
        //{"bmp", "dib"};
        if (LingmoImageCodec::g_bmpList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_bmpList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "bmp";
            }
            goto ends;
        }
        //{"png", "apng"}
        if (LingmoImageCodec::g_pngList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_pngList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "png";
            }
            goto ends;
        }
        //{"fax", "g3"}
        if (LingmoImageCodec::g_faxList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_faxList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "fax";
            }
            goto ends;
        }
        //{"koa", "koala"}
        if (LingmoImageCodec::g_koaList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_koaList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "koa";
            }
            goto ends;
        }
        //{"lbm", "iff"}
        if (LingmoImageCodec::g_lbmList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_lbmList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "lbm";
            }
            goto ends;
        }
        //{"tga", "targa"}
        if (LingmoImageCodec::g_tgaList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_tgaList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "tga";
            }
            goto ends;
        }
        //{"tiff", "tif"}
        if (LingmoImageCodec::g_tiffList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_tiffList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "tiff";
            }
            goto ends;
        }
        //{"pct", "pic", "pict"}
        if (LingmoImageCodec::g_pictList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_pictList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "pict";
            }
            goto ends;
        }
        //{"ppm", "pbm", "pgm"}
        if (LingmoImageCodec::g_pbmList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_pbmList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "pbm";
            }
            goto ends;
        }
        if (LingmoImageCodec::g_otherFormatList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_otherFormatList.contains(fileFormat) && fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            }
        }
    }

ends:
    return m_imageRealFormat;
}

void File::processStart(const QString &cmd, QStringList arguments)
{
    if (m_process == nullptr) {
        // File构造函数是在主线程调用的，m_process如果在构造函数中实例化，会报错
        m_process = new QProcess; //操作文件
        connect(m_process, &QProcess::readyReadStandardError, [=] {
            qDebug() << "*******process error*******\n"
                     << QString::fromLocal8Bit(m_process->readAllStandardError()) << "\n*******process error*******";
        });
        connect(m_process, &QProcess::readyReadStandardOutput, [=] {
            m_imageRealFormat = QString::fromLocal8Bit(m_process->readAll());
        });
        m_process->setReadChannel(QProcess::StandardOutput); //标准输出读取内容必须设置该属性
    }
    m_process->start(cmd, arguments);
    m_process->waitForStarted();
    m_process->waitForFinished();
    m_process->waitForReadyRead();
}
