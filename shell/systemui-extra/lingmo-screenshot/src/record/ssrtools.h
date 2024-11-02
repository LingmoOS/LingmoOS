#ifndef SSRTOOLS_H
#define SSRTOOLS_H

#include <QWidget>
//#include "mypopup.h"
#include "AV/output/OutputSettings.h"
#include <memory>
#include "AV/output/OutputManager.h"
#include "Global.h"
#include "common/WidgetWrapper.h"

#if SSR_USE_PULSEAUDIO
#include "AV/input/PulseAudioInput.h"
#endif


namespace Ui {
class ssrtools;
}

class mypopup;
class X11Input;


class ssrtools : public QWidget
{
    Q_OBJECT

private:
    static constexpr int PRIORITY_RECORD = 0, PRIORITY_PREVIEW = -1;

public:
    explicit ssrtools(QWidget *parent = nullptr, mypopup *myp = nullptr);
    ~ssrtools();

    void setStyle();

    void Init();

    void Input_init();
    void Output_init();
    void Prepare_Record();

    void LoadSettings();

    void LoadInputSettings(QSettings* settings);
    void LoadOutputSettings(QSettings* settings);


    //record
    void StopPage(bool save);
    void StartOutput();
    void StopOutput(bool final);
    void StartInput();
    void StopInput();


private slots:
    void on_m_toolButton_options_clicked();

    void on_m_pushButton_start_clicked();


    void on_m_pushbutton_video_select_rectangle_clicked();

    void on_m_pushbutton_video_select_window_clicked();

    void OnUpdateInformation();


private:
    void LoadInputProfileSettings(QSettings* settings);
    void LoadOutputProfileSettings(QSettings* settings);

    void OnRecordPause();

public:
    void OnRecordStart();
    void RecordStartOrPause();

public:
#if SSR_USE_PULSEAUDIO
    QString GetPulseAudioSourceName();
#endif

private:
    void StartGrabbing();
    void StopGrabbing();
    void UpdateRubberBand();
    void SetVideoAreaFromRubberBand();


#if SSR_USE_PULSEAUDIO
    void LoadPulseAudioSources();
#endif

    //record
    void FinishOutput();
    void UpdateInput();
    void UpdateRecordButton();
    void UpdateSchedule();

public slots:
    void OnUpdateRecordingFrame();
    void OnUpdateVideoAreaFields();
    void OnUpdateVideoAreaFields_(QRect &rect);
    void OnRecordCancel();
    void OnRecordSave();
    void OnScheduleTimer();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::ssrtools *ui;
    mypopup *mp;
    QSettings settings;

    bool options_show;

    bool m_video_area_follow_fullscreen;
    unsigned int m_video_x, m_video_y, m_video_in_width, m_video_in_height;
    unsigned int m_video_frame_rate;

    bool m_video_record_cursor;

    bool m_video_follow_mouse;

    bool m_audio_enabled;
    unsigned int m_audio_channels, m_audio_sample_rate;


    OutputSettings m_output_settings;
    std::unique_ptr<OutputManager> m_output_manager;

    QButtonGroup *m_buttongroup_video_area;

    QLabel *m_label_video_x, *m_label_video_y, *m_label_video_w, *m_label_video_h;

    bool m_grabbing, m_selecting_window;
    std::unique_ptr<RecordingFrameWindow>  m_rubber_band, m_recording_frame;
    QRect m_rubber_band_rect, m_select_window_outer_rect, m_select_window_inner_rect;

#if SSR_USE_PULSEAUDIO
    bool m_pulseaudio_available;
    std::vector<PulseAudioInput::Source> m_pulseaudio_sources;
#endif

    ssr::enum_audio_backend m_audio_backend;

    bool m_separate_files;

    std::unique_ptr<X11Input> m_x11_input;
#if SSR_USE_PULSEAUDIO
    QString m_pulseaudio_source;
    std::unique_ptr<PulseAudioInput> m_pulseaudio_input;
#endif

//    VideoPreviewer *m_video_previewer;


private:
    //output
//    struct ContainerData {
//        QString name, avname;
//        QStringList suffixes;
//        QString filter;
//        std::set<ssr::enum_video_codec> supported_video_codecs;
//        std::set<ssr::enum_audio_codec> supported_audio_codecs;
//        inline bool operator<(const ContainerData& other) const { return (avname < other.avname); }
//    };


//    std::vector<ContainerData> m_containers, m_containers_av;
//    std::vector<VideoCodecData> m_video_codecs, m_video_codecs_av;
//    std::vector<VideoCodecData> m_video_codecs_av;
//    std::vector<AudioCodecData> m_audio_codecs, m_audio_codecs_av;
//    std::vector<AudioCodecData> m_audio_codecs_av;
//    std::vector<AudioKBitRate> m_audio_kbit_rates;

private:
    //record
    bool m_output_started;
    ssr::enum_video_area m_video_area;

    QString m_file_base;
    QString m_file_protocol;

    bool m_add_timestamp;

    bool m_input_started;
    bool m_recorded_something;

    QTimer *m_timer_schedule, *m_timer_update_info;

    bool m_schedule_active;


public:
    inline ssr::enum_video_area GetVideoArea() { return (ssr::enum_video_area) clamp(m_buttongroup_video_area->checkedId(), 0, int(ssr::enum_video_area::VIDEO_AREA_COUNT) - 1); }

    void SetVideoArea(ssr::enum_video_area area);
//    void SetVideoAreaScreen(unsigned int screen);


    void SetVideoX(unsigned int x);
    void SetVideoY(unsigned int y);
    void SetVideoW(unsigned int w);
    void SetVideoH(unsigned int h);
    unsigned int GetVideoX();
    unsigned int GetVideoY();
    unsigned int GetVideoW();
    unsigned int GetVideoH();

    inline void SetVideoRecordRursor(bool b) { m_video_record_cursor = b; }
    inline bool GetVideoRecordRursor() { return m_video_record_cursor; }

    inline void SetVideoRecordFollowMouse(bool b) { m_video_follow_mouse = b; }
    inline bool GetVideoRecordFollowMouse() { return m_video_follow_mouse; }

//    inline std::vector<ContainerData> GetContainers() { return m_containers; }
//    inline std::vector<ContainerData> GetContainersAV() { return m_containers_av; }
//    inline std::vector<VideoCodecData> GetVideoCodecs() { return m_video_codecs; }
//    inline std::vector<VideoCodecData> GetVideoCodecsAV() { return m_video_codecs_av; }
//    inline std::vector<AudioCodecData> GetAudioCodecs() { return m_audio_codecs; }
//    inline std::vector<AudioCodecData> GetAudioCodecsAV() { return m_audio_codecs_av; }
//    inline std::vector<AudioKBitRate> GetAudioKBitRates() { return m_audio_kbit_rates; }

    inline Ui::ssrtools* GetUi() { return ui; }


#if SSR_USE_PULSEAUDIO
    inline unsigned int GetPulseAudioSource() { return clamp(m_combobox_pulseaudio_source->currentIndex(), 0, (int) m_pulseaudio_sources.size() - 1); }
#endif

    //will change
private:
//    QLineEdit *m_lineedit_audio_options_not_shown;
#if SSR_USE_PULSEAUDIO
//	QLabel *m_label_pulseaudio_source;
    QComboBox *m_combobox_pulseaudio_source;
//	QPushButton *m_pushbutton_pulseaudio_refresh;
#endif
public:
//    inline void SetAudioOptions(const QString& options) { m_lineedit_audio_options_not_shown->setText(options); }
//    inline QString GetAudioOptions() { return m_lineedit_audio_options_not_shown->text(); }

};

#endif // SSRTOOLS_H
