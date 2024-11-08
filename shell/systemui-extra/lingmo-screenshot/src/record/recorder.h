#ifndef RECORD_H
#define RECORD_H

#include <QObject>

#include "src/widgets/capture/capturebutton.h"

class ssrtools;
class mypopup;

class Recorder : public QObject
{
    Q_OBJECT
    friend class CaptureWidget;
public:
    explicit Recorder(QWidget *parent = nullptr);

    void OnRecordCursorClicked(bool isPressed);
    void OnRecordFollowMouseClicked(bool isPressed);
    void OnRecordAudioClicked(bool isPressed);
    void OnRecordOptionClicked();
    void OnRecordStartClicked();

private:
    ssrtools* ssr;
    mypopup *mp;
    QPushButton *m_pushbutton_save, *m_pushbutton_cancel;

private slots:
    void record_save_clicked();
    void record_cancel_clicked();
};

#endif // RECORD_H
