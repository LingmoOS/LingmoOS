#include "option_record_tool.h"

OptionRecordTool::OptionRecordTool(QObject *parent) : AbstractActionTool(parent)
{
}
bool OptionRecordTool::closeOnButtonPressed() const
{
     return false;
}

QIcon OptionRecordTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
   // return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-copy.svg") :
     //                 QIcon(QStringLiteral(":/img/material/white/") + "content-copy.svg");
    return QIcon();
}
#ifdef SUPPORT_LINGMO
QIcon OptionRecordTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
   // return inEditor ?  QIcon(QStringLiteral(":/img/material/black/") + "content-copy.svg") :
     //                 QIcon(QStringLiteral(":/img/material/white/") + "content-copy.svg");
    return QIcon();

}
#endif
QString OptionRecordTool::name() const
{
    return tr("OptionRecord");
}
QString OptionRecordTool::nameID()
{
     return QLatin1String("");
}
QString OptionRecordTool::description() const
{
    return tr("options record tool");
    //return tr("图片保存配置");
}

CaptureTool* OptionRecordTool::copy(QObject *parent)
{
     return new OptionRecordTool(parent);
}

void OptionRecordTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_OPTION_RECORD);
}
