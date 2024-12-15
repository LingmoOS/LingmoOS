// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageloadtool.h"
#include "drawboard.h"
#include "setting.h"

#include <QDebug>
#include <QStandardPaths>
#include <QToolButton>

class ImageLoadTool::ImageLoadTool_private
{
public:
    explicit ImageLoadTool_private(ImageLoadTool *father): _father(father) {}
    ImageLoadTool *_father;
};
ImageLoadTool::ImageLoadTool(QObject *parent)
    : DrawTool(parent), ImageLoadTool_d(new ImageLoadTool_private(this))
{
    auto m_picBtn = toolButton();
    m_picBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_I)));
    setWgtAccesibleName(m_picBtn, "Import tool button");
    m_picBtn->setToolTip(tr("Import (I)"));
    m_picBtn->setIconSize(TOOL_ICON_RECT);
    m_picBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_picBtn->setCheckable(true);
    m_picBtn->setIcon(QIcon::fromTheme("picture_normal"));
    connect(m_picBtn, &QToolButton::toggled, m_picBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("picture_normal");
        QIcon activeIcon = QIcon::fromTheme("picture_highlight");
        m_picBtn->setIcon(b ? activeIcon : icon);
    });
}

SAttrisList ImageLoadTool::attributions()
{
    SAttrisList result;
    return result;
}

ImageLoadTool::~ImageLoadTool()
{
    //delete ImageLoadTool_d;
}

int ImageLoadTool::toolType() const
{
    return picture;
}

void ImageLoadTool::pressOnScene(ToolSceneEvent *event)
{

}

void ImageLoadTool::moveOnScene(ToolSceneEvent *event)
{

}

void ImageLoadTool::releaseOnScene(ToolSceneEvent *event)
{

}


void ImageLoadTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
//    if (nowStatus == EReady) {
//        drawBoard()->setCurrentTool(selection);
//    }
    if (nowStatus == EReady) {
        DrawTool::onStatusChanged(oldStatus, nowStatus);
        QFileDialog fileDialog(drawBoard());
        fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        fileDialog.setWindowTitle(tr("Import Picture"));

        QStringList filters;
        auto formatsList = Setting::instance()->readableFormats();
        if (!formatsList.isEmpty())
            formatsList.removeFirst();
        auto formats = QString(" *.") + formatsList.join(" *.");
        filters << formats;

        fileDialog.setNameFilters(filters);
        fileDialog.setFileMode(QFileDialog::ExistingFiles);
        fileDialog.setDirectory(Setting::instance()->defaultFileDialogPath());

        if (fileDialog.exec() == QDialog::Accepted && drawBoard() != nullptr) {
            QStringList filenames = fileDialog.selectedFiles();

            int ret = drawBoard()->execPicturesLimit(filenames.size());
            if (ret == 0) {
                drawBoard()->loadFiles(filenames);
            }
        }
        drawBoard()->setCurrentTool(selection);
    }
}
