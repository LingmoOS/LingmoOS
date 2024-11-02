#include "audio_record_tool.h"

AudioRecordTool::AudioRecordTool(QObject *parent) : AbstractActionTool(parent)
{
    setIsInitActive(true);
    setIsPressed(true);
}

bool AudioRecordTool::closeOnButtonPressed() const
{
    return false;
}

QString AudioRecordTool::name() const
{
    return tr("AudioRecord");
}

bool AudioRecordTool::isIsolated() const
{
    return true;
}

QIcon AudioRecordTool::icon(const QColor &background, bool inEditor) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "audio_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "audio_record_noactive.png");
    //    return QIcon(QStringLiteral(":/img/material/black/") + "luping_icon.svg");
}

QIcon AudioRecordTool::icon(const QColor &background, bool inEditor, const CaptureContext &context) const
{
    Q_UNUSED(background);
    return inEditor ? QIcon(QStringLiteral(":/img/material/black/record/") + "audio_record_active.png") :
                      QIcon(QStringLiteral(":/img/material/black/record/") + "audio_record_noactive.png");
    //    return QIcon(QStringLiteral(":/img/material/black/") + "luping_icon.svg");
}

QString AudioRecordTool::description() const
{
    return tr("AudioRecord");
}

CaptureTool *AudioRecordTool::copy(QObject *parent)
{
    return new AudioRecordTool(parent);
}

void AudioRecordTool::pressed(const CaptureContext &context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_AUDIO_RECORD);
}
