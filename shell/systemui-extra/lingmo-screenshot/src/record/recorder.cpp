#include "recorder.h"

#include "ssrtools.h"
#include "mypopup.h"
#include "src/tools/capturetool.h"

Recorder::Recorder(QWidget *parent)
{
    mp =  new mypopup(parent);
    ssr = new ssrtools(parent, mp);
    m_pushbutton_save = new QPushButton(parent);
    m_pushbutton_cancel = new QPushButton(parent);

    connect(m_pushbutton_cancel, SIGNAL(clicked()), this, SLOT(record_cancel_clicked()));
    connect(m_pushbutton_save, SIGNAL(clicked()), this, SLOT(record_save_clicked()));
}

void Recorder::OnRecordCursorClicked(bool isPressed)
{
    ssr->SetVideoRecordRursor(isPressed);
}

void Recorder::OnRecordFollowMouseClicked(bool isPressed)
{
    ssr->SetVideoRecordFollowMouse(isPressed);
}

void Recorder::OnRecordAudioClicked(bool isPressed)
{
    ssr->SetVideoRecordRursor(isPressed);
}

void Recorder::OnRecordOptionClicked()
{

}

void Recorder::OnRecordStartClicked()
{
    ssr->OnRecordStart();
    m_pushbutton_cancel->show();
    m_pushbutton_save->show();
}

void Recorder::record_save_clicked()
{
    m_pushbutton_cancel->hide();
    m_pushbutton_save->hide();
//        syslog(LOG_INFO, "will save, cancel and save button hide");
    ssr->OnRecordSave();
}

void Recorder::record_cancel_clicked()
{
    m_pushbutton_cancel->hide();
    m_pushbutton_save->hide();
//        syslog(LOG_INFO, "will save, cancel and save button hide");
    ssr->OnRecordCancel();
}
