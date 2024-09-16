// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <mutex>

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QFontInfo>
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QFontDatabase>
#include <QTextCodec>
#include <QProcess>
#include <QStorageInfo>
#include <QDBusInterface>

#include <DSysInfo>

#include <QApt/Package>

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

QString Utils::loadFontFamilyByType(FontType fontType)
{
    Q_UNUSED(fontType);
    QFont font;
    return font.family();
}

QFont Utils::loadFontBySizeAndWeight(const QString &fontFamily, int fontSize, int fontWeight)
{
    Q_UNUSED(fontSize)

    QFont font(fontFamily);
    font.setWeight(fontWeight);

    return font;
}

void Utils::bindFontBySizeAndWeight(QWidget *widget, const QString &fontFamily, int fontSize, int fontWeight)
{
    if (nullptr == widget)
        return;
    QFont font = loadFontBySizeAndWeight(fontFamily, fontSize, fontWeight);
    widget->setFont(font);

    DFontSizeManager::SizeType sizeType = DFontSizeManager::T6;
    switch (fontSize) {
        case 10: {
            sizeType = DFontSizeManager::T10;
        } break;
        case 11: {
            sizeType = DFontSizeManager::T9;
        } break;
        case 12: {
            sizeType = DFontSizeManager::T8;
        } break;
        case 13: {
            sizeType = DFontSizeManager::T7;
        } break;
        case 14: {
            sizeType = DFontSizeManager::T6;
        } break;
        case 17: {
            sizeType = DFontSizeManager::T5;
        } break;
        case 20: {
            sizeType = DFontSizeManager::T4;
        } break;
        case 24: {
            sizeType = DFontSizeManager::T3;
        } break;
        case 30: {
            sizeType = DFontSizeManager::T2;
        } break;
        case 40: {
            sizeType = DFontSizeManager::T1;
        } break;
    }

    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(widget, sizeType, fontWeight);
}

QString Utils::fromSpecialEncoding(const QString &inputStr)
{
    bool bFlag = inputStr.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
    if (bFlag) {
        return inputStr;
    }

    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    if (codec) {
        QString unicodeStr = codec->toUnicode(inputStr.toLatin1());
        return unicodeStr;
    } else {
        return inputStr;
    }
}
bool Utils::Return_Digital_Verify(const QString &strfilepath, const QString &strfilename)
{
    QDir dir(strfilepath);
    if (!dir.exists()) {
        qDebug() << "文件夹不存在";
        return false;
    }
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
    int file_count = list.count();
    qDebug() << "file_count                 " << file_count;
    if (file_count <= 0) {
        qDebug() << "当前文件夹为空";
        return false;
    }
    for (int i = 0; i < list.count(); i++) {
        QFileInfo file_info = list.at(i);
        if (file_info.fileName() == strfilename) {
            qDebug() << "文件路径：  " << file_info.path() << "           "
                     << "文件名：  " << file_info.fileName();
            return true;
        }
    }
    return false;
}

Utils::VerifyResultCode Utils::Digital_Verify(const QString &filepath_name)
{
    QString verifyfilepath = "/usr/bin/";
    QString verifyfilename = "deepin-deb-verify";
    bool result_verify_file = Return_Digital_Verify(verifyfilepath, verifyfilename);
    qDebug() << "result_verify_file" << result_verify_file;
    if (result_verify_file) {
        QProcess proc;
        QString program = "/usr/bin/deepin-deb-verify";
        proc.start(program, { filepath_name });
        proc.waitForFinished(-1);
        const QString output1 = proc.readAllStandardError();
        qInfo() << "签名校验结果：" << output1;
        for (const auto &item : output1.split('\n')) {
            if (item.toLatin1() == "[INFO] signature verified!") {
                return VerifySuccess;
            }
            if (item.toLatin1() == "cannot find signinfo in deb file") {
                return DebfileInexistence;
            }
            if (item.toLatin1() == "extract deb_file failed!") {
                return ExtractDebFail;
            }
            if (item.toLatin1() == "verify deb file failed!") {
                return DebVerifyFail;
            }
        }
    }
    return OtherError;
}

QString Utils::holdTextInRect(const QFont &font, const QString &srcText, const QSize &size)
{
    bool bContainsChinese = srcText.contains(QRegExp("[\\x4e00-\\x9fa5]+"));

    QString text;
    QString tempText;
    int totalHeight = size.height();
    int lineWidth = size.width() - font.pixelSize();

    int offset = bContainsChinese ? font.pixelSize() : 0;

    QFontMetrics fm(font);

    int calcHeight = 0;
    int lineHeight = fm.height();
    int lineSpace = 0;
    int lineCount = (totalHeight - lineSpace) / lineHeight;
    int prevLineCharIndex = 0;
    for (int charIndex = 0; charIndex < srcText.size() && lineCount >= 0; ++charIndex) {
        int fmWidth = fm.horizontalAdvance(tempText);
        if (fmWidth > lineWidth - offset || tempText.contains("\n")) {
            calcHeight += lineHeight + 3;
            if (calcHeight + lineHeight > totalHeight) {
                QString endString = srcText.mid(prevLineCharIndex);
                const QString &endText = fm.elidedText(endString, Qt::ElideRight, size.width());
                text += endText;

                lineCount = 0;
                break;
            }

            if (!bContainsChinese) {
                QChar currChar = tempText.at(tempText.length() - 1);
                QChar nextChar = srcText.at(srcText.indexOf(tempText) + tempText.length());
                if (currChar.isLetter() && nextChar.isLetter()) {
                    tempText += '-';
                }
                fmWidth = fm.horizontalAdvance(tempText);
                if (fmWidth > size.width()) {
                    --charIndex;
                    --prevLineCharIndex;
                    tempText = tempText.remove(tempText.length() - 2, 1);
                }
            }
            text += tempText;

            --lineCount;
            if (lineCount > 0) {
                if (!tempText.contains("\n"))
                    text += "\n";
            }
            tempText = srcText.at(charIndex);

            prevLineCharIndex = charIndex;
        } else {
            tempText += srcText.at(charIndex);
        }
    }

    if (lineCount > 0) {
        text += tempText;
    }
    return text;
}

QString Utils::holdTextInRect(const QFont &font, const QString &srcText, const int &width)
{
    bool bContainsChinese = srcText.contains(QRegExp("[\\x4e00-\\x9fa5]+"));

    QString text;
    QString tempText;
    int lineWidth = width - 12;

    int offset = bContainsChinese ? font.pixelSize() : 0;

    QFontMetrics fm(font);

    int prevLineCharIndex = 0;
    for (int charIndex = 0; charIndex < srcText.size(); ++charIndex) {
        int fmWidth = fm.horizontalAdvance(tempText);
        if (fmWidth > lineWidth - offset || tempText.contains("\n")) {
            if (!bContainsChinese) {
                QChar currChar = tempText.at(tempText.length() - 1);
                QChar nextChar = srcText.at(srcText.indexOf(tempText) + tempText.length());
                if (currChar.isLetter() && nextChar.isLetter()) {
                    tempText += '-';
                }
                fmWidth = fm.horizontalAdvance(tempText);
                if (fmWidth > lineWidth) {
                    --charIndex;
                    --prevLineCharIndex;
                    tempText = tempText.remove(tempText.length() - 2, 1);
                    if (currChar != '\n' && nextChar != '\n') {
                        text += tempText;
                        text += "\n";
                    }
                }
            } else {
                text += tempText;
                text += "\n";
            }

            tempText = srcText.at(charIndex);
            prevLineCharIndex = charIndex;
        } else {
            tempText += srcText.at(charIndex);
        }
    }
    if (text.size() >= 1 && text[text.size() - 1] != '\n')
        text += "\n";
    text += tempText;

    return text;
}

/*!
  @brief 检测软件包 `packagePath` 是否可读，首先判断路径指向的挂载设备信息，
    若为 gvfs 或 cifs 文件系统(文管当前及V6后的远程挂载系统)，抛出false。
    同时判断文件是否有权限安装，无权限则抛出异常。
  @param[in] packagePath 软件包路径
  @note 只能安装本地的软件包，samba/ftp等远程目录不允许。

  @todo 增加对可移除设备的判断!
*/
bool Utils::checkPackageReadable(const QString &packagePath)
{
    // 获取路径信息
    QStorageInfo info(packagePath);
    // 判断路径信息是不是本地路径
    QString device = info.device();
    // 黑名单识别 gvfs/cifs 为当前管理远程目录的文件系统
    if (device.startsWith("gvfs") || device.startsWith("cifs")) {
        qWarning() << "Disable open remote file, the devices is" << device;
        return false;
    }

    QFileInfo debFileIfo(packagePath);
    // 查看包是否能够打开，无法打开说明包不在本地或无权限。
    QFile outfile(packagePath.toUtf8());
    outfile.open(QFile::ReadOnly);

    if (!outfile.isOpen()) {  // 打不开，文件不在本地或无安装权限
        QFile::FileError error = outfile.error();
        if (error == QFile::FileError::NoError) {
            // 文件不存在或路径错误
            qWarning() << "Package has permission but cannot open!";
            return false;
        } else {
            // 无安装权限
            qWarning() << "Package has no read permission!";
            return false;
        }
    } else {  // 文件能打开，说明文件在本地且有权限
        outfile.close();
        return true;
    }
}

int Utils::compareVersion(const QString &v1, const QString &v2)
{
    return QApt::Package::compareVersion(v1, v2);
}

/**
   @return Check if the current mode is development mode, the status will be called once and stored.
 */
bool Utils::isDevelopMode()
{
    static bool kIsDevelopMode = false;
    static std::once_flag kDevelopOnceFlag;
    std::call_once(kDevelopOnceFlag, [&]() {
    // Add for judge OS Version
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 2, 2, 2))
        qInfo() << "system code(UOS): " << Dtk::Core::DSysInfo::uosEditionType();
        switch (Dtk::Core::DSysInfo::uosEditionType()) {
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 4, 10, 0))
            case Dtk::Core::DSysInfo::UosEducation:
            case Dtk::Core::DSysInfo::UosDeviceEdition:
#endif
            case Dtk::Core::DSysInfo::UosProfessional:
            case Dtk::Core::DSysInfo::UosHome: {
                // Check if current is develop mode
                QDBusInterface *dbusInterFace = new QDBusInterface(
                    "com.deepin.sync.Helper", "/com/deepin/sync/Helper", "com.deepin.sync.Helper", QDBusConnection::systemBus());
                bool deviceMode = dbusInterFace->property("DeveloperMode").toBool();
                qInfo() << "DebListModel:"
                        << "system editon:" << Dtk::Core::DSysInfo::uosEditionName() << "develop mode:" << deviceMode;
                kIsDevelopMode = deviceMode;
                delete dbusInterFace;
                break;
            }
            case Dtk::Core::DSysInfo::UosCommunity:   // The community edition does not need signature verification
            case Dtk::Core::DSysInfo::UosEnterprise:  // Server Version
                kIsDevelopMode = true;
                break;
            default:
                kIsDevelopMode = true;
                break;
        }
#else
        qInfo() << "system code(Deepin): " << Dtk::Core::DSysInfo::deepinType();
        switch (Dtk::Core::DSysInfo::deepinType()) {
        case Dtk::Core::DSysInfo::DeepinDesktop:
            kIsDevelopMode = true;
            break;
        case Dtk::Core::DSysInfo::DeepinPersonal:
        case Dtk::Core::DSysInfo::DeepinProfessional:
            // Check if develop mode
            QDBusInterface *dbusInterFace = new QDBusInterface("com.deepin.deepinid", "/com/deepin/deepinid", "com.deepin.deepinid");
            bool deviceMode = dbusInterFace->property("DeviceUnlocked").toBool();
            qInfo() << "DebListModel:" << "system editon:" << Dtk::Core::DSysInfo::uosEditionName() << "develop mode:" << deviceMode;
            kIsDevelopMode = deviceMode;
            delete dbusInterFace;
            break;
        case Dtk::Core::DSysInfo::isCommunityEdition():
        case Dtk::Core::DSysInfo::DeepinServer:
            kIsDevelopMode = true;
            break;
        default:
            kIsDevelopMode = true;
            break;
        }

#endif
    });

    return kIsDevelopMode;
}

DebApplicationHelper *DebApplicationHelper::instance()
{
    static DebApplicationHelper *phelper = new DebApplicationHelper;

    return phelper;
}

#define CAST_INT static_cast<int>

static QColor light_qpalette[QPalette::NColorRoles]{
    QColor("#414d68"),                           // WindowText
    QColor("#e5e5e5"),                           // Button
    QColor("#e6e6e6"),                           // Light
    QColor("#e5e5e5"),                           // Midlight
    QColor("#001A2E"),                           // Dark  -- changed origin is #e3e3e3
    QColor("#e4e4e4"),                           // Mid
    QColor("#414d68"),                           // Text
    Qt::black,                                   // BrightText
    QColor("#414d68"),                           // ButtonText
    Qt::white,                                   // Base
    QColor("#f8f8f8"),                           // Window
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),       // Shadow
    QColor("#0081ff"),                           // Highlight
    QColor(0, 45, 255, CAST_INT(0.5 * 255)),     // HighlightedText   //old Qt::white
    QColor("#0082fa"),                           // Link
    QColor("#ad4579"),                           // LinkVisited
    QColor(0, 0, 0, CAST_INT(0.03 * 255)),       // AlternateBase
    Qt::white,                                   // NoRole
    QColor(255, 255, 255, CAST_INT(0.8 * 255)),  // ToolTipBase
    QColor("#526A7F")                            // ToolTipText -- changed origin is Qt::black
};

static QColor dark_qpalette[QPalette::NColorRoles]{
    QColor("#c0c6d4"),                            // WindowText
    QColor("#444444"),                            // Button
    QColor("#484848"),                            // Light
    QColor("#474747"),                            // Midlight
    QColor("#C0C6D4"),                            // Dark      -- changed origin is #414141
    QColor("#434343"),                            // Mid
    QColor("#c0c6d4"),                            // Text
    Qt::white,                                    // BrightText
    QColor("#c0c6d4"),                            // ButtonText
    QColor(255, 255, 255, CAST_INT(0.05 * 255)),  // Base --- changed origin is #282828
    QColor("#252525"),                            // Window
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),        // Shadow
    QColor("#095EFF"),                            // Highlight         //old : #0081ff
    QColor("#0059D2"),                            // HighlightedText   //old: b8d3ff
    QColor("#0082fa"),                            // Link
    QColor("#ad4579"),                            // LinkVisited
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),        // AlternateBase
    Qt::black,                                    // NoRole
    QColor(45, 45, 45, CAST_INT(0.8 * 255)),      // ToolTipBase
    QColor("#6D7C88")                             // ToolTipText -- changed origin is #c0c6d4
};

static QColor light_dpalette[DPalette::NColorTypes]{
    QColor(),                               // NoType
    QColor(0, 0, 0, CAST_INT(255 * 0.03)),  // ItemBackground
    QColor("#414d68"),                      // TextTitle
    QColor("#609DC8"),                      // TextTips --- changed origin is #526A7F
    QColor("#FF5A5A"),                      // TextWarning -- changed origin is #FF5736
    QColor("#7C7C7C"),                      // TextLively  -- changed origin is #0082FA
    QColor("#417505"),                      // LightLively -- changed origin is #25b7ff
    QColor("#47790C"),                      // DarkLively -- changed origin is #0098ff
    QColor(0, 0, 0, CAST_INT(0.03 * 255))   // FrameBorder
};

static QColor dark_dpalette[DPalette::NColorTypes]{
    QColor(),                                     // NoType
    QColor(255, 255, 255, CAST_INT(255 * 0.05)),  // ItemBackground
    QColor("#c0c6d4"),                            // TextTitle
    QColor("#6D7C88"),                            // TextTips
    QColor("#9A2F2F"),                            // TextWarning -- changed origin is #FF5736
    QColor("#7C7C7C"),                            // TextLively -- changed origin is #0082FA
    QColor("#417505"),                            // LightLively -- changed origin is #0056c1
    QColor("#47790C"),                            // DarkLively  -- changed origin is #004c9c
    QColor(0, 0, 0, CAST_INT(0.08 * 255))         // FrameBorder
};

DPalette DebApplicationHelper::standardPalette(DGuiApplicationHelper::ColorType type) const
{
    DPalette pa;
    const QColor *qcolor_list, *dcolor_list;

    if (type == DarkType) {
        qcolor_list = dark_qpalette;
        dcolor_list = dark_dpalette;
    } else {
        qcolor_list = light_qpalette;
        dcolor_list = light_dpalette;
    }

    for (int i = 0; i < DPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);

        QColor color = qcolor_list[i];
        pa.setColor(DPalette::Active, role, color);
        generatePaletteColor(pa, role, type);
    }

    for (int i = 0; i < DPalette::NColorTypes; ++i) {
        DPalette::ColorType role = static_cast<DPalette::ColorType>(i);

        QColor color = dcolor_list[i];
        pa.setColor(DPalette::Active, role, color);
        generatePaletteColor(pa, role, type);
    }

    return *const_cast<const DPalette *>(&pa);
}

DPalette DebApplicationHelper::palette(const QWidget *widget, const QPalette &base) const
{
    Q_UNUSED(base)

    DPalette palette;

    do {
        // 存在自定义palette时应该根据其自定义的palette获取对应色调的DPalette
        const QPalette &wp = widget->palette();

        palette = standardPalette(toColorType(wp));

        // 关注控件palette改变的事件
        const_cast<QWidget *>(widget)->installEventFilter(const_cast<DebApplicationHelper *>(this));
    } while (false);

    return palette;
}

void DebApplicationHelper::setPalette(QWidget *widget, const DPalette &palette)
{
    // 记录此控件被设置过palette
    if (nullptr == widget)
        return;
    widget->setProperty("_d_set_palette", true);
    widget->setPalette(palette);
}

void DebApplicationHelper::resetPalette(QWidget *widget)
{
    if (nullptr == widget)
        return;
    widget->setProperty("_d_set_palette", QVariant());
    widget->setAttribute(Qt::WA_SetPalette, false);
}
