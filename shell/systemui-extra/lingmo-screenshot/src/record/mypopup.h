#pragma once

#include <QWidget>

#include "Global.h"
#include "common/WidgetWrapper.h"


namespace Ui {
class mypopup;
}
class ssrtools;


class mypopup : public QWidget
{
    Q_OBJECT

public:
//    explicit mypopup(ssrtools* ssr, QWidget *parent = nullptr);
    explicit mypopup(QWidget *parent = nullptr);
    ~mypopup();

//    inline Ui::mypopup *getUI() {return ui;}

    void LoadScreenConfigurations();
    void LoadAVProfileSettings(QSettings *settings);

public:

    QString GetFileProtocol();
    QString GetContainerAVName();
    QString GetVideoCodecAVName();
    QString GetAudioCodecAVName();
    QString GetAudioKBitRateName();

private:
    Ui::mypopup *ui;
//    ssrtools *ssr;
//    std::vector<ContainerData> m_containers, m_containers_av;
    std::vector<ScreenLabelWindow*> m_screen_labels;

    ssr::enum_container m_old_container;
    unsigned int m_old_container_av;

    std::vector<ContainerData> m_containers, m_containers_av;
    std::vector<VideoCodecData> m_video_codecs, m_video_codecs_av;
    std::vector<AudioCodecData> m_audio_codecs, m_audio_codecs_av;
    std::vector<AudioKBitRate> m_audio_kbit_rates;

private:
    QLabel *m_label_container_av_not_shown;
    QComboBox *m_combobox_container_av_not_shown;
    QLineEdit *m_lineedit_file_not_shown;
    QLineEdit *m_lineedit_audio_options_not_shown;

    void InputInit();
    void OutputInit(QWidget *parent);

public slots:
//    void OnUpdateVideoAreaFields();
    void OnUpdateSuffixAndContainerFields();
    void OnUpdateContainerFields();

public:
//    void OnUpdateVideoAreaFields(int& sc);

private slots:
    void OnIdentifyScreens();
    void OnStopIdentifyScreens();

    void OnBrowse();


public:
    inline Ui::mypopup *GetUi() {return ui;}
    ssr::enum_container GetContainer();
    unsigned int GetContainerAV();
    inline void SetContainer(ssr::enum_container container);
    inline void SetContainerAV(unsigned int container);
    inline QString GetFile() { return m_lineedit_file_not_shown->text(); }
    inline void SetFile(const QString& file) { m_lineedit_file_not_shown->setText(file); }


    ssr::enum_video_codec GetVideoCodec();
    void SetVideoCodec(ssr::enum_video_codec video_codec);

//    ssr::enum_video_codec GetVideoCodecAV();
    ssr::enum_audio_codec GetAudioCodec();
    void SetAudioCodec(ssr::enum_audio_codec audio_codec);

//    ssr::enum_audio_codec GetAudioCodecAV();

    int GetAudioKBitRate();

//    void SetVideoKBitRate(unsigned int kbit_rate);

    inline std::vector<ContainerData> GetContainers() { return m_containers; }
    inline std::vector<ContainerData> GetContainersAV() { return m_containers_av; }
    inline std::vector<VideoCodecData> GetVideoCodecs() { return m_video_codecs; }
    inline std::vector<VideoCodecData> GetVideoCodecsAV() { return m_video_codecs_av; }
    inline std::vector<AudioCodecData> GetAudioCodecs() { return m_audio_codecs; }
    inline std::vector<AudioCodecData> GetAudioCodecsAV() { return m_audio_codecs_av; }
    inline std::vector<AudioKBitRate> GetAudioKBitRates() { return m_audio_kbit_rates; }



};
