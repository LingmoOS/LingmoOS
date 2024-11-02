#include "follow_mouse_record_tool.h"

FollowMouseRecordTool::FollowMouseRecordTool(QObject *parent) : AbstractActionTool(parent)
{
}

bool FollowMouseRecordTool::closeOnButtonPressed() const
{
    return false;
}

QString FollowMouseRecordTool::name() const
{
    return tr("FollowMouseRecord");
}

bool FollowMouseRecordTool::isIsolated() const
{
    return true;
}

QIcon FollowMouseRecordTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "follow_mouse_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "follow_mouse_record_noactive.png");
    //    return QIcon(QStringLiteral(":/img/material/black/") + "luping_icon.svg");
}
#ifdef SUPPORT_LINGMO
QIcon FollowMouseRecordTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "follow_mouse_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "follow_mouse_record_noactive.png");
    //    return QIcon(QStringLiteral(":/img/material/black/") + "luping_icon.svg");
}
#endif
QString FollowMouseRecordTool::description() const
{
    return tr("FollowMouseRecord");
}

CaptureTool *FollowMouseRecordTool::copy(QObject *parent)
{
    return new FollowMouseRecordTool(parent);
}

void FollowMouseRecordTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_FOLLOW_MOUSE_RECORD);
}
