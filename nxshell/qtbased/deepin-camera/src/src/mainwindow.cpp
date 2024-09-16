// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "capplication.h"
#include "v4l2_core.h"
#include "datamanager.h"
#include "shortcut.h"
#include "ac-deepin-camera-define.h"
#include "imageitem.h"
#include "takephotosettingareawidget.h"
#include "photorecordbtn.h"
#include "switchcamerabtn.h"
#include "windowstatethread.h"
#include "camera.h"
#include "eventlogutils.h"
#include "config.h"
#include "globalutils.h"

#include <DLabel>
#include <DApplication>
#include <DSettingsDialog>
#include <DSettingsOption>
#include <DSettings>
#include <DLineEdit>
#include <DFileDialog>
#include <DDialog>
#include <DDesktopServices>
#include <DMessageBox>
#include <DComboBox>
#include <DWindowMinButton>
#include <DWindowCloseButton>
#include <DWindowMaxButton>
#include <DWindowOptionButton>
#include <DWindowQuitFullButton>
#include <DStyleHelper>
#include <DStyle>

#include <QListWidgetItem>
#include <QTextLayout>
#include <QStyleFactory>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFormLayout>
#include <QLibraryInfo>
#include <QLibrary>

#include <qsettingbackend.h>
#include <dsettingswidgetfactory.h>

extern "C" {
#include "encoder.h"
#include "load_libs.h"
extern int encodeenv;
}

using namespace dc;

const int CMainWindow::minWindowWidth = 680;
const int CMainWindow::minWindowHeight = 510;

const int rightOffset = 10;
const int SwitchcameraDiam = 40; //切换摄像头直径
const int photeRecordDiam = 64;
const int snapLabelDiam = 52;
const int switchBtnWidth = 50;
const int switchBtnHeight = 26;
const int labelCameraNameWidth = 150;
const int labelCameraNameHeight = 26;
const int labelFilterNameWidth = 150;
const int labelFilterNameHeight = 26;
const int pathEditWidth = 285;
const int defaultWindowWidth = 800;
const int defaultWindowHeight = 600;

static void workaround_updateStyle(QWidget *parent, const QString &theme)
{
    parent->setStyle(QStyleFactory::create(theme));
    for (auto obj : parent->children()) {
        QWidget *tmp_widget = qobject_cast<QWidget *>(obj);
        if (tmp_widget) {
            workaround_updateStyle(tmp_widget, theme);
        }
    }
}

static QString ElideText(const QString &text, const QSize &size,
                         QTextOption::WrapMode wordWrap, const QFont &font,
                         Qt::TextElideMode mode, int lineHeight, int lastLineWidth)
{
    int tmpheight = 0;
    QTextLayout textLayout(text);
    QString textlinestr = nullptr;
    QFontMetrics fontMetrics(font);
    textLayout.setFont(font);
    const_cast<QTextOption *>(&textLayout.textOption())->setWrapMode(wordWrap);
    textLayout.beginLayout();
    QTextLine tmptextline = textLayout.createLine();

    while (tmptextline.isValid()) {
        tmpheight += lineHeight;

        if (tmpheight + lineHeight >= size.height()) {
            textlinestr += fontMetrics.elidedText(text.mid(tmptextline.textStart() + tmptextline.textLength() + 1),
                                                  mode, lastLineWidth);
            break;
        }
        tmptextline.setLineWidth(size.width());
        const QString &tmp_str = text.mid(tmptextline.textStart(), tmptextline.textLength());

        if (tmp_str.indexOf('\n'))
            tmpheight += lineHeight;

        textlinestr += tmp_str;
        tmptextline = textLayout.createLine();

        if (tmptextline.isValid())
            textlinestr.append("\n");

    }

    textLayout.endLayout();

    if (textLayout.lineCount() == 1)
        textlinestr = fontMetrics.elidedText(textlinestr, mode, lastLineWidth);

    return textlinestr;
}

static QWidget *createFormatLabelOptionHandle(QObject *opt)
{
    DTK_CORE_NAMESPACE::DSettingsOption *option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(opt);
    DLabel *lab = new DLabel();
    DWidget *main = new DWidget();
    QHBoxLayout *layout = new QHBoxLayout;
    QWidget *optionWidget = new QWidget;
    QFormLayout *optionLayout = new QFormLayout(optionWidget);

    main->setLayout(layout);
    main->setContentsMargins(0, 0, 0, 0);
    main->setMinimumWidth(240);
    main->setMinimumHeight(20);
    layout->addWidget(lab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignVCenter);
    lab->setObjectName(OPTION_FORMAT_LABER);
    lab->setAccessibleName(OPTION_FORMAT_LABER);
    lab->setMinimumHeight(20);

    lab->setText(option->value().toString());
    QFont ft("SourceHanSansSC");
    ft.setPixelSize(15);
    ft.setWeight(QFont::Medium);
    lab->setFont(ft);
    lab->setAlignment(Qt::AlignVCenter);
    lab->show();
    optionWidget->setObjectName(OPTION_FRAME);
    optionWidget->setAccessibleName(OPTION_FRAME);
    optionLayout->setContentsMargins(0, 0, 0, 0);

    optionLayout->setSpacing(0);
    DLabel *dLabel = new DLabel(QObject::tr(option->name().toStdString().c_str()));
    dLabel->setMinimumHeight(20);
    dLabel->setAlignment(Qt::AlignVCenter);
    optionLayout->addRow(dLabel, main);
    optionWidget->setContentsMargins(0, 0, 0, 0);
    workaround_updateStyle(optionWidget, "light");

    return optionWidget;
}

static QWidget *createPicSelectableLineEditOptionHandle(QObject *opt)
{
    DTK_CORE_NAMESPACE::DSettingsOption *option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(opt);
    option->setObjectName(PIC_OPTION);
    DLineEdit *picPathLineEdit = new DLineEdit;//文本框
    DWidget *main = new DWidget;
    QHBoxLayout *horboxlayout = new QHBoxLayout;
    DSuggestButton *icon = new DSuggestButton(main);
    QWidget *optionWidget = new QWidget;
    QFormLayout *optionLayout = new QFormLayout(optionWidget);
    DDialog *optinvaliddialog = new DDialog(optionWidget);
    //主目录相对路径
    QString relativeHomePath = QString("~") + QDir::separator();
    //保存上一次正确的文件夹路径，否则为默认文件夹
    static QString lastPicPath = nullptr;

    main->setLayout(horboxlayout);
    main->setContentsMargins(0, 0, 0, 0);
    horboxlayout->setAlignment(Qt::AlignTop);
    horboxlayout->setContentsMargins(0, 0, 0, 0);
    QFont ft("SourceHanSansSC");
    ft.setPixelSize(14);
    ft.setWeight(QFont::Medium);
    picPathLineEdit->setFont(ft);

    icon->setAutoDefault(false);
    icon->setObjectName(BUTTON_OPTION_PIC_LINE_EDIT);
    icon->setAccessibleName(BUTTON_OPTION_PIC_LINE_EDIT);
    picPathLineEdit->setFixedHeight(37);
    picPathLineEdit->setObjectName(OPTION_PIC_SELECTABLE_LINE_EDIT);
    picPathLineEdit->setAccessibleName(OPTION_PIC_SELECTABLE_LINE_EDIT);
    //获取当前设置的照片保存路径
    QString curPicSettingPath = option->value().toString();
    if (curPicSettingPath.contains(relativeHomePath))
        curPicSettingPath.replace(0, 1, QDir::homePath());

    QDir curPicSettingPathdir(curPicSettingPath);

    //路径不存在
    if (!curPicSettingPathdir.exists()) {
        picPathLineEdit->setText(option->defaultValue().toString());//更改文本框为默认路径~/Pictures/Camera
        curPicSettingPath = option->defaultValue().toString();
        if (curPicSettingPath.contains(relativeHomePath))
            curPicSettingPath.replace(0, 1, QDir::homePath());
    }
    else
        picPathLineEdit->setText(option->value().toString());//更改文本框为设置的路径

    lastPicPath = curPicSettingPath;

    QFontMetrics tem_fontmetrics = picPathLineEdit->fontMetrics();
    QString picStrElideText = ElideText(picPathLineEdit->text(), {pathEditWidth, tem_fontmetrics.height()}, QTextOption::WrapAnywhere,
                                        picPathLineEdit->font(), Qt::ElideMiddle, tem_fontmetrics.height(), pathEditWidth);
    picPathLineEdit->setText(picStrElideText);

    option->connect(picPathLineEdit, &DLineEdit::focusChanged, [ = ](bool on) {
        if (on) {
            QString settingPicPathValue = option->value().toString();

            if (settingPicPathValue.front() == '~')
                settingPicPathValue.replace(0, 1, QDir::homePath());

            QDir dir(settingPicPathValue);

            if (!dir.exists())
                picPathLineEdit->setText(option->defaultValue().toString());//更改文本框为默认路径~/Pictures/Camera
            else
                picPathLineEdit->setText(option->value().toString());//更改文本框为设置的路径
        }
    });

    icon->setIcon(DStyleHelper(icon->style()).standardIcon(DStyle::SP_SelectElement, nullptr));
    icon->setIconSize(QSize(24, 24));
    icon->setFixedSize(41, 37);

    horboxlayout->addWidget(picPathLineEdit);
    horboxlayout->addWidget(icon);

    optionWidget->setObjectName(OPTION_FRAME);
    optionWidget->setAccessibleName(OPTION_FRAME);
    optionWidget->setFixedHeight(37);

    optionLayout->setContentsMargins(0, 0, 0, 0);
    optionLayout->setSpacing(0);
    optionLayout->setVerticalSpacing(0);
    optionLayout->setMargin(0);

    main->setMinimumWidth(240);
    DLabel *lab = new DLabel(QObject::tr(option->name().toStdString().c_str()));
    lab->setAlignment(Qt::AlignVCenter);
    optionLayout->addRow(lab, main);
    workaround_updateStyle(optionWidget, "light");
    optinvaliddialog->setObjectName(OPTION_INVALID_DIALOG);
    optinvaliddialog->setAccessibleName(OPTION_INVALID_DIALOG);
    optinvaliddialog->setIcon(QIcon(":/images/icons/warning.svg"));
    optinvaliddialog->setMessage(QObject::tr("You don't have permission to operate this folder"));
    optinvaliddialog->setWindowFlags(optinvaliddialog->windowFlags() | Qt::WindowStaysOnTopHint);
    optinvaliddialog->addButton(QObject::tr("Close"), true, DDialog::ButtonRecommend);

    /**
     *@brief 照片文本框路径是否正确
     *@param name:图片保存路径, alert:提示
     */
    auto validate = [ = ](QString name, bool alert = true) -> bool {
        Q_UNUSED(alert);
        name = name.trimmed();

        if (name.isEmpty())
            return false;

        if (name.size() > 2 && name.contains(relativeHomePath))
            name.replace(0, 1, QDir::homePath());

        QFileInfo fileinfo(name);
        QDir dir(name);

        if (fileinfo.exists())
        {
            QString picStr("/home/");
            if (!fileinfo.isDir())
                return false;

            if (picStr.contains(name))
                return false;

            if (!fileinfo.isReadable() || !fileinfo.isWritable())
                return false;

        } else
        {
            if (relativeHomePath.contains(name))
                return false;

            if (dir.cdUp()) {
                QFileInfo ch(dir.path());

                if (!ch.isReadable() || !ch.isWritable())
                    return false;
            }
            return false;
        }
        return true;
    };

    /**
     *@brief 打开照片文件夹按钮槽函数
     */
    option->connect(icon, &DPushButton::clicked, [ = ]() {

        QString selectPicSavePath;
        QString tmplastpicpath = lastPicPath;

        if (tmplastpicpath.contains(relativeHomePath))
            tmplastpicpath.replace(0, 1, QDir::homePath());

        QDir dir(tmplastpicpath);

        if (!dir.exists()) {
            //设置文本框为新的路径
            tmplastpicpath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                             + QDir::separator() + "Camera";
            QDir defaultdir(tmplastpicpath);

            if (!defaultdir.exists())
                dir.mkdir(tmplastpicpath);

            picPathLineEdit->setText(option->defaultValue().toString());
        }
#ifdef UNITTEST
        selectPicSavePath = "~/Pictures/";
#else
        //打开文件夹
        selectPicSavePath = DFileDialog::getExistingDirectory(nullptr, QObject::tr("Open folder"),
                                                              tmplastpicpath,
                                                              DFileDialog::ShowDirsOnly | DFileDialog::DontResolveSymlinks);
#endif

        if (validate(selectPicSavePath, false)) {
            option->setValue(selectPicSavePath);
            QFontMetrics tem_fontmetrics = picPathLineEdit->fontMetrics();
            QString elideText = ElideText(picPathLineEdit->text(), {pathEditWidth, tem_fontmetrics.height()}, QTextOption::WrapAnywhere,
                                                picPathLineEdit->font(), Qt::ElideMiddle, tem_fontmetrics.height(), pathEditWidth);
            picPathLineEdit->setText(elideText);
        }

        QString qlastPath = Settings::get().getOption("base.save.picdatapath").toString();
        qDebug() << QString("-----------------qlastPath: %1").arg(qlastPath);

        QFileInfo fm(selectPicSavePath);

        if ((!fm.isReadable() || !fm.isWritable()) && !selectPicSavePath.isEmpty())
            optinvaliddialog->show();
    });

    /**
     *@brief 照片文本框编辑完成槽函数
     */
    option->connect(picPathLineEdit, &DLineEdit::editingFinished, option, [ = ]() {

        QString curPicEditStr = picPathLineEdit->text();
        QDir dir(curPicEditStr);
        //获取当前文本框路径
        QString curPicElidePath = ElideText(curPicEditStr, {pathEditWidth, tem_fontmetrics.height()}, QTextOption::WrapAnywhere,
                                               picPathLineEdit->font(), Qt::ElideMiddle, tem_fontmetrics.height(), pathEditWidth);

        if (!validate(curPicEditStr, false)) {
            QFileInfo fn(dir.path());

            if ((!fn.isReadable() || !fn.isWritable()) && !curPicEditStr.isEmpty()) {
                // 设置为默认路径
                curPicEditStr = option->defaultValue().toString();
                option->setValue(option->defaultValue().toString());
                curPicElidePath = option->defaultValue().toString();
            }
        }

        if (!picPathLineEdit->lineEdit()->hasFocus()) {

            //文本框路径有效
            if (validate(curPicEditStr, false)) {
                option->setValue(curPicEditStr);
            }
            //文本框路径无效
            else {
                // 设置为默认路径
                curPicEditStr = option->defaultValue().toString();
                if (curPicEditStr.front() == '~')
                    curPicEditStr.replace(0, 1, QDir::homePath());
                option->setValue(option->defaultValue().toString());
                curPicElidePath = option->defaultValue().toString();
            }

            lastPicPath = curPicEditStr;
            picPathLineEdit->setText(curPicElidePath);
        }
    });

    option->connect(picPathLineEdit, &DLineEdit::textEdited, option, [ = ](const QString & newStr) {
        validate(newStr);
    });

    /**
     *@brief 照片后台设置参数更改槽函数
     */
    option->connect(option, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged, picPathLineEdit,
    [ = ](const QVariant & value) {
        auto pi = ElideText(value.toString(), {pathEditWidth, tem_fontmetrics.height()}, QTextOption::WrapAnywhere,
                            picPathLineEdit->font(), Qt::ElideMiddle, tem_fontmetrics.height(), pathEditWidth);
        picPathLineEdit->setText(pi);
        qDebug() << "picPathLineEdit text:" << picPathLineEdit->text() << endl;
        lastPicPath = value.toString();
        option->setValue(value.toString());
        qDebug() << "save pic last path:" << value.toString() << endl;
        picPathLineEdit->update();
    });

    return  optionWidget;
}

static QWidget *createVdSelectableLineEditOptionHandle(QObject *opt)
{
    DTK_CORE_NAMESPACE::DSettingsOption *option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(opt);
    option->setObjectName(VIDEO_OPTION);

    DLineEdit *videoPathLineEdit = new DLineEdit;
    DWidget *main = new DWidget;
    QHBoxLayout *horboxlayout = new QHBoxLayout;
    DSuggestButton *icon = new DSuggestButton(main);
    QWidget *optionWidget = new QWidget;
    QFormLayout *optionLayout = new QFormLayout(optionWidget);
    DDialog *optinvaliddialog = new DDialog(optionWidget);
    //主目录相对路径
    QString relativeHomePath = QString("~") + QDir::separator();
    static QString lastVideoPath = nullptr;

    main->setLayout(horboxlayout);
    main->setContentsMargins(0, 0, 0, 0);
    horboxlayout->setAlignment(Qt::AlignTop);
    horboxlayout->setContentsMargins(0, 0, 0, 0);
    QFont ft("SourceHanSansSC");
    ft.setPixelSize(14);
    ft.setWeight(QFont::Medium);
    videoPathLineEdit->setFont(ft);
    icon->setAutoDefault(false);
    icon->setObjectName(BUTTON_OPTION_VIDEO_LINE_EDIT);
    icon->setAccessibleName(BUTTON_OPTION_VIDEO_LINE_EDIT);
    videoPathLineEdit->setFixedHeight(37);
    videoPathLineEdit->setObjectName(OPTION_VIDEO_SELECTABLE_LINE_EDIT);
    videoPathLineEdit->setAccessibleName(OPTION_VIDEO_SELECTABLE_LINE_EDIT);
    QString curVideoSettingPath = option->value().toString();
    if (curVideoSettingPath.contains(relativeHomePath))
        curVideoSettingPath.replace(0, 1, QDir::homePath());

    QDir curVideoSettingPathdir(curVideoSettingPath);

    //路径不存在
    if (!curVideoSettingPathdir.exists()) {
        videoPathLineEdit->setText(option->defaultValue().toString());//更改文本框为默认路径~/Videos/Camera
        curVideoSettingPath = option->defaultValue().toString();
        if (curVideoSettingPath.contains(relativeHomePath))
            curVideoSettingPath.replace(0, 1, QDir::homePath());
    }
    else
        videoPathLineEdit->setText(option->value().toString());//更改文本框为设置的路径

    lastVideoPath = curVideoSettingPath;

    QFontMetrics tem_fontmetrics = videoPathLineEdit->fontMetrics();
    QString vdStrElideText = ElideText(videoPathLineEdit->text(), {pathEditWidth, tem_fontmetrics.height()}, QTextOption::WrapAnywhere,
                                          videoPathLineEdit->font(), Qt::ElideMiddle, tem_fontmetrics.height(), pathEditWidth);
    videoPathLineEdit->setText(vdStrElideText);

    option->connect(videoPathLineEdit, &DLineEdit::focusChanged, [ = ](bool on) {
        if (on) {
            QString settingVideoPathValue = option->value().toString();

            if (settingVideoPathValue.front() == '~')
                settingVideoPathValue.replace(0, 1, QDir::homePath());

            QDir dir(settingVideoPathValue);

            if (!dir.exists())
                videoPathLineEdit->setText(option->defaultValue().toString());
            else
                videoPathLineEdit->setText(option->value().toString());
        }
    });

    icon->setIcon(DStyleHelper(icon->style()).standardIcon(DStyle::SP_SelectElement, nullptr));
    icon->setIconSize(QSize(24, 24));
    icon->setFixedSize(41, 37);
    horboxlayout->addWidget(videoPathLineEdit);
    horboxlayout->addWidget(icon);
    optionWidget->setObjectName(OPTION_FRAME);
    optionWidget->setAccessibleName(OPTION_FRAME);
    optionLayout->setContentsMargins(0, 0, 0, 0);
    optionLayout->setSpacing(0);
    optionWidget->setFixedHeight(37);
    main->setMinimumWidth(240);
    DLabel *lab = new DLabel(QObject::tr(option->name().toStdString().c_str()));
    lab->setAlignment(Qt::AlignVCenter);
    optionLayout->addRow(lab, main);
    workaround_updateStyle(optionWidget, "light");
    optinvaliddialog->setObjectName(OPTION_INVALID_DIALOG);
    optinvaliddialog->setAccessibleName(OPTION_INVALID_DIALOG);
    optinvaliddialog->setIcon(QIcon(":/images/icons/warning.svg"));
    optinvaliddialog->setMessage(QObject::tr("You don't have permission to operate this folder"));
    optinvaliddialog->setWindowFlags(optinvaliddialog->windowFlags() | Qt::WindowStaysOnTopHint);
    optinvaliddialog->addButton(QObject::tr("Close"), true, DDialog::ButtonRecommend);

    /**
     *@brief 视频文本框路径是否正确
     *@param name:视频保存路径, alert:提示
     */
    auto validate = [ = ](QString name, bool alert = true) -> bool {
        Q_UNUSED(alert);
        name = name.trimmed();

        if (name.isEmpty())
            return false;

        if (name.size() > 2 && name.contains(relativeHomePath))
            name.replace(0, 1, QDir::homePath());

        QFileInfo fileinfo(name);
        QDir dir(name);

        if (fileinfo.exists())
        {
            QString homeStr("/home/");
            if (!fileinfo.isDir())
                return false;

            if (homeStr.contains(name))
                return false;

            if (!fileinfo.isReadable() || !fileinfo.isWritable())
                return false;

        } else
        {
            if (relativeHomePath.contains(name))
                return false;

            if (dir.cdUp()) {
                QFileInfo ch(dir.path());

                if (!ch.isReadable() || !ch.isWritable())
                    return false;
            }
            return false;
        }
        return true;
    };

    /**
     *@brief 打开视频文件夹按钮槽函数
     */
    option->connect(icon, &DPushButton::clicked, [ = ]() {

        QString selectVideoSavePath;
        QString tmplastvdcpath = lastVideoPath;

        if (tmplastvdcpath.contains(relativeHomePath))
            tmplastvdcpath.replace(0, 1, QDir::homePath());

        QDir dir(tmplastvdcpath);

        if (!dir.exists()) {
            //设置文本框为新的路径
            tmplastvdcpath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
                            + QDir::separator() + "Camera";
            QDir defaultdir(tmplastvdcpath);

            if (!defaultdir.exists()) {
                dir.mkdir(tmplastvdcpath);
            }
            videoPathLineEdit->setText(option->defaultValue().toString());
        }
#ifdef UNITTEST
        selectVideoSavePath = "~/Videos/";
#else
        //打开文件夹
        selectVideoSavePath = DFileDialog::getExistingDirectory(nullptr, QObject::tr("Open folder"),
                                                                tmplastvdcpath,
                                                                DFileDialog::ShowDirsOnly | DFileDialog::DontResolveSymlinks);
        // 视频保存路径暂时不能设置到smb网盘路径(会卡死在fwrite)，临时处理方案：重置回默认路径
        if (selectVideoSavePath.contains("/run/user/1000/gvfs/smb-share:server="))
            selectVideoSavePath = option->defaultValue().toString();
#endif

        if (validate(selectVideoSavePath, false)) {
            option->setValue(selectVideoSavePath);
            QFontMetrics tem_fontmetrics = videoPathLineEdit->fontMetrics();
            QString elideText = ElideText(videoPathLineEdit->text(), {pathEditWidth, tem_fontmetrics.height()}, QTextOption::WrapAnywhere,
                                                videoPathLineEdit->font(), Qt::ElideMiddle, tem_fontmetrics.height(), pathEditWidth);
            videoPathLineEdit->setText(elideText);
        }

        QFileInfo fm(selectVideoSavePath);

        if ((!fm.isReadable() || !fm.isWritable()) && !selectVideoSavePath.isEmpty())
            optinvaliddialog->show();

    });

    /**
     *@brief 视频文本框编辑完成槽函数
     */
    option->connect(videoPathLineEdit, &DLineEdit::editingFinished, option, [ = ]() {

        QString curVideoEditStr = videoPathLineEdit->text();

        // 视频保存路径暂时不能设置到smb网盘路径(会卡死在fwrite)，临时处理方案：重置回默认路径
        if (curVideoEditStr.contains("/run/user/1000/gvfs/smb-share:server="))
            curVideoEditStr = option->defaultValue().toString();

        QDir dir(curVideoEditStr);
        // 获取当前文本路径
        QString curVdElidePath = ElideText(curVideoEditStr, {pathEditWidth, tem_fontmetrics.height()}, QTextOption::WrapAnywhere,
                                                 videoPathLineEdit->font(), Qt::ElideMiddle, tem_fontmetrics.height(), pathEditWidth);

        if (!validate(curVideoEditStr, false)) {
            QFileInfo fn(dir.path());

            if ((!fn.isReadable() || !fn.isWritable()) && !curVideoEditStr.isEmpty()) {
                // 设置为默认路径
                curVideoEditStr = option->defaultValue().toString();
                option->setValue(option->defaultValue().toString());
                curVdElidePath = option->defaultValue().toString();
            }
        }

        if (!videoPathLineEdit->lineEdit()->hasFocus()) {

            //文本框路径有效
            if (validate(videoPathLineEdit->text(), false)) {
                option->setValue(curVideoEditStr);
            }
            //文本框路径无效
            else {
                // 设置为默认路径
                curVideoEditStr = option->defaultValue().toString();
                if (curVideoEditStr.front() == '~')
                    curVideoEditStr.replace(0, 1, QDir::homePath());
                option->setValue(option->defaultValue().toString());
                curVdElidePath = option->defaultValue().toString();
            }

            lastVideoPath = curVideoEditStr;
            videoPathLineEdit->setText(curVdElidePath);
        }
    });

    option->connect(videoPathLineEdit, &DLineEdit::textEdited, option, [ = ](const QString & newStr) {
        validate(newStr);
    });

    /**
     *@brief 视频后台设置参数更改槽函数
     */
    option->connect(option, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged, videoPathLineEdit,
    [ = ](const QVariant & value) {
        auto pi = ElideText(value.toString(), {pathEditWidth, tem_fontmetrics.height()},
                            QTextOption::WrapAnywhere, videoPathLineEdit->font(), Qt::ElideMiddle,
                            tem_fontmetrics.height(), pathEditWidth);
        videoPathLineEdit->setText(pi);
        qDebug() << "picPathLineEdit text:" << videoPathLineEdit->text() << endl;
        lastVideoPath = value.toString();
        option->setValue(value.toString());
        qDebug() << "save video last path:" << value.toString() << endl;
        videoPathLineEdit->update();
    });

    return  optionWidget;
}

QString CMainWindow::lastOpenedPath(QStandardPaths::StandardLocation standard)
{
    QString lastPath;
    if (standard == QStandardPaths::MoviesLocation) {
        lastPath = Settings::get().getOption("base.save.vddatapath").toString();
        if (lastPath.compare(Settings::get().settings()->option("base.save.vddatapath")->defaultValue().toString()) == 0)
            lastPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
                       + QDir::separator() + "Camera";

    } else {
        lastPath = Settings::get().getOption("base.save.picdatapath").toString();
        if (lastPath.compare(Settings::get().settings()->option("base.save.picdatapath")->defaultValue().toString()) == 0)
            lastPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                       + QDir::separator() + "Camera";
    }

    // 包含主目录相对路径，QDir不能识别，必须转为绝对路径
    QString relativeHomePath = QString("~") + QDir::separator();
    if (lastPath.contains(relativeHomePath)) {
        lastPath.replace(0, 1, QDir::homePath());
    }

    QDir lastDir(lastPath);
    if (lastPath.isEmpty() || !lastDir.exists()) {
        if (standard == QStandardPaths::MoviesLocation) {
            lastPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
                       + QDir::separator() + "Camera";
        } else {
            lastPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                       + QDir::separator() + "Camera";
        }

        QDir dirNew(lastPath);
        if (!dirNew.exists()) {
            if (!dirNew.mkdir(lastPath)) {
                qWarning() << "make dir error:" << lastPath;
                lastPath = QDir::currentPath();//这是错误的路径，但肯定存在
                qDebug() << "now lastVdPath is:" << lastPath;
            }
        }
    }

    if (lastPath.size() && lastPath[0] == '~') {
        QString str = QDir::homePath();
        lastPath.replace(0, 1, str);
    }
    return lastPath;
}

void CMainWindow::setWayland(bool bTrue)
{
    m_bWayland = bTrue;
}

CMainWindow::CMainWindow(QWidget *parent)
    : DMainWindow(parent),
      m_photoState(photoNormal),
      m_bRecording(false),
      m_bSwitchCameraShowEnable(false),
      m_bUIinit(false),
      m_windowStateThread(nullptr)
{
    m_cameraSwitchBtn = nullptr;
    m_photoRecordBtn = nullptr;
    m_modeSwitchBox = nullptr;
    m_snapshotLabel = nullptr;
    m_SetDialog = nullptr;
    m_videoPre = nullptr;
    m_pLoginManager = nullptr;
    m_pLoginMgrSleep = nullptr;
    m_pDBus = nullptr;
    m_filterName = nullptr;
    m_bShowFilterName = false;
    m_bWayland = false;
    m_nActTpye = ActTakePic;
    m_SpaceKeyInterval = QDateTime::currentMSecsSinceEpoch();
    this->setObjectName(MAIN_WINDOW);
    this->setAccessibleName(MAIN_WINDOW);

    titlebar()->deleteLater();
    setupTitlebar();
    m_pTitlebar->raise();

    //修复在性能较差的电脑上启动闪烁白框
    initUI();
}

void CMainWindow::slotPopupSettingsDialog()
{
#ifndef UNITTEST
    settingDialog();
    m_SetDialog->exec();
    settingDialogDel();
#endif
}

QString CMainWindow::libPath(const QString &strlib)
{
    QDir  dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib

    if (list.contains(strlib))
        return strlib;

    list.sort();
    if (list.size() > 0)
        return list.last();

    // Qt LibrariesPath 不包含，返回默认名称
    return strlib;
}

void CMainWindow::reflushSnapshotLabel()
{
    QFileInfoList picPathList;
    QFileInfoList videoPathList;
    QString lastFilePath = "";

    getMediaFileInfoList(m_videoPath, videoPathList);
    getMediaFileInfoList(m_picPath, picPathList);
    if (!videoPathList.isEmpty()
            && picPathList.isEmpty()) {
        lastFilePath = videoPathList[0].filePath();
    } else if (videoPathList.isEmpty()
               && ! picPathList.isEmpty()) {
        lastFilePath = picPathList[0].filePath();
    } else if (!videoPathList.isEmpty()
               && !picPathList.isEmpty()) {
        lastFilePath = videoPathList[0].filePath();
        if (videoPathList[0].fileTime(QFileDevice::FileModificationTime) < picPathList[0].fileTime(QFileDevice::FileModificationTime)) {
            lastFilePath = picPathList[0].filePath();
        }
    }
    m_snapshotLabel->updatePicPath(lastFilePath);
}

void CMainWindow::getMediaFileInfoList(const QString &path, QFileInfoList &fileList)
{
    if (path.isEmpty()) {
        return;
    }

    QDir dir(path);
    QStringList filters;
    filters << QString("*.jpg") << QString("*.mp4") << QString("*.webm");
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time /*| QDir::Reversed*/);     //按时间逆序排序
    if (dir.exists()) {
        fileList = dir.entryInfoList();
    }
}

void CMainWindow::initDynamicLibPath()
{
    LoadLibNames tmp;
    QByteArray avcodec = libPath("libavcodec.so").toLatin1();
    tmp.chAvcodec = avcodec.data();
    QByteArray avformat = libPath("libavformat.so").toLatin1();
    tmp.chAvformat = avformat.data();
    QByteArray avutil = libPath("libavutil.so").toLatin1();
    tmp.chAvutil = avutil.data();
    QByteArray udev = libPath("libudev.so").toLatin1();
    tmp.chUdev = udev.data();
    QByteArray usb = libPath("libusb-1.0.so").toLatin1();
    tmp.chUsb = usb.data();
    QByteArray portaudio = libPath("libportaudio.so").toLatin1();
    tmp.chPortaudio = portaudio.data();
    QByteArray v4l2 = libPath("libv4l2.so").toLatin1();
    tmp.chV4l2 = v4l2.data();
    QByteArray ffmpegthumbnailer = libPath("libffmpegthumbnailer.so").toLatin1();
    tmp.chFfmpegthumbnailer = ffmpegthumbnailer.data();
    QByteArray swscale = libPath("libswscale.so").toLatin1();
    tmp.chSwscale = swscale.data();
    QByteArray swresample = libPath("libswresample.so").toLatin1();
    tmp.chSwresample = swresample.data();
    setLibNames(tmp);
}

void CMainWindow::initBlockShutdown()
{
    if (!m_arg.isEmpty() || m_reply.value().isValid()) {
        qInfo() << "m_reply.value().isValid():" << m_reply.value().isValid();
        return;
    }

    m_pLoginManager = new QDBusInterface("org.freedesktop.login1",
                                         "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager",
                                         QDBusConnection::systemBus());

    m_arg << QString("shutdown")             // what
          << qApp->productName()           // who
          << QObject::tr("File not saved")          // why
          << QString("block");                        // mode

    m_reply = m_pLoginManager->callWithArgumentList(QDBus::Block, "Inhibit", m_arg);

    if (m_reply.isValid())
        (void)m_reply.value().fileDescriptor();

    //如果for结束则表示没有发现未保存的tab项，则放开阻塞关机
    if (m_reply.isValid()) {
        QDBusReply<QDBusUnixFileDescriptor> tmp = m_reply;
        m_reply = QDBusReply<QDBusUnixFileDescriptor>();
        qDebug() << "init Nublock shutdown.";
    }
}

void CMainWindow::initBlockSleep()
{
    if (!m_argSleep.isEmpty() || m_replySleep.value().isValid()) {
        qInfo() << "m_reply.value().isValid():" << m_replySleep.value().isValid();
        return;
    }

    m_pLoginMgrSleep = new QDBusInterface("org.freedesktop.login1",
                                          "/org/freedesktop/login1",
                                          "org.freedesktop.login1.Manager",
                                          QDBusConnection::systemBus());

    m_argSleep << QString("sleep")             // what
               << qApp->productName()           // who
               << QObject::tr("File not saved")          // why
               << QString("block");                        // mode

    m_replySleep = m_pLoginMgrSleep->callWithArgumentList(QDBus::Block, "Inhibit", m_argSleep);

    if (m_replySleep.isValid())
        (void)m_replySleep.value().fileDescriptor();

    //如果for结束则表示没有发现未保存的tab项，则放开阻塞睡眠
    if (m_replySleep.isValid()) {
        QDBusReply<QDBusUnixFileDescriptor> tmp = m_replySleep;
        m_replySleep = QDBusReply<QDBusUnixFileDescriptor>();
        qDebug() << "init Nublock sleep.";
    }

}

void CMainWindow::initTabOrder()
{
    m_windowMinBtn = m_pTitlebar->titlebar()->findChild<DWindowMinButton *>("DTitlebarDWindowMinButton");
    m_windowoptionButton = m_pTitlebar->titlebar()->findChild<DWindowOptionButton *>("DTitlebarDWindowOptionButton");
    m_windowMaxBtn = m_pTitlebar->titlebar()->findChild<DWindowMaxButton *>("DTitlebarDWindowMaxButton");
    m_windowCloseBtn = m_pTitlebar->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");

    m_takePhotoSettingArea->setFocusPolicy(Qt::TabFocus);
    m_cameraSwitchBtn->setFocusPolicy(Qt::TabFocus);
    m_photoRecordBtn->setFocusPolicy(Qt::TabFocus);
    m_modeSwitchBox->setFocusPolicy(Qt::TabFocus);
    m_snapshotLabel->setFocusPolicy(Qt::TabFocus);

    setTabOrder(m_windowoptionButton, m_windowMinBtn);
    setTabOrder(m_windowMinBtn, m_windowMaxBtn);
    setTabOrder(m_windowMaxBtn, m_windowCloseBtn);
    setTabOrder(m_windowCloseBtn, m_takePhotoSettingArea);
}

void CMainWindow::initEventFilter()
{
    /**
     * @brief windowMinBtn 最小化按钮
     */
    DWindowMinButton *windowMinBtn = m_pTitlebar->titlebar()->findChild<DWindowMinButton *>("DTitlebarDWindowMinButton");
    if (windowMinBtn)
        windowMinBtn->installEventFilter(this);

    /**
     * @brief windowoptionButton 菜单栏按钮
     */
    DWindowOptionButton *windowoptionButton = m_pTitlebar->titlebar()->findChild<DWindowOptionButton *>("DTitlebarDWindowOptionButton");
    if (windowoptionButton)
        windowoptionButton->installEventFilter(this);

    /**
     * @brief windowMaxBtn  最大化按钮
     */
    DWindowMaxButton *windowMaxBtn = m_pTitlebar->titlebar()->findChild<DWindowMaxButton *>("DTitlebarDWindowMaxButton");
    if (windowMaxBtn)
        windowMaxBtn->installEventFilter(this);

    /**
     * @brief windowCloseBtn 关闭按钮
     */
    DWindowCloseButton *windowCloseBtn = m_pTitlebar->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
    if (windowCloseBtn)
        windowCloseBtn->installEventFilter(this);

    //拍照/录像控件添加过滤
    if (m_photoRecordBtn)
        m_photoRecordBtn->installEventFilter(this);

    if (m_pTitlebar->titlebar())
        m_pTitlebar->titlebar()->installEventFilter(this);

    if (m_videoPre)
        m_videoPre->installEventFilter(this);
}

void CMainWindow::initShortcut()
{
    QShortcutEx *scViewShortcut = new QShortcutEx(QKeySequence("Ctrl+Shift+/"), this);
    QShortcutEx *scSpaceShortcut = new QShortcutEx(Qt::Key_Space, this);
    QShortcutEx *scEnterShortcut = new QShortcutEx(Qt::Key_Return, this);
    QShortcutEx *scUpShortcut = new QShortcutEx(Qt::Key_Up, this);
    QShortcutEx *scDownShortcut = new QShortcutEx(Qt::Key_Down, this);
    QShortcutEx *scLeftShortcut = new QShortcutEx(Qt::Key_Left, this);
    QShortcutEx *scRightShortcut = new QShortcutEx(Qt::Key_Right, this);

    connect(scUpShortcut, &QShortcut::activated, this, &CMainWindow::onKeyUp);
    connect(scDownShortcut, &QShortcut::activated, this, &CMainWindow::onKeyDown);
    connect(scLeftShortcut, &QShortcut::activated, this, &CMainWindow::onKeyLeft);
    connect(scRightShortcut, &QShortcut::activated, this, &CMainWindow::onkeyRight);

    scViewShortcut->setObjectName(SHORTCUT_VIEW);
    scSpaceShortcut->setObjectName(SHORTCUT_SPACE);

    connect(scViewShortcut, &QShortcut::activated, this, [ = ] {
        qDebug() << "receive Ctrl+Shift+/";
        QRect rect = window()->geometry();
        QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
        Shortcut sc;
        QStringList shortcutString;
        QString param1 = "-j=" + sc.toStr();
        QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());

        shortcutString << param1 << param2;
#ifndef UNITTEST
        QProcess::startDetached("deepin-shortcut-viewer", shortcutString);
#endif
    });

    connect(scSpaceShortcut, &QShortcut::activated, this, [ = ] {
        if (QDateTime::currentMSecsSinceEpoch() < m_SpaceKeyInterval)
            //获取录制按钮第一次点击时间(ms)
            m_SpaceKeyInterval = QDateTime::currentMSecsSinceEpoch();

        //第二次点击时间与第一次时间差
        if (QDateTime::currentMSecsSinceEpoch() - m_SpaceKeyInterval > 300)
        {
            m_SpaceKeyInterval = QDateTime::currentMSecsSinceEpoch();
            onPhotoRecordBtnClked();
            m_takePhotoSettingArea->closeAllGroup();
        }
    });

    //根据tab焦点进行Enter键操作
    connect(scEnterShortcut, &QShortcut::activated, this, [ = ] {
        DWindowMinButton *windowMinBtn = m_pTitlebar->titlebar()->findChild<DWindowMinButton *>("DTitlebarDWindowMinButton");
        DWindowOptionButton *windowoptionButton = m_pTitlebar->titlebar()->findChild<DWindowOptionButton *>("DTitlebarDWindowOptionButton");
        DWindowMaxButton *windowMaxBtn = m_pTitlebar->titlebar()->findChild<DWindowMaxButton *>("DTitlebarDWindowMaxButton");
        DWindowCloseButton *windowCloseBtn = m_pTitlebar->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
        QWidget *focuswidget = focusWidget();


        /**
         *@brief 判断当前的焦点窗口在哪个控件上，通过enter键触发对应的点击操作
         */
        if (windowoptionButton == focuswidget)
            windowoptionButton->click();
        else if (windowMinBtn == focuswidget)
            windowMinBtn->click();
        else if (windowMaxBtn == focuswidget)
            windowMaxBtn->click();
        else if (windowCloseBtn == focuswidget)
            windowCloseBtn->click();
        else if (m_photoRecordBtn == focuswidget)
            emit m_photoRecordBtn->clicked();
        else if (m_snapshotLabel == focuswidget)
            m_snapshotLabel->openFile();
        else if (m_cameraSwitchBtn == focuswidget)
            //m_videoPre->onChangeDev();
            m_cameraSwitchBtn->clicked();
        else
            return;
    });

}

void CMainWindow::settingDialog()
{
    m_SetDialog = new DSettingsDialog(this);
    m_SetDialog->setFixedSize(820, 600);
    m_SetDialog->widgetFactory()->registerWidget("selectableEditpic", createPicSelectableLineEditOptionHandle);
    m_SetDialog->widgetFactory()->registerWidget("selectableEditvd", createVdSelectableLineEditOptionHandle);
    m_SetDialog->widgetFactory()->registerWidget("formatLabel", createFormatLabelOptionHandle);
    m_SetDialog->setObjectName(SETTING_DIALOG);
    m_SetDialog->setAccessibleName(SETTING_DIALOG);

    connect(m_SetDialog, SIGNAL(destroyed()), this, SLOT(onSettingsDlgClose()));

    auto resolutionmodeFamily = Settings::get().settings()->option("outsetting.resolutionsetting.resolution");



    if (get_v4l2_device_handler() != nullptr) {
        //格式索引
        int format_index = v4l2core_get_frame_format_index(
                               get_v4l2_device_handler(),
                               v4l2core_get_requested_frame_format(get_v4l2_device_handler()));
        //分辨率索引
        int resolu_index = v4l2core_get_format_resolution_index(
                               get_v4l2_device_handler(),
                               format_index,
                               v4l2core_get_frame_width(get_v4l2_device_handler()),
                               v4l2core_get_frame_height(get_v4l2_device_handler()));

        //获取当前摄像头的格式表
        v4l2_stream_formats_t *list_stream_formats = v4l2core_get_formats_list(get_v4l2_device_handler());
        //初始化分辨率字符串表
        QStringList resolutionDatabase;
        resolutionDatabase.clear();

        //当前分辨率下标
        if (format_index >= 0 && resolu_index >= 0) {

            //format_index = -1 &&resolu_index = -1 表示设备被占用或者不存在
            for (int i = 0 ; i < list_stream_formats[format_index].numb_res; i++) {

                if ((list_stream_formats[format_index].list_stream_cap[i].width > 0
                        && list_stream_formats[format_index].list_stream_cap[i].height > 0) &&
                        (list_stream_formats[format_index].list_stream_cap[i].width < 7680
                         && list_stream_formats[format_index].list_stream_cap[i].height < 4320) &&
                        ((list_stream_formats[format_index].list_stream_cap[i].width % 8) == 0
                         && (list_stream_formats[format_index].list_stream_cap[i].width % 16) == 0
                         && (list_stream_formats[format_index].list_stream_cap[i].height % 8) ==  0)) {
                    //加入分辨率的字符串
                    QString res_str = QString("%1x%2").arg(list_stream_formats[format_index].list_stream_cap[i].width).arg(list_stream_formats[format_index].list_stream_cap[i].height);

                    //去重
                    if (resolutionDatabase.contains(res_str) == false)
                        resolutionDatabase.append(res_str);
                }
            }
            int defres = 0;
            int tempostion = 0;
            int len = resolutionDatabase.size() - 1;

            for (int i = 0; i < resolutionDatabase.size() - 1; i++) {
                int flag = 1;

                for (int j = 0 ; j < len; j++) {
                    QStringList resolutiontemp1 = resolutionDatabase[j].split("x");
                    QStringList resolutiontemp2 = resolutionDatabase[j + 1].split("x");

                    if ((resolutiontemp1[0].toInt() < resolutiontemp2[0].toInt()) ||
                            (resolutiontemp1[0].toInt() == resolutiontemp2[0].toInt() && (resolutiontemp1[1].toInt() < resolutiontemp2[1].toInt()))) {
                        QString resolutionstr = resolutionDatabase[j + 1];
                        resolutionDatabase[j + 1] = resolutionDatabase[j];
                        resolutionDatabase[j] = resolutionstr;
                        flag = 0;
                        tempostion = j;
                    }
                }
                len = tempostion;

                if (flag == 1)
                    continue;

            }

            for (int i = 0; i < resolutionDatabase.size(); i++) {
                QStringList resolutiontemp = resolutionDatabase[i].split("x");

                if ((v4l2core_get_frame_width(get_v4l2_device_handler()) == resolutiontemp[0].toInt()) &&
                        (v4l2core_get_frame_height(get_v4l2_device_handler()) == resolutiontemp[1].toInt())) {
                    defres = i; //设置分辨率下拉菜单当前索引
                    break;
                }

            }

            resolutionmodeFamily->setData("items", resolutionDatabase);
            //设置当前分辨率的索引
            resolutionDatabase.append(QString(tr("None")));
            Settings::get().settings()->setOption(QString("outsetting.resolutionsetting.resolution"), defres);
        } else {
            resolutionDatabase.clear();
            resolutionmodeFamily->setData("items", resolutionDatabase);
        }
    } else {
        //初始化分辨率字符串表
        int defres = 0;
        QStringList resolutionDatabase = resolutionmodeFamily->data("items").toStringList();
        if (DataManager::instance()->encodeEnv() == QCamera_Env && !Camera::instance()->getSupportResolutions().isEmpty()) {
            resolutionDatabase = Camera::instance()->getSupportResolutions();

            for (int i = 0; i < resolutionDatabase.size(); i++) {
                QStringList resolutiontemp = resolutionDatabase[i].split("x");

                if ((Camera::instance()->getCameraResolution().width() == resolutiontemp[0].toInt()) &&
                        Camera::instance()->getCameraResolution().height() == resolutiontemp[1].toInt()) {
                    defres = i; //设置分辨率下拉菜单当前索引
                    break;
                }
            }
        } else {
            if (resolutionDatabase.size() > 0)
                resolutionmodeFamily->data("items").clear();

            resolutionDatabase.clear();
            resolutionDatabase.append(QString(tr("None")));
        }
        Settings::get().settings()->setOption(QString("outsetting.resolutionsetting.resolution"), defres);
        resolutionmodeFamily->setData("items", resolutionDatabase);
    }
    m_SetDialog->setProperty("_d_QSSThemename", "dark");
    m_SetDialog->setProperty("_d_QSSFilename", "DSettingsDialog");
    m_SetDialog->updateSettings(Settings::get().settings());

    QPushButton *resetbtn = m_SetDialog->findChild<QPushButton *>("SettingsContentReset");
    resetbtn->setDefault(false);
    resetbtn->setAutoDefault(false);
}

void CMainWindow::settingDialogDel()
{
    if (m_SetDialog) {
        delete m_SetDialog;
        m_SetDialog = nullptr;
    }
}

void CMainWindow::loadAfterShow()
{
    encodeenv = DataManager::instance()->encodeEnv();
    initDynamicLibPath();
    initShortcut();
    gviewencoder_init();
    v4l2core_init();

    m_devnumMonitor = new DevNumMonitor();
    m_devnumMonitor->setParent(this);
    m_devnumMonitor->setObjectName("DevMonitorThread");
    m_devnumMonitor->startCheck();
    initTitleBar();
    initConnection();
    //后续修改为标准Qt用法
    QString strCache = QString(getenv("HOME")) + QDir::separator() + QString(".cache")
                       + QDir::separator() + QString("deepin") + QDir::separator()
                       + QString("deepin-camera") + QDir::separator();
    QDir dir;
    dir.mkpath(strCache);
    initRightButtons();
    initTabOrder();
    initEventFilter();
    reflushSnapshotLabel();

    connect(m_devnumMonitor, SIGNAL(seltBtnStateEnable()), this, SLOT(setSelBtnShow()));//显示切换按钮
    connect(m_devnumMonitor, SIGNAL(seltBtnStateDisable()), this, SLOT(setSelBtnHide()));//多设备信号
    if(DataManager::instance()->encodeEnv() != QCamera_Env) {
        connect(m_devnumMonitor, SIGNAL(existDevice()), m_videoPre, SLOT(onRestartDevices()));//重启设备
        connect(m_devnumMonitor, SIGNAL(noDeviceFound()), m_videoPre, SLOT(onRestartDevices()));//重启设备
    } else if (DataManager::instance()->encodeEnv() == QCamera_Env) {
        initCameraConnection();
    }

    m_videoPre->delayInit();
    m_videoPre->setFilterType(filter_Normal);
    m_takePhotoSettingArea->setFilterType(filter_Normal);
    m_filterName->setText(filterPreviewButton::filterName(filter_Normal));

    showChildWidget();
    m_windowStateThread = new windowStateThread(m_bWayland, this);
    connect(m_windowStateThread, &windowStateThread::someWindowFullScreen, this, &CMainWindow::onStopPhotoAndRecord);

    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"mode", 1},
        {"version", VERSION},
        {"camera_connected", DataManager::instance()->getdevStatus() ? true : false}
    };
    EventLogUtils::get().writeLogs(obj);
}

void CMainWindow::updateBlockSystem(bool bTrue)
{
    if (!CamApp->isPanelEnvironment()) {
        initBlockShutdown();

        if (bTrue) {
            m_reply = m_pLoginManager->callWithArgumentList(QDBus::Block, "Inhibit", m_arg);
        } else {
            QDBusReply<QDBusUnixFileDescriptor> tmp = m_reply;
            m_reply = QDBusReply<QDBusUnixFileDescriptor>();
            qDebug() << "Nublock shutdown.";
        }
    }
    if (m_bWayland) {
        initBlockSleep();

        if (bTrue) {
            m_replySleep = m_pLoginMgrSleep->callWithArgumentList(QDBus::Block, "Inhibit", m_argSleep);
        } else {
            QDBusReply<QDBusUnixFileDescriptor> tmp = m_replySleep;
            m_replySleep = QDBusReply<QDBusUnixFileDescriptor>();
            qDebug() << "Nublock sleep.";
        }
    }
}

void CMainWindow::onNoCam()
{
    m_pTitlebar->titlebar()->setBackgroundTransparent(false);
    m_pTitlebar->slotThemeTypeChanged();
    showChildWidget();
    onEnableSettings(true);
}

void CMainWindow::stopCancelContinuousRecording(bool bTrue)
{
    if (bTrue) {
        onStopPhotoAndRecord();
    }
}

void CMainWindow::onDirectoryChanged(const QString &filePath)
{
    Q_UNUSED(filePath);
    // 转为绝对路径进行比较
    QString relativeHomePath = QString("~") + QDir::separator();
    QString videoPath = Settings::get().getOption("base.save.vddatapath").toString();
    QString picPath = Settings::get().getOption("base.save.picdatapath").toString();
    if (videoPath.contains(relativeHomePath))
            videoPath.replace(0, 1, QDir::homePath());
    if (picPath.contains(relativeHomePath))
        picPath.replace(0, 1, QDir::homePath());
    QDir  dir(videoPath);
    if (!dir.exists()) {
        QString videoDefaultPath = lastOpenedPath(QStandardPaths::MoviesLocation);
        if (videoDefaultPath != videoPath) {
            QDir setVideoDir(videoDefaultPath);
            QDir lastVideoDir(m_videoPath);
            setVideoDir.cdUp();
            lastVideoDir.cdUp();
            m_videoPre->setSaveVdFolder(videoDefaultPath);
            m_fileWatcher.removePath(m_videoPath);
            if (false == m_fileWatcher.directories().contains(videoDefaultPath)) {
                m_fileWatcher.addPath(videoDefaultPath);
            }

            if (setVideoDir.path() != lastVideoDir.path()) {
                m_fileWatcherUp.removePath(lastVideoDir.path());
                if (false == m_fileWatcherUp.directories().contains(setVideoDir.path())) {
                    m_fileWatcherUp.addPath(setVideoDir.path());
                }
            }
            m_videoPath = videoDefaultPath;
            Settings::get().setOption("base.save.vddatapath", QVariant(m_videoPath));
        } else {
            m_fileWatcher.removePath(videoPath);
            dir.mkdir(videoPath);
            m_fileWatcher.addPath(videoPath);
        }
    }

    dir = QDir(picPath);
    if (!dir.exists()) {
        QString picDefaultPath = lastOpenedPath(QStandardPaths::PicturesLocation);
        if (picDefaultPath != picPath) {
            QDir setPicDir(picDefaultPath);
            QDir lastPicDir(m_picPath);
            setPicDir.cdUp();
            lastPicDir.cdUp();
            m_videoPre->setSavePicFolder(picDefaultPath);
            m_fileWatcher.removePath(m_picPath);
            if (false == m_fileWatcher.directories().contains(picDefaultPath)) {
                m_fileWatcher.addPath(picDefaultPath);
            }

            if (setPicDir.path() != lastPicDir.path()) {
                m_fileWatcherUp.removePath(lastPicDir.path());
                if (false == m_fileWatcherUp.directories().contains(setPicDir.path())) {
                    m_fileWatcherUp.addPath(setPicDir.path());
                }
            }
            m_picPath = picDefaultPath;
            Settings::get().setOption("base.save.picdatapath", QVariant(m_picPath));
        } else {
            m_fileWatcher.removePath(picPath);
            dir.mkdir(picPath);
            m_fileWatcher.addPath(picPath);
        }
    }

    reflushSnapshotLabel();
}

void CMainWindow::onSwitchCameraSuccess(const QString &cameraName)
{
    QStringList tmpList = cameraName.split(":");
    if (!tmpList.isEmpty()) {
        m_labelCameraName->setText(tmpList[0]);
    }
    int width = m_labelCameraName->fontMetrics().width(tmpList[0]);
    m_labelCameraName->setFixedWidth(width);
    m_labelCameraName->show();
    m_filterName->hide();
    m_showCameraNameTimer->start();
}

void CMainWindow::onTimeoutLock(const QString &serviceName, QVariantMap key2value, QStringList)
{
    qDebug() << serviceName << key2value << endl;
    //仅wayland需要锁屏结束录制并停止使用摄像头，从锁屏恢复重新开启摄像头
    //wayland下只需要停止录像，不需要停止线程，需要在锁屏状态下继续处理摄像头状态
//    if (m_bWayland) {

//        if (key2value.value("Locked").value<bool>()) {
//            qDebug() << "locked";

//            onStopPhotoAndRecord();

//            m_videoPre->m_imgPrcThread->stop();

//            v4l2_dev_t *vd =  get_v4l2_device_handler();
//            qDebug() << "lock end";
//        } else {
//            qDebug() << "restart use camera cause ScreenBlack or PoweerLock";
//            //打开摄像头
//            m_videoPre->onChangeDev();
//            qDebug() << "v4l2core_start_stream OK";
//        }

//    } else { //锁屏结束连拍
        if (key2value.value("Locked").value<bool>()) {
            onStopPhotoAndRecord();
        }
//    }
}

void CMainWindow::onTrashFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
        return;
    }
    file.remove();
    //删除文件后，filewatcher会自动刷新缩略图
}

void CMainWindow::onSwitchPhotoBtnClked()
{
    m_photoRecordBtn->setState(true);
    m_photoRecordBtn->setToolTip(tr("Photo"));
    m_takePhotoSettingArea->setState(true);
    m_videoPre->setState(true);
    locateRightButtons();
}

void CMainWindow::onSwitchRecordBtnClked()
{
    m_photoRecordBtn->setState(false);
    m_photoRecordBtn->setToolTip(tr("Video"));
    m_takePhotoSettingArea->setState(false);
    m_videoPre->setState(false);
    locateRightButtons();
}

void CMainWindow::onModeSwitchBoxValueChanged(int type)
{
    if (type == ActType::ActTakePic) {
        onSwitchPhotoBtnClked();
    } else if (type == ActType::ActTakeVideo) {
        onSwitchRecordBtnClked();
    } else {
        return;
    }
}

void CMainWindow::onPhotoRecordBtnClked()
{
    QJsonObject obj {
        {"tid", m_photoRecordBtn->photoState() ? EventLogUtils::StartPhoto : EventLogUtils::StartRecording},
        {"version", VERSION},
        {"camera_connected", DataManager::instance()->getdevStatus() ? true : false}
    };
    if(!m_bRecording)
        EventLogUtils::get().writeLogs(obj);

    //没有摄像机，不进行任何操作
    if (NOCAM == DataManager::instance()->getdevStatus()) {
        return;
    }
    //拍照模式下
    if (true == m_photoRecordBtn->photoState()) {
        //正在拍照
        if (photoNormal != m_photoState) {
            m_videoPre->onTakePic(false);
        } else {
            int nContinuous = Settings::get().getOption("photosetting.photosnumber.takephotos").toInt();
            if (nContinuous > 1) {
                // 仅在连拍模式下才开启窗口状态监听线程
                if (!m_windowStateThread->isRunning() && !m_bWayland) {
                    m_windowStateThread->start();
                }
            }
            m_videoPre->onTakePic(true);
        }
    } else { //录像模式下
        if (true == m_bRecording) {
            m_videoPre->onEndBtnClicked();
        } else {
            QFileInfo fi(m_videoPath);
            if (!fi.isWritable())
                return;

            m_videoPre->onTakeVideo();
            if (!m_windowStateThread->isRunning() && !m_bWayland) {
                m_windowStateThread->start();
            }
        }
    }
}

void CMainWindow::onShowCameraNameTimer()
{
    m_showCameraNameTimer->stop();
    m_labelCameraName->hide();
    if (m_bShowFilterName)
        m_filterName->show();
}

void CMainWindow::onUpdateRecordState(int state)
{
    m_photoRecordBtn->setRecordState(state);
    m_bRecording = (photoRecordBtn::Normal != state);
    m_actionSettings->setEnabled(!m_bRecording);
    showChildWidget();
    if (false == m_bRecording
            && m_windowStateThread->isRunning() && !m_bWayland) {
        m_windowStateThread->requestInterruption();
    }
}

void CMainWindow::onUpdatePhotoState(int state)
{
    m_actionSettings->setEnabled(photoNormal == state);
    m_photoState = state;
    showChildWidget();
    if (photoNormal == state
            && m_windowStateThread->isRunning() && !m_bWayland) {
        m_windowStateThread->requestInterruption();
    }
}

void CMainWindow::onStopPhotoAndRecord()
{
    if (photoNormal != m_photoState) {
        m_videoPre->onTakePic(false);
    }
    if (m_bRecording) {
        m_videoPre->onEndBtnClicked();
    }
}

void CMainWindow::onMirrorStateChanged(bool bMirror)
{
    if (m_videoPre) {
        m_videoPre->setHorizontalMirror(bMirror);
    }
}

void CMainWindow::onSetFlash(bool bFlashOn)
{
    if (m_videoPre) {
        m_videoPre->setFlash(bFlashOn);
    }
    if (bFlashOn != dc::Settings::get().getOption("photosetting.Flashlight.Flashlight").toBool()) {
        dc::Settings::get().setOption("photosetting.Flashlight.Flashlight", bFlashOn);
    }
    if (bFlashOn != m_takePhotoSettingArea->flashLight()) {
        m_takePhotoSettingArea->setFlashlight(bFlashOn);
    }
}

void CMainWindow::onKeyUp()
{
    QWidget *pWidget = focusWidget();
    if (m_cameraSwitchBtn == pWidget
            || m_takePhotoSettingArea == pWidget) {
        m_windowoptionButton->setFocus();
    } else if (m_photoRecordBtn == pWidget) {
        if (m_cameraSwitchBtn->isHidden()) {
            m_windowoptionButton->setFocus();
        } else {
            m_cameraSwitchBtn->setFocus();
        }
    }
}

void CMainWindow::onKeyDown()
{
    QWidget *pWidget = focusWidget();
    if (m_windowMinBtn == pWidget
            || m_windowoptionButton == pWidget
            || m_windowMaxBtn == pWidget
            || m_windowCloseBtn == pWidget) {
        if (m_cameraSwitchBtn->isHidden()) {
            m_photoRecordBtn->setFocus();
        } else {
            m_cameraSwitchBtn->setFocus();
        }
    } else if (m_cameraSwitchBtn == pWidget) {
        m_photoRecordBtn->setFocus();
    } else if (m_photoRecordBtn == pWidget) {
        m_snapshotLabel->setFocus();
    }
}

void CMainWindow::onKeyLeft()
{
    QWidget *pWidget = focusWidget();
    if (m_cameraSwitchBtn == pWidget
            || m_photoRecordBtn == pWidget
            || m_snapshotLabel == pWidget) {
        m_takePhotoSettingArea->setFocus();
    } else if (m_windowCloseBtn == pWidget) {
        m_windowMaxBtn->setFocus();
    } else if (m_windowMaxBtn == pWidget) {
        m_windowMinBtn->setFocus();
    } else if (m_windowMinBtn == pWidget) {
        m_windowoptionButton->setFocus();
    }
}

void CMainWindow::onkeyRight()
{
    QWidget *pWidget = focusWidget();
    if (m_windowoptionButton == pWidget) {
        m_windowMinBtn->setFocus();
    } else if (m_windowMinBtn == pWidget) {
        m_windowMaxBtn->setFocus();
    } else if (m_windowMaxBtn == pWidget) {
        m_windowCloseBtn->setFocus();
    } else if (m_takePhotoSettingArea == pWidget) {
        if (m_cameraSwitchBtn->isHidden()) {
            m_photoRecordBtn->setFocus();
        } else {
            m_cameraSwitchBtn->setFocus();
        }
    }
}

void CMainWindow::onFilterChanged(efilterType type)
{
    if (m_videoPre) {
        m_videoPre->setFilterType(type);
    }
}

void CMainWindow::onShowFilterName(bool bShow)
{
    m_filterName->setVisible(bShow);
    m_bShowFilterName = bShow;
}

void CMainWindow::onSetFilterName(const QString& name)
{
    m_filterName->setText(name);
}

void CMainWindow::initUI()
{
    m_videoPre = new videowidget(this);
    QPalette paletteTime = m_videoPre->palette();
    m_videoPre->setObjectName(VIDEO_PREVIEW_WIDGET);
    m_videoPre->setAccessibleName(VIDEO_PREVIEW_WIDGET);
    setCentralWidget(m_videoPre);
    paletteTime.setBrush(QPalette::Dark, QColor(0, 0, 0, 51)); //深色
    m_videoPre->setPalette(paletteTime);

    m_filterName = new QLabel(this);
    m_filterName->hide();
    m_filterName->setFixedSize(labelFilterNameWidth, labelFilterNameHeight);
    QPalette paletteFilterName = m_filterName->palette();
    paletteFilterName.setColor(QPalette::Background, QColor(0, 0, 0, 0));
    paletteFilterName.setColor(QPalette::WindowText, QColor(255, 255, 255, 255));
    m_filterName->setAutoFillBackground(true);
    m_filterName->setPalette(paletteFilterName);
    QFont ftFilter("SourceHanSansSC");
    ftFilter.setPixelSize(18);
    ftFilter.setWeight(QFont::Medium);
    m_filterName->setFont(ftFilter);
    m_filterName->setAlignment(Qt::AlignCenter);
    m_filterName->move((defaultWindowWidth - labelFilterNameWidth) / 2, defaultWindowHeight - 20 - labelFilterNameHeight);

    QGraphicsDropShadowEffect *shadow_effect1 = new QGraphicsDropShadowEffect(this);
    shadow_effect1->setOffset(0, 0);              //阴影的偏移量
    shadow_effect1->setColor(QColor(33, 33, 33)); //阴影的颜色
    shadow_effect1->setBlurRadius(15);             // 阴影圆角的大小
    m_filterName->setGraphicsEffect(shadow_effect1);

    m_takePhotoSettingArea = new takePhotoSettingAreaWidget(this);

    connect(m_takePhotoSettingArea, &takePhotoSettingAreaWidget::sngSetDelayTakePhoto, this, [ = ](int delaytime) {
        m_videoPre->setInterval(delaytime);

        int setIndex = 0;
        if (3 == delaytime)
            setIndex = 1;
        else if (6 == delaytime)
            setIndex = 2;

        auto curIndex = dc::Settings::get().settings()->getOption(QString("photosetting.photosdelay.photodelays"));

        if (curIndex.toInt() != setIndex)//防止循环设置
            dc::Settings::get().settings()->setOption(QString("photosetting.photosdelay.photodelays"), setIndex);
    });
    connect(m_takePhotoSettingArea, &takePhotoSettingAreaWidget::sngSetFlashlight, this, &CMainWindow::onSetFlash);
    connect(m_takePhotoSettingArea, &takePhotoSettingAreaWidget::sngFilterChanged, this, &CMainWindow::onFilterChanged);
    connect(m_takePhotoSettingArea, &takePhotoSettingAreaWidget::sngShowFilterName, this, &CMainWindow::onShowFilterName);
    connect(m_takePhotoSettingArea, &takePhotoSettingAreaWidget::sigFilterGroupDisplay, m_videoPre, &videowidget::onFilterDisplayChanged);
    connect(m_takePhotoSettingArea, &takePhotoSettingAreaWidget::sngSetFilterName, this, &CMainWindow::onSetFilterName);
    connect(m_takePhotoSettingArea, &takePhotoSettingAreaWidget::sigExposureChanged, m_videoPre, &videowidget::onExposureChanged);
    connect(&dc::Settings::get(), SIGNAL(flashLightChanged(bool)), this, SLOT(onSetFlash(bool)));
    //切换镜像
    connect(&Settings::get(), SIGNAL(mirrorModeChanged(bool)), this, SLOT(onMirrorStateChanged(bool)));
    connect(&Settings::get(), &Settings::delayTimeChanged, this, [ = ](const QString & str) {

        int delayTime = 0;
        if (str == "3s") {
            delayTime = 3;
        } else if (str == "6s") {
            delayTime = 6;
        }

        m_takePhotoSettingArea->setDelayTime(delayTime);
    });

    m_takePhotoSettingArea->init();
    m_takePhotoSettingArea->moveToParentLeft();
    m_takePhotoSettingArea->setVisible(true);

    m_videoPath = lastOpenedPath(QStandardPaths::MoviesLocation);//如果路径不存在会自动使用并创建默认路径
    m_picPath = lastOpenedPath(QStandardPaths::PicturesLocation);

    m_fileWatcher.addPath(m_videoPath);
    m_fileWatcher.addPath(m_picPath);

    m_labelCameraName = new DLabel(this);
    m_showCameraNameTimer = new QTimer(this);
    m_labelCameraName->hide();
    m_showCameraNameTimer->setInterval(2000);
    m_labelCameraName->setFixedSize(labelCameraNameWidth, labelCameraNameHeight);
    QPalette paletteName = m_labelCameraName->palette();
    paletteName.setColor(QPalette::Background, QColor(0, 0, 0, 0)); //深色
    paletteName.setColor(QPalette::WindowText, QColor(255, 255, 255, 255));
    m_labelCameraName->setAutoFillBackground(true);
    m_labelCameraName->setPalette(paletteName);
    QFont ft;
    ft.setFamily("SourceHanSansSC, SourceHanSansSC-Normal");
    ft.setWeight(20);
    ft.setPointSize(14);
    m_labelCameraName->setFont(ft);
    m_labelCameraName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_labelCameraName->move((width() - labelCameraNameWidth) / 2, height() - 20 - labelCameraNameHeight);

    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);              //阴影的偏移量
    shadow_effect->setColor(QColor(33, 33, 33)); //阴影的颜色
    shadow_effect->setBlurRadius(15);             // 阴影圆角的大小
    m_labelCameraName->setGraphicsEffect(shadow_effect);

    QDir dirVd(m_videoPath);
    dirVd.cdUp();
    m_fileWatcherUp.addPath(dirVd.path());

    QDir dirPic(m_picPath);
    dirPic.cdUp();
    if (!m_fileWatcherUp.directories().contains(dirPic.path())) {
        m_fileWatcherUp.addPath(dirPic.path());
    }

    m_videoPre->setSaveVdFolder(m_videoPath);
    m_videoPre->setSavePicFolder(m_picPath);

    GridType gridType = static_cast<GridType>(Settings::get().getOption("base.photogrid.photogrids").toInt());
    int nContinuous = Settings::get().getOption("photosetting.photosnumber.takephotos").toInt();
    int nDelayTime = Settings::get().getOption("photosetting.photosdelay.photodelays").toInt();
    bool soundphoto = Settings::get().getOption("photosetting.audiosetting.soundswitchbtn").toBool();

    switch (nContinuous) {//连拍次数
    case 1:
        nContinuous = 4;
        break;
    case 2:
        nContinuous = 10;
        break;
    default:
        nContinuous = 1;
        break;
    }

    switch (nDelayTime) {//延迟时间
    case 1:
        nDelayTime = 3;
        break;
    case 2:
        nDelayTime = 6;
        break;
    default:
        nDelayTime = 0;
        break;
    }

    if (soundphoto)
        set_takeing_photo_sound(1);
    else
        set_takeing_photo_sound(0);

    m_videoPre->setGridType(gridType);
    m_videoPre->setInterval(nDelayTime);
    m_videoPre->setContinuous(nContinuous);
    m_videoPre->setMaxRecTime(dc::Settings::get().getOption("photosetting.maxRecordTime.maxRecordTime").toInt());
    resize(defaultWindowWidth, defaultWindowHeight);

    m_takePhotoSettingArea->setDelayTime(nDelayTime);
    m_takePhotoSettingArea->setFlashlight(dc::Settings::get().getOption("photosetting.Flashlight.Flashlight").toBool());

    m_bUIinit = true;
}

void CMainWindow::initTitleBar()
{
    m_pTitlebar->titlebar()->setIcon(QIcon::fromTheme("deepin-camera"));
    m_pTitlebar->raise();
    m_pTitlebar->titlebar()->raise();
}

void CMainWindow::initConnection()
{
    connect(CamApp, &CApplication::popupConfirmDialog, this, [ = ] {
        if (m_videoPre->getCapStatus())
        {
            CloseDialog closeDlg(this, tr("Video recording is in progress. Close the window?"));
#ifdef UNITTEST
            closeDlg.show();
            sleep(1);
            closeDlg.close();
#else
            int ret = closeDlg.exec();

            if (ret == 1) {
                m_videoPre->onEndBtnClicked();
                usleep(200);
                qApp->quit();
            }
#endif
        } else
            qApp->quit();
    });

    //设置按钮信号
    connect(m_actionSettings, &QAction::triggered, this, &CMainWindow::slotPopupSettingsDialog);
    //切换分辨率
    connect(&Settings::get(), SIGNAL(resolutionchanged(const QString &)), m_videoPre, SLOT(slotresolutionchanged(const QString &)));
    //切换网格类型
    connect(&Settings::get(), SIGNAL(gridTypeChanged(int)), m_videoPre, SLOT(slotGridTypeChanged(int)));
    //切换录制视频格式
    connect(&Settings::get(), SIGNAL(videoFormatChanged(const QString &)), m_videoPre, SLOT(slotVideoFormatChanged(const QString &)));
    //拍照
    connect(m_videoPre, SIGNAL(takePicOnce()), this, SLOT(onTakePicOnce()));
    //拍照取消
    connect(m_videoPre, SIGNAL(takePicCancel()), this, SLOT(onTakePicCancel()));
    //拍照结束
    connect(m_videoPre, SIGNAL(takePicDone()), this, SLOT(onTakePicDone()));
    //录制结束
    connect(m_videoPre, SIGNAL(takeVdDone()), this, SLOT(onTakeVdDone()));
    //录制取消 （倒计时3秒内的）
    connect(m_videoPre, SIGNAL(takeVdCancel()), this, SLOT(onTakeVdCancel()));
    //录制关机/休眠阻塞
    connect(m_videoPre, SIGNAL(updateBlockSystem(bool)), this, SLOT(updateBlockSystem(bool)));
    //没有相机了，结束拍照、录制
    connect(m_videoPre, SIGNAL(noCam()), this, SLOT(onNoCam()));
    //相机被抢占了，结束拍照、录制
    connect(m_videoPre, SIGNAL(noCamAvailable()), this, SLOT(onNoCam()));
    connect(m_videoPre, &videowidget::camAvailable, this, [=](){
        m_pTitlebar->titlebar()->setBackgroundTransparent(true);
    });
    //设置新的分辨率
    connect(m_videoPre, SIGNAL(sigDeviceChange()), &Settings::get(), SLOT(setNewResolutionList()));

    connect(m_videoPre, SIGNAL(updateRecordState(int)), this, SLOT(onUpdateRecordState(int)));

    connect(m_videoPre, SIGNAL(updatePhotoState(int)), this, SLOT(onUpdatePhotoState(int)));

    // 更新滤镜预览帧图片
    connect(m_videoPre, SIGNAL(updateFilterImage(QImage*)), m_takePhotoSettingArea, SLOT(onUpdateFilterImage(QImage*)));

    connect(m_videoPre, &videowidget::reflushSnapshotLabel, this, &CMainWindow::reflushSnapshotLabel);

    connect(m_showCameraNameTimer, SIGNAL(timeout()), this, SLOT(onShowCameraNameTimer()));

    //连拍在锁屏、熄屏情况下都要结束，与平台无关
    QDBusConnection::sessionBus().connect("com.deepin.SessionManager", "/com/deepin/SessionManager",
                                          "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                                          SLOT(onTimeoutLock(QString, QVariantMap, QStringList)));


    //控制中心更改了本地时间，及时更新当前时间
    //避免时间往前切换过后，需要点击两次才能结束录制
#ifdef OS_BUILD_V23
    QDBusConnection::sessionBus().connect("org.deepin.daemon.Timedate1", "/org/deepin/daemon/Timedate1",
                                          "org.deepin.daemon.Timedate1", "TimeUpdate", this,
                                          SLOT(onLocalTimeChanged()));
#else
    QDBusConnection::sessionBus().connect("com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate",
                                          "com.deepin.daemon.Timedate", "TimeUpdate", this,
                                          SLOT(onLocalTimeChanged()));
#endif

    QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager", "PrepareForSleep", this, SLOT(stopCancelContinuousRecording(bool)));

    QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1",
                                         "org.freedesktop.login1.Manager", "PrepareForShutdown", this, SLOT(stopCancelContinuousRecording(bool)));
}

void CMainWindow::initCameraConnection()
{
    connect(m_devnumMonitor, &DevNumMonitor::existDevice, Camera::instance(), &Camera::refreshCamera);
    connect(Camera::instance(), SIGNAL(cameraSwitched(const QString &)), this, SLOT(onSwitchCameraSuccess(const QString &)));
    // 摄像头重连，刷新设置页分辨率下拉框内容
    connect(Camera::instance(), SIGNAL(cameraDevRestarted()), &Settings::get(), SLOT(setNewResolutionList()));
    // 当前无摄像头设备，给出无摄像头提示
    connect(m_devnumMonitor, &DevNumMonitor::noDeviceFound, this, [=](){
        Camera::instance()->stopCamera();
        DataManager::instance()->setdevStatus(NOCAM);
        m_videoPre->showNocam();
        QImage image;
        m_takePhotoSettingArea->onUpdateFilterImage(&image);
    });
    // 当前设备断开连接，给出无摄像头提示
    connect(Camera::instance(), &Camera::currentCameraDisConnected, this, [=](){
        Camera::instance()->stopCamera();
        DataManager::instance()->setdevStatus(NOCAM);
        m_videoPre->showNocam();
        QImage image;
        m_takePhotoSettingArea->onUpdateFilterImage(&image);
    });
    connect(Camera::instance(), &Camera::cameraCannotUsed, this, [=](){
        DataManager::instance()->setdevStatus(CAM_CANNOT_USE);
        m_videoPre->showCamUsed();
    });
}

void CMainWindow::initRightButtons()
{
    m_cameraSwitchBtn = new SwitchCameraBtn(this);
    m_cameraSwitchBtn->setFixedSize(SwitchcameraDiam, SwitchcameraDiam);
    m_cameraSwitchBtn->setObjectName(CAMERA_SWITCH_BTN);
    m_cameraSwitchBtn->setAccessibleName(CAMERA_SWITCH_BTN);
    m_cameraSwitchBtn->setToolTip(tr("Switch Cameras"));

    m_photoRecordBtn = new photoRecordBtn(this);
    m_photoRecordBtn->setFixedSize(photeRecordDiam, photeRecordDiam);
    m_photoRecordBtn->setObjectName(BUTTON_PICTURE_VIDEO);
    m_photoRecordBtn->setAccessibleName(BUTTON_PICTURE_VIDEO);
    m_photoRecordBtn->setToolTip(tr("Photo"));

    m_modeSwitchBox = new RollingBox(this);
    m_modeSwitchBox->setFixedSize(switchBtnWidth, switchBtnHeight * 3 + rightOffset * 2);
    QStringList content;
    content << tr("Photo") << tr("Video");
    m_modeSwitchBox->setContentList(content);
    m_modeSwitchBox->setObjectName(MODE_SWITCH_BOX);
    m_modeSwitchBox->setAccessibleName(MODE_SWITCH_BOX);
    m_modeSwitchBox->show();

    m_snapshotLabel = new ImageItem(this);
    m_snapshotLabel->setFixedSize(snapLabelDiam, snapLabelDiam);
    m_snapshotLabel->setObjectName(THUMBNAIL_PREVIEW);
    m_snapshotLabel->setAccessibleName(THUMBNAIL_PREVIEW);

    if (DataManager::instance()->encodeEnv() != QCamera_Env)
        connect(m_cameraSwitchBtn, SIGNAL(clicked()), m_videoPre, SLOT(onChangeDev()));
    else
        connect(m_cameraSwitchBtn, SIGNAL(clicked()), Camera::instance(), SLOT(switchCamera()));
    connect(m_modeSwitchBox, &RollingBox::currentValueChanged, this, &CMainWindow::onModeSwitchBoxValueChanged);
    connect(m_photoRecordBtn, SIGNAL(clicked()), this, SLOT(onPhotoRecordBtnClked()));
    connect(m_snapshotLabel, SIGNAL(trashFile(const QString &)), SLOT(onTrashFile(const QString &)));

    //系统文件夹变化信号
    connect(&m_fileWatcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(onDirectoryChanged(const QString &)));
    //系统文件变化信号
    connect(&m_fileWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(onDirectoryChanged(const QString &)));
    //系统文件夹变化信号
    connect(&m_fileWatcherUp, SIGNAL(directoryChanged(const QString &)), this, SLOT(onDirectoryChanged(const QString &)));
    //系统文件夹变化信号
    connect(&m_fileWatcherUp, SIGNAL(fileChanged(const QString &)), this, SLOT(onDirectoryChanged(const QString &)));
    //摄像头切换成功信号
    connect(m_videoPre, SIGNAL(switchCameraSuccess(const QString &)), this, SLOT(onSwitchCameraSuccess(const QString &)));
    locateRightButtons();
}

void CMainWindow::locateRightButtons()
{
    if (nullptr == m_photoRecordBtn) {
        return;
    }
    int buttonCenterX = width() - rightOffset - photeRecordDiam / 2;
    int buttonCenterY = height() / 2;
    int photoRecordLeftX = buttonCenterX - photeRecordDiam / 2;
    int photoRecordLeftY = buttonCenterY - photeRecordDiam / 2;
    m_photoRecordBtn->move(photoRecordLeftX, photoRecordLeftY);

    int switchCameraOffset = height() / 15;
    int switchCameraX = buttonCenterX - SwitchcameraDiam / 2;
    int switchCameraY = photoRecordLeftY - SwitchcameraDiam - switchCameraOffset;
    m_cameraSwitchBtn->move(switchCameraX, switchCameraY);

    int switchBtnX = buttonCenterX - switchBtnWidth / 2;
    int switchBtnY = photoRecordLeftY + photeRecordDiam;
    m_modeSwitchBox->move(switchBtnX, switchBtnY);

    int snapLabelOffset = height() / 10;
    int snapLabelX = buttonCenterX - snapLabelDiam / 2;
    int snapLabelY = switchBtnY + switchBtnHeight * 2 + snapLabelOffset;
    m_snapshotLabel->move(snapLabelX, snapLabelY);
}

void CMainWindow::setSelBtnHide()
{
    m_bSwitchCameraShowEnable = false;
    if (!m_cameraSwitchBtn->isHidden()) {
        showChildWidget();
    }
}

void CMainWindow::onLocalTimeChanged()
{
    m_SpaceKeyInterval = QDateTime::currentMSecsSinceEpoch();
}

void CMainWindow::setSelBtnShow()
{
    m_bSwitchCameraShowEnable = true;
    if (m_cameraSwitchBtn->isHidden()) {
        showChildWidget();
    }
}

void CMainWindow::setupTitlebar()
{
    m_pTitlebar = new Titlebar(this);
    m_titlemenu = new DMenu(this);
    m_actionSettings = new QAction(tr("Settings"), this);

    m_pTitlebar->setObjectName(TITLEBAR);
    m_pTitlebar->setAccessibleName(TITLEBAR);

    m_titlemenu->setObjectName(TITLE_MUNE);
    m_titlemenu->setAccessibleName(TITLE_MUNE);

    m_actionSettings->setObjectName("SettingAction");
    m_titlemenu->addAction(m_actionSettings);
    m_pTitlebar->titlebar()->setMenu(m_titlemenu);
    m_pTitlebar->titlebar()->setParent(this);

    m_pTitlebar->move(0, 0);
    m_pTitlebar->setFixedHeight(50);
}

void CMainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_labelCameraName && m_bUIinit) {
        m_labelCameraName->move((width() - labelCameraNameWidth) / 2, height() - 20 - labelCameraNameHeight);
    }
    if (m_filterName && m_bUIinit) {
        m_filterName->move((width() - labelFilterNameWidth) / 2, height() - 20 - labelFilterNameHeight);
    }
    locateRightButtons();
    if (m_videoPre)
        m_videoPre->update();

    m_pTitlebar->setFixedWidth(this->size().width());
    m_pTitlebar->titlebar()->setFixedWidth(this->size().width());

    QTimer::singleShot(1, this, [ = ] {
        if (nullptr != m_takePhotoSettingArea) {
            m_takePhotoSettingArea->moveToParentLeft();
            m_takePhotoSettingArea->resizeScrollHeight(height() - minWindowHeight);
        }
    });
}

void CMainWindow::closeEvent(QCloseEvent *event)
{
    if (m_videoPre->getCapStatus()) {
        CloseDialog closeDlg(this, tr("Video recording is in progress. Close the window?"));
        closeDlg.setObjectName(CLOSE_DIALOG);
        closeDlg.setAccessibleName(CLOSE_DIALOG);
#ifdef UNITTEST
        closeDlg.show();
        event->ignore();
        closeDlg.close();
#else
        int ret = closeDlg.exec();

        switch (ret) {
        case 0:
            event->ignore();
            break;
        case 1:
            m_videoPre->onEndBtnClicked();
            event->accept();
            break;
        default:
            event->ignore();
            break;
        }
#endif
    }
}

void CMainWindow::changeEvent(QEvent *event)
{
    Q_UNUSED(event);

    if (windowState() & Qt::WindowMinimized) {
        onStopPhotoAndRecord();
    }
}

void CMainWindow::onSettingsDlgClose()
{
    bool bPathChanged = false;
    QString setVideoPath = lastOpenedPath(QStandardPaths::MoviesLocation);
    QString setPicPath  = lastOpenedPath(QStandardPaths::PicturesLocation);

    //判断图片路径是否改变
    if (m_picPath != setPicPath) { //图片路径修改了
        QDir setPicDir(setPicPath);
        QDir lastPicDir(m_picPath);
        setPicDir.cdUp();
        lastPicDir.cdUp();
        m_videoPre->setSavePicFolder(setPicPath);
        m_fileWatcher.removePath(m_picPath);
        if (false == m_fileWatcher.directories().contains(setPicPath)) {
            m_fileWatcher.addPath(setPicPath);
        }

        if (setPicDir.path() != lastPicDir.path()) {
            m_fileWatcherUp.removePath(lastPicDir.path());
            if (false == m_fileWatcherUp.directories().contains(setPicDir.path())) {
                m_fileWatcherUp.addPath(setPicDir.path());
            }
        }
        m_picPath = setPicPath;
        bPathChanged = true;
    }

    if (m_videoPath != setVideoPath) {
        QDir setVideoDir(setVideoPath);
        QDir lastVideoDir(m_videoPath);
        setVideoDir.cdUp();
        lastVideoDir.cdUp();
        m_videoPre->setSaveVdFolder(setVideoPath);
        m_fileWatcher.removePath(m_videoPath);
        if (false == m_fileWatcher.directories().contains(setVideoPath)) {
            m_fileWatcher.addPath(setVideoPath);
        }

        if (setVideoDir.path() != lastVideoDir.path()) {
            m_fileWatcherUp.removePath(lastVideoDir.path());
            if (false == m_fileWatcherUp.directories().contains(setVideoDir.path())) {
                m_fileWatcherUp.addPath(setVideoDir.path());
            }
        }
        m_videoPath = setVideoPath;
        bPathChanged = true;
    }


    int nContinuous = Settings::get().getOption("photosetting.photosnumber.takephotos").toInt();
    int nDelayTime = Settings::get().getOption("photosetting.photosdelay.photodelays").toInt();
    bool soundphoto = Settings::get().getOption("photosetting.audiosetting.soundswitchbtn").toBool();

    /**********************************************/
    switch (nContinuous) {
    case 1:
        nContinuous = 4;
        break;
    case 2:
        nContinuous = 10;
        break;
    default:
        nContinuous = 1;
        break;
    }

    switch (nDelayTime) {
    case 1:
        nDelayTime = 3;
        break;
    case 2:
        nDelayTime = 6;
        break;
    default:
        nDelayTime = 0;
        break;
    }

    if (soundphoto)
        set_takeing_photo_sound(1);
    else
        set_takeing_photo_sound(0);

    m_videoPre->setInterval(nDelayTime);
    m_videoPre->setContinuous(nContinuous);
    Settings::get().settings()->sync();

    if (bPathChanged) {
        reflushSnapshotLabel();
    }
}

void CMainWindow::onEnableSettings(bool bTrue)
{
    m_actionSettings->setEnabled(bTrue);
}

void CMainWindow::onTakePicDone()
{
    onEnableSettings(true);
}

void CMainWindow::onTakePicOnce()
{
    qDebug() << "onTakePicOnce";
}

void CMainWindow::onTakePicCancel()
{
    onEnableSettings(true);
}

void CMainWindow::onTakeVdDone()
{
    onEnableSettings(true);

    QTimer::singleShot(200, this, [ = ] {
        QString strFileName = m_videoPath + QDir::separator() + DataManager::instance()->getstrFileName();
        QFile file(strFileName);

        if (!file.exists())
            usleep(200000);

    });
    qDebug() << "Taking video completed!";
    reflushSnapshotLabel();
}

void CMainWindow::onTakeVdCancel()   //保存视频完成，通过已有的文件检测实现缩略图恢复，这里不需要额外处理
{
    onEnableSettings(true);
}

bool CMainWindow::eventFilter(QObject *obj, QEvent *e)
{
    /**
     * @brief 关注焦点进入事件和鼠标点击事件。
     * 焦点进入：设置对应控件索引序号。
     * 鼠标点击：焦点移入预览界面，并将下标设置为0
     */

    DWindowMinButton *windowMinBtn = m_pTitlebar->titlebar()->findChild<DWindowMinButton *>("DTitlebarDWindowMinButton");
    DWindowOptionButton *windowoptionButton = m_pTitlebar->titlebar()->findChild<DWindowOptionButton *>("DTitlebarDWindowOptionButton");
    DWindowMaxButton *windowMaxBtn = m_pTitlebar->titlebar()->findChild<DWindowMaxButton *>("DTitlebarDWindowMaxButton");
    DWindowCloseButton *windowCloseBtn = m_pTitlebar->titlebar()->findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
    DPushButton *picvideobtn = findChild<DPushButton *>(BUTTON_PICTURE_VIDEO);
    DPushButton *endbtn = findChild<DPushButton *>(BUTTON_TAKE_VIDEO_END);

    if (e->type() == QEvent::MouseButtonPress) {
        m_takePhotoSettingArea->closeAllGroup();
    }

    if ((obj == windowoptionButton) && (e->type() == QEvent::FocusIn)) {
        DataManager::instance()->m_tabIndex = 4;
    } else if ((obj == windowMinBtn) && (e->type() == QEvent::FocusIn)) {
        DataManager::instance()->m_tabIndex = 5;
    } else if ((obj == windowMaxBtn) && (e->type() == QEvent::FocusIn)) {
        DataManager::instance()->m_tabIndex = 6;
    } else if ((obj == windowCloseBtn) && (e->type() == QEvent::FocusIn)) {
        DataManager::instance()->m_tabIndex = 7;
    } else if ((obj == picvideobtn) && (e->type() == QEvent::FocusIn)) {
        DataManager::instance()->m_tabIndex = 8;
    } else if ((obj == endbtn) && (e->type() == QEvent::FocusIn)) {
        DataManager::instance()->m_tabIndex = 9;
    } else if (e->type() == QEvent::MouseButtonPress) {
        DataManager::instance()->m_tabIndex = 0;
        m_pTitlebar->setFocus();
    } else {
        QWidget::eventFilter(obj, e);//调用父类事件过滤器
    }
    return QWidget::eventFilter(obj, e);
}

/*
 * 右侧按钮显示状态
 * 1、正在拍照，闪光灯，全部不显示
 * 2、拍照倒计时 只显示拍照按钮
 * 3、正在录像，只显示拍照按钮
 * 4、无摄像头或者只有一个摄像头不显示切换摄像机状态
*/

void CMainWindow::showChildWidget()
{
    //正在拍照过程中
    if (photoNormal != m_photoState) {
        showWidget(m_cameraSwitchBtn, false);
        showWidget(m_snapshotLabel, false);
        showWidget(m_modeSwitchBox, false);
        showWidget(m_takePhotoSettingArea, false);
        showWidget(m_photoRecordBtn, true);
        showWidget(m_filterName, false);
        return;
    }
    if (m_bRecording) {  //正在录像
        showWidget(m_cameraSwitchBtn, false);
        showWidget(m_snapshotLabel, false);
        showWidget(m_modeSwitchBox, false);
        showWidget(m_takePhotoSettingArea, false);
        showWidget(m_photoRecordBtn, true);
        return;
    }
    showWidget(m_snapshotLabel, true);
    showWidget(m_cameraSwitchBtn, m_bSwitchCameraShowEnable);
    showWidget(m_modeSwitchBox, true);
    showWidget(m_photoRecordBtn, true);
    showWidget(m_takePhotoSettingArea, true);
    showWidget(m_filterName, m_bShowFilterName);
}

void CMainWindow::showWidget(DWidget *widget, bool bShow)
{
    if (widget) {
        if (!bShow) {
            widget->hide();
        }

        if (bShow) {
            widget->show();
        }
    }
}

CMainWindow::~CMainWindow()
{
    if (m_devnumMonitor) {
        m_devnumMonitor->deleteLater();
        m_devnumMonitor = nullptr;
    }

    if (m_videoPre) {
        m_videoPre->deleteLater();
        m_videoPre = nullptr;
    }
    if (m_windowStateThread) {
        m_windowStateThread->requestInterruption();
        m_windowStateThread->wait();
        m_windowStateThread->deleteLater();
    }
    qDebug() << "stop_encoder_thread";
}
