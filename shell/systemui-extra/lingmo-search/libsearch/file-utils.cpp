/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangpengfei <zhangpengfei@kylinos.cn>
 * Modified by: zhangzihao <zhangzihao@kylinos.cn>
 * Modified by: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#include "file-utils.h"
#include <QMutexLocker>
#include <gio/gdesktopappinfo.h>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDesktopServices>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QUrl>
#include <QClipboard>
#include <QFontMetrics>
#include <QTextBoundaryFinder>
#include "gobject-template.h"
#include "hanzi-to-pinyin.h"
#include "common.h"
#include "icon-loader.h"
#include "file-indexer-config.h"

using namespace LingmoUISearch;

FileUtils::FileUtils() {
}

std::string FileUtils::makeDocUterm(const QString& path) {
    return QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Md5).toHex().toStdString();
}

/**
 * @brief FileUtils::getFileIcon 获取文件图标
 * @param uri "file:///home/xxx/xxx/xxxx.txt"格式
 * @param checkValid
 * @return
 */
QIcon FileUtils::getFileIcon(const QString &uri, bool checkValid) {
    Q_UNUSED(checkValid)
    auto file = wrapGFile(g_file_new_for_uri(uri.toUtf8().constData()));
    auto info = wrapGFileInfo(g_file_query_info(file.get()->get(),
                              G_FILE_ATTRIBUTE_STANDARD_ICON,
                              G_FILE_QUERY_INFO_NONE,
                              nullptr,
                              nullptr));
    if(!G_IS_FILE_INFO(info.get()->get()))
        return IconLoader::loadIconQt("unknown",QIcon(":/res/icons/unknown.svg"));

    GIcon *g_icon = g_file_info_get_icon(info.get()->get());

    //do not unref the GIcon from info.
    if(G_IS_ICON(g_icon)) {
        const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON(g_icon));
        if(icon_names) {
            auto p = icon_names;
            while(*p) {
                QIcon icon = IconLoader::loadIconQt(*p);
                if(!icon.isNull()) {
                    return icon;
                } else {
                    p++;
                }
            }
        }
    }
    return IconLoader::loadIconQt("unknown", QIcon(":/res/icons/unknown.svg"));
}

QIcon FileUtils::getSettingIcon() {
    return IconLoader::loadIconQt("lingmo-control-center", QIcon(":/res/icons/lingmo-control-center.svg")); //返回控制面板应用图标
// 返回控制面板应用图标
}

bool FileUtils::isOrUnder(const QString& pathA, const QString& pathB)
{
    if (pathB == "/") {
        return true;
    }

    if(pathA.length() < pathB.length())
        return false;

    if(pathA == pathB || pathA.startsWith(pathB + "/"))
        return true;

    return false;
}

QMimeType FileUtils::getMimetype(const QString &path) {
    QMimeDatabase mdb;
    QMimeType type = mdb.mimeTypeForFile(path, QMimeDatabase::MatchContent);

    return type;
}

QStringList FileUtils::findMultiToneWords(const QString &hanzi) {
    QStringList output, results;
    HanZiToPinYin::getInstance()->getResults(hanzi.toStdString(), results);
    QString oneResult(results.join(""));
    QString firstLetter;
    for (QString & info : results) {
        if (!info.isEmpty())
            firstLetter += info.at(0);
    }
    return output << oneResult << firstLetter;
}

int FileUtils::openFile(QString &path, bool openInDir)
{
    int res = -1;
    if(openInDir) {
        QStringList list;
        list.append(path);
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.FileManager1",
                                                              "/org/freedesktop/FileManager1",
                                                              "org.freedesktop.FileManager1",
                                                              "ShowItems");
        message.setArguments({list, "lingmo-search"});
        QDBusMessage messageRes = QDBusConnection::sessionBus().call(message);
        if (QDBusMessage::ReplyMessage == messageRes.type()) {
            res = 0;
        } else {
            qDebug() << "Error! QDBusMessage reply error! ReplyMessage:" << messageRes.ReplyMessage;
            res = -1;
        }
    } else {
        auto file = wrapGFile(g_file_new_for_uri(QUrl::fromLocalFile(path).toString().toUtf8().constData()));
        auto fileInfo = wrapGFileInfo(g_file_query_info(file.get()->get(),
                                  "standard::*," "time::*," "access::*," "mountable::*," "metadata::*," "trash::*," G_FILE_ATTRIBUTE_ID_FILE,
                                  G_FILE_QUERY_INFO_NONE,
                                  nullptr,
                                  nullptr));
        QString mimeType = g_file_info_get_content_type (fileInfo.get()->get());
        if (mimeType == nullptr) {
            if (g_file_info_has_attribute(fileInfo.get()->get(), "standard::fast-content-type")) {
                mimeType = g_file_info_get_attribute_string(fileInfo.get()->get(), "standard::fast-content-type");
            }
        }

        GError *error = nullptr;
        GAppInfo *info  = nullptr;
        /*
        * g_app_info_get_default_for_type function get wrong default app, so we get the
        * default app info from mimeapps.list, and chose the right default app for mimeType file
        */
        QString mimeAppsListPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
           + "/.config/mimeapps.list";
        GKeyFile *keyfile = g_key_file_new();
        gboolean ret = g_key_file_load_from_file(keyfile, mimeAppsListPath.toUtf8(), G_KEY_FILE_NONE, &error);
        if (false == ret) {
            qWarning()<<"load mimeapps list error msg"<<error->message;
            info = g_app_info_get_default_for_type(mimeType.toUtf8().constData(), false);
            g_error_free(error);
        } else {
            gchar *desktopApp = g_key_file_get_string(keyfile, "Default Applications", mimeType.toUtf8(), &error);
            if (nullptr != desktopApp) {
                info = (GAppInfo*)g_desktop_app_info_new(desktopApp);
                g_free (desktopApp);
            } else {
                info = g_app_info_get_default_for_type(mimeType.toUtf8().constData(), false);
            }
        }
        g_key_file_free (keyfile);
        if (!G_IS_APP_INFO(info)) {
            res = -1;
        } else {
            bool isSuccess(false);
            QDBusInterface * appLaunchInterface = new QDBusInterface(QStringLiteral("com.lingmo.ProcessManager"),
                                                                     QStringLiteral("/com/lingmo/ProcessManager/AppLauncher"),
                                                                     QStringLiteral("com.lingmo.ProcessManager.AppLauncher"),
                                                                     QDBusConnection::sessionBus());
            if (!appLaunchInterface->isValid()) {
                qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
                isSuccess = false;
            } else {
                appLaunchInterface->setTimeout(10000);
                QDBusReply<void> reply = appLaunchInterface->call("LaunchDefaultAppWithUrl", QUrl::fromLocalFile(path).toString());
                if(reply.isValid()) {
                    isSuccess = true;
                } else {
                    qWarning() << "ProcessManager dbus called failed!" << reply.error();
                    isSuccess = false;
                }
            }
            if (appLaunchInterface) {
                delete appLaunchInterface;
            }
            appLaunchInterface = nullptr;
            
            if (!isSuccess){
                QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            }
            res = 0;
        }
        g_object_unref(info);
    }
    return res;
}

bool FileUtils::copyPath(QString &path)
{
    QApplication::clipboard()->setText(path);
    return true;
}

QString FileUtils::escapeHtml(const QString &str)
{
    QString temp = str;
    temp.replace("<", "&lt;");
    temp.replace(">", "&gt;");
    return temp;
}

QString FileUtils::getSnippet(const std::string &myStr, uint start, const QString &keyword)
{
    QFont boldFont(qApp->font().family());
    boldFont.setPointSizeF(qApp->font().pointSizeF() + 2);
    boldFont.setWeight(QFont::Bold);
    QFontMetricsF boldMetricsF(boldFont);

    uint strLength = 240;
    bool elideLeft(false);
    std::string sub = myStr.substr(start, strLength);
    QString content = QString::fromStdString(sub);

    //不够截往前补
    if (start + strLength > myStr.length()) {
        //新的起始位置
        int newStart = myStr.length() - strLength;

        if (myStr.length() < strLength) {
            newStart = 0;
            sub = myStr;
        } else {
            sub = myStr.substr(newStart, strLength);
        }

        if (horizontalAdvanceContainsKeyword(QString::fromStdString(myStr.substr(newStart, start)) + boldMetricsF.horizontalAdvance(keyword), keyword) > 2 * LABEL_MAX_WIDTH) {
            if (horizontalAdvanceContainsKeyword(QString::fromStdString(myStr.substr(start)), keyword) <= 2 * LABEL_MAX_WIDTH) {
                elideLeft = true;
            } else {
                sub = myStr.substr(start);
            }
        }
        content = QString::fromStdString(sub);
    }

    QFont font(qApp->font().family());
    font.setPointSizeF(qApp->font().pointSizeF());
    QFontMetricsF fontMetricsF(font);

    qreal blockLength = 0;
    qreal total = 0;
    int lineCount = 0;
    int normalLength = 0;
    int boldLength = 0;

    QString snippet;
    int boundaryStart = 0;
    int boundaryEnd = 0;
    QTextBoundaryFinder fm(QTextBoundaryFinder::Grapheme, content);

    if (!elideLeft) {
        for (;fm.position() != -1;fm.toNextBoundary()) {
            boundaryEnd = fm.position();
            QString word = content.mid(boundaryStart, boundaryEnd - boundaryStart);
            if (boundaryStart == boundaryEnd) {
                continue;
            }

            if (keyword.toUpper().contains(word.toUpper())) {
                if (normalLength) {
                    total += fontMetricsF.horizontalAdvance(content.mid(boundaryStart - normalLength, normalLength));
                    normalLength = 0;
                    blockLength = 0;
                }
                boldLength += (boundaryEnd - boundaryStart);
                blockLength = boldMetricsF.horizontalAdvance(content.mid(boundaryEnd - boldLength, boldLength));
            } else {
                if (boldLength) {
                    total += boldMetricsF.horizontalAdvance(content.mid(boundaryStart - boldLength, boldLength));
                    boldLength = 0;
                    blockLength = 0;
                }
                normalLength += (boundaryEnd - boundaryStart);
                blockLength = fontMetricsF.horizontalAdvance(content.mid(boundaryEnd - normalLength, normalLength));

            }

            if (total + blockLength >= LABEL_MAX_WIDTH && lineCount == 0) {
                if (total + blockLength > LABEL_MAX_WIDTH) {
                    fm.toPreviousBoundary();
                    snippet.append("\n");
                } else {
                    snippet.append(word).append("\n");
                    boundaryStart = boundaryEnd;
                }
                normalLength = 0;
                boldLength = 0;
                lineCount++;
                total = 0;
                blockLength = 0;
                continue;
            } else if (total + blockLength >= LABEL_MAX_WIDTH && lineCount == 1) {
                qreal distance = 0;
                qreal wordSize = 0;
                if (total + blockLength > LABEL_MAX_WIDTH) {
                    boundaryEnd = boundaryStart;
                    fm.toPreviousBoundary();
                } else {
                    snippet.append(word);
                }
                while (wordSize < fontMetricsF.horizontalAdvance("…")) {
                    boundaryStart = fm.position();

                    wordSize += keyword.toUpper().contains(content.mid(boundaryStart, boundaryEnd - boundaryStart).toUpper()) ?
                                boldMetricsF.horizontalAdvance(content.mid(boundaryStart, boundaryEnd - boundaryStart))
                              : fontMetricsF.horizontalAdvance(content.mid(boundaryStart, boundaryEnd - boundaryStart));
                    distance += (boundaryEnd - boundaryStart);
                    boundaryEnd = boundaryStart;
                    fm.toPreviousBoundary();
                }
                snippet = snippet.left(snippet.size() - distance);
                snippet.append("…");
                break;
            }
            snippet.append(word);
            boundaryStart = boundaryEnd;
        }
    } else {
        boundaryEnd = content.size();
        for (fm.toEnd(); fm.position() != -1; fm.toPreviousBoundary()) {
            boundaryStart = fm.position();
            if (boundaryEnd == boundaryStart) {
                continue;
            }

            QString word = content.mid(boundaryStart, boundaryEnd - boundaryStart);
            if (keyword.toUpper().contains(word.toUpper())) {
                if (normalLength) {
                    total += fontMetricsF.horizontalAdvance(content.mid(boundaryEnd, normalLength));
                    normalLength = 0;
                    blockLength = 0;
                }
                boldLength += (boundaryEnd - boundaryStart);
                blockLength = boldMetricsF.horizontalAdvance(content.mid(boundaryStart, boldLength));
            } else {
                if (boldLength) {
                    total += boldMetricsF.horizontalAdvance(content.mid(boundaryEnd, boldLength));
                    boldLength = 0;
                    blockLength = 0;
                }
                normalLength += (boundaryEnd - boundaryStart);
                blockLength = fontMetricsF.horizontalAdvance(content.mid(boundaryStart, normalLength));

            }

            if (total + blockLength >= LABEL_MAX_WIDTH && lineCount == 0) {
                if (total + blockLength > LABEL_MAX_WIDTH) {
                    fm.toNextBoundary();
                    snippet.prepend("\n");
                } else {
                    snippet.prepend(word).prepend("\n");
                    boundaryStart = boundaryEnd;
                }
                normalLength = 0;
                boldLength = 0;
                lineCount++;
                total = 0;
                blockLength = 0;
                continue;
            } else if (total + blockLength >= LABEL_MAX_WIDTH && lineCount == 1) {
                qreal distance = 0;
                qreal wordSize = 0;
                if (total + blockLength > LABEL_MAX_WIDTH) {
                    boundaryStart = boundaryEnd;
                    fm.toNextBoundary();
                } else {
                    snippet.prepend(word);
                }
                while (wordSize < fontMetricsF.horizontalAdvance("…")) {
                    boundaryEnd = fm.position();
                    QString firstLetter = content.mid(boundaryStart, boundaryEnd - boundaryStart);
                    wordSize += keyword.toUpper().contains(firstLetter.toUpper()) ?
                                boldMetricsF.horizontalAdvance(firstLetter) : fontMetricsF.horizontalAdvance(firstLetter);
                    distance += (boundaryEnd - boundaryStart);
                    boundaryStart = boundaryEnd;
                    fm.toNextBoundary();
                }
                snippet = snippet.right(snippet.size() - distance);
                snippet.prepend("…");
                break;
            }
            snippet.prepend(word);
            boundaryEnd = boundaryStart;
        }
    }

    return snippet;
}

bool FileUtils::isOpenXMLFileEncrypted(const QString &path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QByteArray encrypt = file.read(4);
    file.close();
    if (encrypt.length() < 4) {
        qDebug() << "Reading file error!" << path;
        return true;
    }
    //比较前四位是否为对应值来判断OpenXML类型文件是否加密
    if ((encrypt[0] & 0x50) && (encrypt[1] & 0x4b) && (encrypt[2] & 0x03) && (encrypt[3] & 0x04)) {
        return false;
    } else {
        qDebug() << "Encrypt!" << path;
        return true;
    }
}
//todo: only support docx, pptx, xlsx
bool FileUtils::isEncrypedOrUnsupport(const QString& path, const QString& suffix)
{
    QMimeType type = FileUtils::getMimetype(path);
    QString name = type.name();

    if(name == "application/zip") {
        if (suffix == "docx" || suffix == "pptx" || suffix == "xlsx") {

            return FileUtils::isOpenXMLFileEncrypted(path);
        } else if (suffix == "uot" || suffix == "uos" || suffix == "uop") {
            return false;

        } else if (suffix == "ofd") {
            return false;

        } else {
            return true;
        }
    } else if(name == "text/plain") {
        if(suffix.endsWith("txt"))
            return false;
        return true;
    } else if(name == "text/html") {
        if(suffix.endsWith("html"))
            return false;
        return true;
    } else if(type.inherits("application/msword") || type.name() == "application/x-ole-storage") {
        if(suffix == "doc" || suffix == "dot" || suffix == "wps" || suffix == "ppt" ||
                suffix == "pps" || suffix == "dps" || suffix == "et" || suffix == "xls" || suffix == "uof") {
            return false;
        }
        return true;
    } else if(name == "application/pdf") {
        if(suffix == "pdf")
            return false;
        return true;

    } else if(name == "application/xml" || name == "application/uof") {
        if(suffix == "uof") {
            return false;
        }
        return true;

    } else if (FileIndexerConfig::getInstance()->ocrContentIndexTarget()[suffix]) {
        return !isOcrSupportSize(path);
    } else {
//        qInfo() << "Unsupport format:[" << path << "][" << type.name() << "]";
        return true;
    }
}

bool FileUtils::isOcrSupportSize(QString path)
{
/*
    bool res;
    Pix *image = pixRead(path.toStdString().data());
    if (image->h < OCR_MIN_SIZE or image->w < OCR_MIN_SIZE) {//限制图片像素尺寸
        qDebug() << "file:" << path << "is not right size.";
        res = false;
    } else
        res = true;

    pixDestroy(&image);
    return res;
*/
    QImage file(path);
    if (file.height() < OCR_MIN_SIZE or file.width() < OCR_MIN_SIZE) {//限制图片像素尺寸
//        qDebug() << "file:" << path << "is not right size.";
        return false;
    } else
        return true;
}

QString FileUtils::getHtmlText(const QString &text, const QString &keyword)
{
    QString htmlString = QString("<style>"
                         "span {"
                         "font-size:%0pt;"
                         "font-weight:bold;"
                         "}"
                         "</style>").arg(qApp->font().pointSizeF() + 2);
    bool boldOpenned = false;

    QTextBoundaryFinder bf(QTextBoundaryFinder::Grapheme, text);
    int start = 0;
    for (;bf.position() != -1; bf.toNextBoundary()) {
        int end = bf.position();
        if (end == start) {
            continue;
        }
        if (keyword.toUpper().contains(text.mid(start, end - start).toUpper())) {
            if(! boldOpenned) {
                boldOpenned = true;
                htmlString.append(QString("<span>"));
            }
            htmlString.append(FileUtils::escapeHtml(text.mid(start, end - start)));
        } else {
            if(boldOpenned) {
                boldOpenned = false;
                htmlString.append(QString("</span>"));
            }
            htmlString.append(FileUtils::escapeHtml(text.mid(start, end - start)));
        }
        start = end;
    }

    htmlString.replace("\n", "<br />");//替换换行符
    return "<pre>" + htmlString + "</pre>";
}

QString FileUtils::setAllTextBold(const QString &name)
{
    return QString("<h3 style=\"font-weight:normal;\"><pre>%1</pre></h3>").arg(escapeHtml(name));
}

QString FileUtils::wrapData(QLabel *p_label, const QString &text)
{
    QString wrapText = text;

    QFontMetrics fontMetrics = p_label->fontMetrics();
    int textSize = fontMetrics.horizontalAdvance(wrapText);

    if(textSize > LABEL_MAX_WIDTH){
        int lastIndex = 0;
        int count = 0;

        for(int i = lastIndex; i < wrapText.length(); i++) {

            if(fontMetrics.horizontalAdvance(wrapText.mid(lastIndex, i - lastIndex)) == LABEL_MAX_WIDTH) {
                lastIndex = i;
                wrapText.insert(i, '\n');
                count++;
            } else if(fontMetrics.horizontalAdvance(wrapText.mid(lastIndex, i - lastIndex)) > LABEL_MAX_WIDTH) {
                lastIndex = i;
                wrapText.insert(i - 1, '\n');
                count++;
            } else {
                continue;
            }

            if(count == 2){
                break;
            }
        }
    }
    return wrapText;
}

qreal FileUtils::horizontalAdvanceContainsKeyword(const QString &content, const QString &keyword)
{
    QFont boldFont(qApp->font().family());
    boldFont.setPointSizeF(qApp->font().pointSizeF() + 2);
    boldFont.setWeight(QFont::Bold);
    QFontMetricsF boldMetricsF(boldFont);

    QFont font(qApp->font().family());
    font.setPointSizeF(qApp->font().pointSizeF());
    QFontMetricsF fontMetricsF(font);

    QTextBoundaryFinder fm(QTextBoundaryFinder::Grapheme, content);
    int start = 0;

    qreal contentSize = 0;
    int boldLength = 0;
    int normalLength = 0;

    for (;fm.position() != -1;fm.toNextBoundary()) {
        int end = fm.position();
        if (end == start) {
            continue;
        }
        QString letter = content.mid(start, end - start);

        if (keyword.toUpper().contains(letter.toUpper())) {
            if (normalLength) {
                contentSize += fontMetricsF.horizontalAdvance(content.mid(start - normalLength, normalLength));
                normalLength = 0;
            }
            boldLength += (end - start);
        } else {
            if (boldLength) {
                contentSize += boldMetricsF.horizontalAdvance(content.mid(start - boldLength, boldLength));
                boldLength = 0;
            }
            normalLength += (end - start);
        }
        start = end;
    }

    if (boldLength) {
        contentSize += boldMetricsF.horizontalAdvance(content.right(boldLength));
    }
    if (normalLength) {
        contentSize += fontMetricsF.horizontalAdvance(content.right(normalLength));
    }
    return contentSize;
}

QString FileUtils::getSnippetWithoutKeyword(const QString &content, int lineCount) {
    QString snippet;
    int numOfLine = 0;
    QFont font(qApp->font().family());
    font.setPointSizeF(qApp->font().pointSizeF());
    QFontMetricsF fontMetricsF(font);

    qreal length = 0;
    int wordCount = 0;
    int boundaryStart = 0;
    int boundaryEnd = 0;
    QTextBoundaryFinder fm(QTextBoundaryFinder::Grapheme, content);

    for(;fm.position() != -1;fm.toNextBoundary()) {
        boundaryEnd = fm.position();
        if (boundaryEnd == boundaryStart) {
            continue;
        }
        if (numOfLine == lineCount) {
            break;
        }
        QString word = content.mid(boundaryStart, boundaryEnd - boundaryStart);
        wordCount += boundaryEnd - boundaryStart;
        length = fontMetricsF.horizontalAdvance(content.mid(boundaryEnd - wordCount, wordCount));

        if (length >= LABEL_MAX_WIDTH || word == "\n") {
            if (word == "\n") {
                boundaryStart = boundaryEnd;
            } else if (length > LABEL_MAX_WIDTH) {
                fm.toPreviousBoundary();
            } else {
                boundaryStart = boundaryEnd;
                snippet.append(word);
            }
            snippet.append("\n");

            numOfLine++;
            if (numOfLine == lineCount) {
                qreal distance = 1;//最后一位必然是\n
                qreal wordSize = 0;
                if (!(word == "\n" && length < LABEL_MAX_WIDTH)) {
                    if (length > LABEL_MAX_WIDTH) {
                        boundaryEnd = boundaryStart;
                    }
                    while (wordSize < fontMetricsF.horizontalAdvance("…")) {
                        boundaryStart = fm.position();

                        wordSize += fontMetricsF.horizontalAdvance(content.mid(boundaryStart, boundaryEnd - boundaryStart));
                        distance += (boundaryEnd - boundaryStart);
                        boundaryEnd = boundaryStart;
                        fm.toPreviousBoundary();
                    }
                }
                snippet = snippet.left(snippet.size() - distance);
                snippet.append("…");
                break;
            }
            length = 0;
            wordCount = 0;
            continue;
        }
        snippet.append(word);
        boundaryStart = boundaryEnd;
    }
    return snippet;
}
