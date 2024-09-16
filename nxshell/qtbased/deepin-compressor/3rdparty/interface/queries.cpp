// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "queries.h"

#include <DMainWindow>
#include <DDialog>
#include <DFontSizeManager>
#include <DLabel>
#include <DCheckBox>
#include <DPasswordEdit>

#include <QApplication>
#include <QImageReader>
#include <QLayout>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

DMainWindow *getMainWindow()
{
    foreach (QWidget *w, QApplication::allWidgets()) {
        QMainWindow *pWindow = qobject_cast<QMainWindow *>(w);
        if (pWindow) {
            DMainWindow *pp = qobject_cast<DMainWindow *>(pWindow);
            return pp;
        }
    }

    return nullptr;
}

static QPixmap renderSVG(const QString &filePath, const QSize &size)
{
    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    return pixmap;
}

NewStr autoCutText(const QString &text, DLabel *pDesLbl)
{
    if (text.isEmpty() || nullptr == pDesLbl) {
        return NewStr();
    }

    QFont font; // 应用使用字体对象
    QFontMetrics font_label(font);
    QString strText = text;
    int titlewidth = font_label.width(strText);
    QString str;
    NewStr newstr;
    int width = pDesLbl->width();
    if (titlewidth < width) {
        newstr.strList.append(strText);
        newstr.resultStr += strText;
    } else {
        for (int i = 0; i < strText.count(); i++) {
            str += strText.at(i);

            if (font_label.width(str) > width) { //根据label宽度调整每行字符数
                str.remove(str.count() - 1, 1);
                newstr.strList.append(str);
                newstr.resultStr += str + "\n";
                str.clear();
                --i;
            }
        }
        newstr.strList.append(str);
        newstr.resultStr += str;
    }
    newstr.fontHeifht = font_label.height();
    return newstr;
}

CustomDDialog::CustomDDialog(QWidget *parent)
    : DDialog(parent)
{

}

CustomDDialog::CustomDDialog(const QString &title, const QString &message, QWidget *parent)
    : DDialog(title, message, parent)
{

}

void CustomDDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        emit signalFontChange();
    }

    DDialog::changeEvent(event);
}

Query::Query(QObject *parent)
    : QObject(parent)
{

}

Query::~Query()
{

}

void Query::setParent(QWidget *pParent)
{
    m_pParent = pParent;
}

void Query::waitForResponse()
{
    QMutexLocker locker(&m_responseMutex);
    // 如果没有任何选择，等待响应
    if (!m_data.contains(QStringLiteral("response"))) {
        m_responseCondition.wait(&m_responseMutex);
    }
}

void Query::setResponse(const QVariant &response)
{
    // 唤醒响应
    m_data[QStringLiteral("response")] = response;
    m_responseCondition.wakeAll();
}

QString Query::toShortString(QString strSrc, int limitCounts, int left)
{
    left = (left >= limitCounts || left <= 0) ? limitCounts / 2 : left;
    int right = limitCounts - left;
    QString displayName = "";
    displayName = strSrc.length() > limitCounts ? strSrc.left(left) + "..." + strSrc.right(right) : strSrc;
    return displayName;
}


OverwriteQuery::OverwriteQuery(const QString &filename, QObject *parent)
    : Query(parent)
{
    m_data[QStringLiteral("filename")] = filename;
}

OverwriteQuery::~OverwriteQuery()
{

}

void OverwriteQuery::execute()
{
    // 文件名处理
    QUrl sourceUrl = QUrl::fromLocalFile(QDir::cleanPath(m_data.value(QStringLiteral("filename")).toString()));

    QString path = sourceUrl.toString();
    if (path.contains("file://")) {
        path.remove("file://");
    }

    if (path.contains("file:")) {
        path.remove("file:");
    }

    QFileInfo file(path);

    // 获取父窗口（居中显示）
    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    CustomDDialog *dialog = new CustomDDialog(m_pParent);
    dialog->setAccessibleName("Overwrite_dialog");
    dialog->setFixedWidth(380);
    // 设置对话框图标
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    dialog->setIcon(pixmap);

    // 显示文件名
    DLabel *pFileNameLbl = new DLabel;
    pFileNameLbl->setMinimumSize(QSize(280, 20));
    pFileNameLbl->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(pFileNameLbl, DFontSizeManager::T6, QFont::Medium);
    pFileNameLbl->setFixedWidth(340);

    m_strFileName = file.fileName();

    // 显示提示语
    DLabel *pTipLbl = new DLabel;
    pTipLbl->setFixedWidth(340);
    pTipLbl->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(pTipLbl, DFontSizeManager::T6, QFont::Medium);
    pTipLbl->setText(QObject::tr("Another file with the same name already exists, replace it?"));
//    pTipLbl->setWordWrap(true);
    m_strDesText = pTipLbl->text();

    // 应用到全部勾选
    DCheckBox *pApplyAllCkb = new DCheckBox(QObject::tr("Apply to all"));
    pApplyAllCkb->setAccessibleName("Applyall_btn");
    DFontSizeManager::instance()->bind(pApplyAllCkb, DFontSizeManager::T6, QFont::Medium);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    // 控件主题样式
    if (themeType == DGuiApplicationHelper::LightType) {    // 浅色
        setWidgetColor(pFileNameLbl, DPalette::ToolTipText, 0.7);
        setWidgetColor(pTipLbl, DPalette::ToolTipText, 1);
        setWidgetColor(pApplyAllCkb, DPalette::ToolTipText, 0.7);
    } else if (themeType == DGuiApplicationHelper::DarkType) {  // 深色
        setWidgetType(pFileNameLbl, DPalette::TextLively, 0.7);
        setWidgetType(pTipLbl, DPalette::TextLively, 1);
        setWidgetType(pApplyAllCkb, DPalette::TextLively, 0.7);
    }

    // 按钮
    dialog->addButton(QObject::tr("Skip", "button"));
    dialog->addButton(QObject::tr("Replace", "button"), true, DDialog::ButtonWarning);

    dialog->addContent(pTipLbl, Qt::AlignHCenter);
    dialog->addContent(pFileNameLbl, Qt::AlignHCenter);
    dialog->addContent(pApplyAllCkb, Qt::AlignHCenter);

    //setTabOrder需放在布局最后，否则不生效
    dialog->setTabOrder(pApplyAllCkb, dialog->getButton(0));
    dialog->setTabOrder(dialog->getButton(0), dialog->getButton(1));

    autoFeed(pFileNameLbl, pTipLbl, dialog);
    connect(dialog, &CustomDDialog::signalFontChange, this, [&]() {
        autoFeed(pFileNameLbl, pTipLbl, dialog);
    }, Qt::DirectConnection);

    // 操作结果
    const int mode = dialog->exec();

    if (-1 == mode) {
        setResponse(Result_Cancel);
    } else if (0 == mode) {
        if (pApplyAllCkb->isChecked()) {
            setResponse(Result_SkipAll);
        } else {
            setResponse(Result_Skip);
        }
    } else if (1 == mode) {
        if (pApplyAllCkb->isChecked()) {
            setResponse(Result_OverwriteAll);
        } else {
            setResponse(Result_Overwrite);
        }
    }

    delete dialog;
}

bool OverwriteQuery::responseCancelled()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Cancel;
}

bool OverwriteQuery::responseSkip()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Skip;
}

bool OverwriteQuery::responseSkipAll()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_SkipAll;
}

bool OverwriteQuery::responseOverwrite()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_Overwrite;
}

bool OverwriteQuery::responseOverwriteAll()
{
    return m_data.value(QStringLiteral("response")).toInt() == Result_OverwriteAll;
}

void OverwriteQuery::autoFeed(DLabel *label1, DLabel *label2, CustomDDialog *dialog)
{
    NewStr newstr = autoCutText(m_strDesText, label2);
    label2->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label2->setFixedHeight(height_lable);

    // 字符串太长的情况下用中间使用...
    QFont font;
    QFontMetrics elideFont(font);
    label1->setText(elideFont.elidedText(m_strFileName, Qt::ElideMiddle, 340));

    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        dialog->adjustSize();
    } else {
        dialog->setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight - m_iLabelOld1Height - m_iCheckboxOld1Height + height_lable + 2 * newstr.fontHeifht); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iLabelOld1Height = newstr.fontHeifht;
    m_iCheckboxOld1Height = newstr.fontHeifht;
    m_iDialogOldHeight = dialog->height();
}

void OverwriteQuery::setWidgetColor(QWidget *pWgt, DPalette::ColorRole ct, double alphaF)
{
    DPalette palette = DApplicationHelper::instance()->palette(pWgt);
    QColor color = palette.color(ct);
    color.setAlphaF(alphaF);
    palette.setColor(DPalette::Foreground, color);
    DApplicationHelper::instance()->setPalette(pWgt, palette);
}

void OverwriteQuery::setWidgetType(QWidget *pWgt, DPalette::ColorType ct, double alphaF)
{
    DPalette palette = DApplicationHelper::instance()->palette(pWgt);
    QColor color = palette.color(ct);
    color.setAlphaF(alphaF);
    palette.setColor(DPalette::Foreground, color);
    DApplicationHelper::instance()->setPalette(pWgt, palette);
}



PasswordNeededQuery::PasswordNeededQuery(const QString &strFileName, QObject *parent)
    : Query(parent)
{
    m_data[QStringLiteral("fileName")] = strFileName;
}

PasswordNeededQuery::~PasswordNeededQuery()
{

}

void PasswordNeededQuery::execute()
{
    // 获取父窗口（居中显示）
    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    CustomDDialog *dialog = new CustomDDialog(m_pParent);
    dialog->setAccessibleName("PasswordNeeded_dialog");
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(64, 64));
    dialog->setIcon(pixmap);
    dialog->setFixedWidth(380);

    // 加密文件名显示
    DLabel *pFileNameLbl = new DLabel(dialog);
//    pFileNameLbl->setFixedSize(300, 20);
    pFileNameLbl->setFixedWidth(340); //修复英文环境下提示语显示不全
    pFileNameLbl->setForegroundRole(DPalette::ToolTipText);
//    pFileNameLbl->setWordWrap(true);
    DFontSizeManager::instance()->bind(pFileNameLbl, DFontSizeManager::T6, QFont::Medium);
    QString archiveFullPath = m_data[QStringLiteral("fileName")].toString();
    QString fileName = toShortString(archiveFullPath.mid(archiveFullPath.lastIndexOf('/') + 1), 22, 11);
    pFileNameLbl->setText(fileName);
    pFileNameLbl->setAlignment(Qt::AlignCenter);
    pFileNameLbl->setToolTip(archiveFullPath);
    m_strFileName = fileName;

    // 提示语显示
    DLabel *pTipLbl = new DLabel(dialog);
    pTipLbl->setFixedWidth(340); //修复英文环境下提示语显示不全
    pTipLbl->setForegroundRole(DPalette::WindowText);
//    pTipLbl->setWordWrap(true);
    DFontSizeManager::instance()->bind(pTipLbl, DFontSizeManager::T6, QFont::Normal);
    pTipLbl->setText(tr("Encrypted file, please enter the password"));
    pTipLbl->setAlignment(Qt::AlignCenter);
    m_strDesText = pTipLbl->text();

    // 密码框
    DPasswordEdit *passwordedit = new DPasswordEdit(dialog);
    passwordedit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false); //隐藏密码时不能输入中文
    passwordedit->setFocusPolicy(Qt::StrongFocus);
    passwordedit->setFixedHeight(36);
    passwordedit->setFixedWidth(280);

    dialog->addButton(QObject::tr("Cancel", "button"), true, DDialog::ButtonNormal);
    dialog->addButton(QObject::tr("OK", "button"), true, DDialog::ButtonRecommend);
    dialog->getButton(1)->setEnabled(false);
    //确保输入的密码不为空
    connect(passwordedit, &DPasswordEdit::textChanged, passwordedit, [&]() {
        if (passwordedit->text().isEmpty()) {
            dialog->getButton(1)->setEnabled(false);
        } else {
            dialog->getButton(1)->setEnabled(true);
        }
    });

    //隐藏密码时不能输入中文,显示密码时可以输入中文
    connect(passwordedit, &DPasswordEdit::echoModeChanged, passwordedit, [&](bool echoOn) {
        passwordedit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, echoOn);
    });

    // 布局
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setSpacing(0);
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->addWidget(pFileNameLbl, 0, Qt::AlignCenter);
    mainlayout->addWidget(pTipLbl, 0, Qt::AlignCenter);
    mainlayout->addSpacing(10);
    mainlayout->addWidget(passwordedit, 0, Qt::AlignCenter);
    mainlayout->addSpacing(10);

    DWidget *widget = new DWidget(dialog);

    widget->setLayout(mainlayout);
    dialog->addContent(widget);

    if (m_pParent) {
        QRect mainWindowGeometr = m_pParent->geometry();
        dialog->move(mainWindowGeometr.topLeft().x() + (mainWindowGeometr.width() - dialog->width()) / 2, mainWindowGeometr.topLeft().y() - 50 + (mainWindowGeometr.height() - dialog->height()) / 2); //居中显示
    }

    passwordedit->lineEdit()->setFocus(); // 默认焦点落在密码框内

    autoFeed(pFileNameLbl, pTipLbl, dialog);
    connect(dialog, &CustomDDialog::signalFontChange, this, [&]() {
        autoFeed(pFileNameLbl, pTipLbl, dialog);
    }, Qt::DirectConnection);

    const int mode = dialog->exec();

    m_data[QStringLiteral("password")] = passwordedit->text();

    if (-1 == mode || 0 == mode) {
        setResponse(Result_Cancel);
    } else {
        setResponse(Result_Skip);
    }

    delete dialog;
}

void PasswordNeededQuery::autoFeed(DLabel *label1, DLabel *label2, CustomDDialog *dialog)
{
    NewStr newstr = autoCutText(m_strDesText, label2);
    label2->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label2->setFixedHeight(height_lable);

    // 字符串太长的情况下用中间使用...
    QFont font;
    QFontMetrics elideFont(font);
    label1->setText(elideFont.elidedText(m_strFileName, Qt::ElideMiddle, 340));

    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        dialog->adjustSize();
    } else {
        dialog->setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight - m_iLabelOld1Height + height_lable + newstr.fontHeifht); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iLabelOld1Height = newstr.fontHeifht;
    m_iDialogOldHeight = dialog->height();
}

bool PasswordNeededQuery::responseCancelled()
{
    return !m_data.value(QStringLiteral("response")).toBool();
}

QString PasswordNeededQuery::password()
{
    return m_data.value(QStringLiteral("password")).toString();
}

LoadCorruptQuery::LoadCorruptQuery(const QString &archiveFilename, QObject *parent)
{
    m_data[QStringLiteral("archiveFilename")] = archiveFilename;
}

LoadCorruptQuery::~LoadCorruptQuery()
{

}

void LoadCorruptQuery::execute()
{
    qInfo() << "Executing prompt";

    if (m_pParent == nullptr) {
        m_pParent = getMainWindow();
    }

    // 控件
    CustomDDialog *dialog = new CustomDDialog(m_pParent);
    dialog->setAccessibleName("LoadCorruptQuery_dialog");
    dialog->setFixedWidth(380);
    QPixmap pixmap = renderSVG(":assets/icons/deepin/builtin/icons/compress_warning_32px.svg", QSize(32, 32));
    dialog->setIcon(pixmap);

    DLabel *strlabel = new DLabel;
    strlabel->setFixedWidth(340);
    strlabel->setAlignment(Qt::AlignCenter);
//    strlabel->setWordWrap(true);
    strlabel->setForegroundRole(DPalette::ToolTipText);
    DFontSizeManager::instance()->bind(strlabel, DFontSizeManager::T6, QFont::Medium);
    strlabel->setText(tr("The archive is damaged"));
    m_strDesText = strlabel->text();

    dialog->addButton(tr("Open as read-only"));
    dialog->addButton(tr("Cancel", "button"), true, DDialog::ButtonRecommend);


    dialog->addContent(strlabel, Qt::AlignHCenter);

    autoFeed(strlabel, dialog);
    connect(dialog, &CustomDDialog::signalFontChange, this, [&]() {
        autoFeed(strlabel, dialog);
    }, Qt::DirectConnection);

    // 选择
    const int mode = dialog->exec();
    if (0 == mode) {
        setResponse(Result_Readonly);
    } else {
        setResponse(Result_Cancel);
    }

    delete dialog;
    dialog = nullptr;
}

bool LoadCorruptQuery::responseYes()
{
    return (m_data.value(QStringLiteral("response")).toInt() == Result_Readonly);
}

void LoadCorruptQuery::autoFeed(DLabel *label, CustomDDialog *dialog)
{
    NewStr newstr = autoCutText(m_strDesText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        dialog->adjustSize();
    } else {
        dialog->setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight + height_lable); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
    m_iDialogOldHeight = dialog->height();
}
