#include "cursor_record_tool.h"

CursorRecordTool::CursorRecordTool(QObject *parent) : AbstractActionTool(parent)
{
    setIsInitActive(true);
    setIsPressed(true);
}

bool CursorRecordTool::closeOnButtonPressed() const
{
    return false;
}

QString CursorRecordTool::name() const
{
    return tr("CursorRecord");
}

bool CursorRecordTool::isIsolated() const
{
    return true;
}

QIcon CursorRecordTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "cursor_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "cursor_record_noactive.png");
}

#ifdef SUPPORT_LINGMO
QIcon CursorRecordTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "cursor_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "cursor_record_noactive.png");
}
#endif

QString CursorRecordTool::description() const
{
    return tr("CursorRecord");
}

CaptureTool *CursorRecordTool::copy(QObject *parent)
{
    return new CursorRecordTool(parent);
}

void CursorRecordTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
//    b->setIcon(b->tool()->icon(m_contrastUiColor,false));

    emit requestAction(REQ_CURSOR_RECORD);
}
