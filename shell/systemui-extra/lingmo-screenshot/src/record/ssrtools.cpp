#include "ssrtools.h"
#include "ui_ssrtools.h"
//#include "Input_Widgets.h"
#include "my_qt.h"
#include "common/utils.h"
#include "CommandLineOptions.h"
#include "common/EnumStrings.h"

#include "mypopup.h"

#include "../widgets/capture/capturewidget.h"
#include "Logger.h"

#include "AV/input/X11Input.h"
#include "common/X_utils.hpp"

#include <syslog.h>

ENUMSTRINGS(ssr::enum_video_area) = {
    {ssr::enum_video_area::VIDEO_AREA_SCREEN, "screen"},
    {ssr::enum_video_area::VIDEO_AREA_FIXED, "fixed"},
    {ssr::enum_video_area::VIDEO_AREA_CURSOR, "cursor"},
#if SSR_USE_OPENGL_RECORDING
    {PageInput::VIDEO_AREA_GLINJECT, "glinject"},
#endif
};
ENUMSTRINGS(ssr::enum_container) = {
    {ssr::enum_container::CONTAINER_MKV, "mkv"},
    {ssr::enum_container::CONTAINER_MP4, "mp4"},
    {ssr::enum_container::CONTAINER_WEBM, "webm"},
    {ssr::enum_container::CONTAINER_OGG, "ogg"},
    {ssr::enum_container::CONTAINER_OTHER, "other"},
};
ENUMSTRINGS(ssr::enum_video_codec) = {
    {ssr::enum_video_codec::VIDEO_CODEC_H264, "h264"},
    {ssr::enum_video_codec::VIDEO_CODEC_VP8, "vp8"},
    {ssr::enum_video_codec::VIDEO_CODEC_THEORA, "theora"},
    {ssr::enum_video_codec::VIDEO_CODEC_OTHER, "other"},
};
ENUMSTRINGS(ssr::enum_audio_codec) = {
    {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS, "vorbis"},
    {ssr::enum_audio_codec::AUDIO_CODEC_MP3, "mp3"},
    {ssr::enum_audio_codec::AUDIO_CODEC_AAC, "aac"},
    {ssr::enum_audio_codec::AUDIO_CODEC_UNCOMPRESSED, "uncompressed"},
    {ssr::enum_audio_codec::AUDIO_CODEC_OTHER, "other"},
};

ssrtools::ssrtools(QWidget *parent, mypopup *myp) :
    QWidget(parent),
    ui(new Ui::ssrtools),
    mp(myp)
{
    ui->setupUi(this);
    this->hide();
    setStyle();

    Init();
//    mp = new mypopup(this, parent);
    mp->hide();
    OnUpdateVideoAreaFields();

    Prepare_Record();
}

ssrtools::~ssrtools()
{
    delete ui;
}

void ssrtools::setStyle()
{
    QString runPath = QCoreApplication::applicationDirPath();
    QFile file(runPath + "/qss/ssrtools.qss");
    qDebug() << "bybobbi: " << file.fileName();
    file.open(QFile::ReadOnly);
    QTextStream fileText(&file);
    QString stylesheet = fileText.readAll();
    this->setStyleSheet(stylesheet);
    file.close();
}

void ssrtools::Init()
{

    options_show = false;
    m_grabbing = false;
    m_selecting_window = false;

    Input_init();

    Output_init();

    LoadSettings();
}

void ssrtools::Input_init()
{
    //录制光标
    m_video_record_cursor = true;

    //跟随鼠标
    m_video_follow_mouse = false;

    m_video_area_follow_fullscreen = false; //fixed bybobbi

    //input
    m_buttongroup_video_area = new QButtonGroup;
    m_buttongroup_video_area->addButton(ui->m_radioButton_fullscreen, int(ssr::enum_video_area::VIDEO_AREA_SCREEN));
    m_buttongroup_video_area->addButton(ui->m_radioButton_fixed, int(ssr::enum_video_area::VIDEO_AREA_FIXED));
    m_buttongroup_video_area->addButton(ui->m_radioButton_cursor, int(ssr::enum_video_area::VIDEO_AREA_CURSOR));

    ui->m_spinbox_video_x->setRange(0, SSR_MAX_IMAGE_SIZE);
    ui->m_spinbox_video_x->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->m_spinbox_video_y->setRange(0, SSR_MAX_IMAGE_SIZE);
    ui->m_spinbox_video_y->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->m_spinbox_video_w->setRange(0, SSR_MAX_IMAGE_SIZE);
    ui->m_spinbox_video_w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->m_spinbox_video_h->setRange(0, SSR_MAX_IMAGE_SIZE);
    ui->m_spinbox_video_h->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(m_buttongroup_video_area, SIGNAL(buttonClicked(int)), this, SLOT(OnUpdateVideoAreaFields()));
    connect(ui->m_spinbox_video_x, SIGNAL(focusIn()), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_x, SIGNAL(focusOut()), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_x, SIGNAL(valueChanged(int)), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_y, SIGNAL(focusIn()), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_y, SIGNAL(focusOut()), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_y, SIGNAL(valueChanged(int)), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_w, SIGNAL(focusIn()), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_w, SIGNAL(focusOut()), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_w, SIGNAL(valueChanged(int)), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_h, SIGNAL(focusIn()), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_h, SIGNAL(focusOut()), this, SLOT(OnUpdateRecordingFrame()));
    connect(ui->m_spinbox_video_h, SIGNAL(valueChanged(int)), this, SLOT(OnUpdateRecordingFrame()));


    m_combobox_pulseaudio_source = new QComboBox;
    LoadPulseAudioSources();
}

void ssrtools::Output_init()
{
//    m_containers = {
//        ContainerData({"Matroska (MKV)", "matroska", QStringList({"mkv"}), tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg("Matroska") + " (*.mkv)",
//            {ssr::enum_video_codec::VIDEO_CODEC_H264, ssr::enum_video_codec::VIDEO_CODEC_VP8, ssr::enum_video_codec::VIDEO_CODEC_THEORA},
//            {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS, ssr::enum_audio_codec::AUDIO_CODEC_MP3,ssr::enum_audio_codec:: AUDIO_CODEC_AAC, ssr::enum_audio_codec::AUDIO_CODEC_UNCOMPRESSED}}),
//        ContainerData({"MP4", "mp4", QStringList({"mp4"}), tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg("MP4") + " (*.mp4)",
//            {ssr::enum_video_codec::VIDEO_CODEC_H264},
//            {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS, ssr::enum_audio_codec::AUDIO_CODEC_MP3, ssr::enum_audio_codec::AUDIO_CODEC_AAC}}),
//        ContainerData({"WebM", "webm", QStringList({"webm"}), tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg("WebM") + " (*.webm)",
//            {ssr::enum_video_codec::VIDEO_CODEC_VP8},
//            {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS}}),
//        ContainerData({"OGG", "ogg", QStringList({"ogg"}), tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg("OGG") + " (*.ogg)",
//            {ssr::enum_video_codec::VIDEO_CODEC_THEORA},
//            {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS}}),
//        ContainerData({tr("Other..."), "other", QStringList(), "", std::set<ssr::enum_video_codec>({}), std::set<ssr::enum_audio_codec>({})}),
//    };

////    m_video_codecs = {
////        {"H.264"       , "libx264"  },
////        {"VP8"         , "libvpx"   },
////        {"Theora"      , "libtheora"},
////        {tr("Other..."), "other"    },
////    };
////    m_audio_codecs = {
////        {"Vorbis"          , "libvorbis"   },
////        {"MP3"             , "libmp3lame"  },
////        {"AAC"             , "libvo_aacenc"},
////        {tr("Uncompressed"), "pcm_s16le"   },
////        {tr("Other...")    , "other"       },
////    };
////    m_audio_kbit_rates = {
////        {"32"},
////        {"96"},
////        {"128"},
////        {"160"},
////        {"192"}
////    };

//    m_containers_av.clear();
//#if SSR_USE_AV_MUXER_ITERATE
//    const AVOutputFormat *format;
//    void *format_opaque = NULL;
//    while((format = av_muxer_iterate(&format_opaque)) != NULL) {
//#else
//    for(AVOutputFormat *format = av_oformat_next(NULL); format != NULL; format = av_oformat_next(format)) {
//#endif

//        if (format->video_codec == AV_CODEC_ID_NONE)
//            continue;
//        ContainerData c;
//        c.name = format->long_name;
//        c.avname = format->name;
//        c.suffixes = QString(format->extensions).split(',', QString::SkipEmptyParts);
//        if (c.suffixes.isEmpty()) {
//            c.filter = "";
//        } else {
//            c.filter = tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg(c.avname) + "(*." + c.suffixes[0];
//            for (int i = 1; i < c.suffixes.size(); ++i) {
//                c.suffixes[i] = c.suffixes[i].trimmed();// needed because libav/ffmpeg isn't very consistent when they say 'comma-separated'
//                c.filter += "*." + c.suffixes[i];
//            }
//            c.filter += ")";
//        }
//        m_containers_av.push_back(c);
//    }
//    std::sort(m_containers_av.begin(), m_containers_av.end());

//    //load AV codec list
//    m_video_codecs_av.clear();
//    m_audio_codecs_av.clear();
//#if SSR_USE_AV_MUXER_ITERATE
//    const AVCodec *codec;
//    void *codec_opaque = NULL;
//    while((codec = av_codec_iterate(&codec_opaque)) != NULL) {
//#else
//    for(AVCodec *codec = av_codec_next(NULL); codec != NULL; codec = av_codec_next(codec)) {
//#endif
//        if(!av_codec_is_encoder(codec))
//            continue;
//        if(codec->type == AVMEDIA_TYPE_VIDEO && VideoEncoder::AVCodecIsSupported(codec->name)) {
//            VideoCodecData c;
//            c.name = codec->long_name;
//            c.avname = codec->name;
//            m_video_codecs_av.push_back(c);
//        }
//        if(codec->type == AVMEDIA_TYPE_AUDIO && AudioEncoder::AVCodecIsSupported(codec->name)) {
//            AudioCodecData c;
//            c.name = codec->long_name;
//            c.avname = codec->name;
//            m_audio_codecs_av.push_back(c);
//        }
//    }
//    std::sort(m_video_codecs_av.begin(), m_video_codecs_av.end());
//    std::sort(m_audio_codecs_av.begin(), m_audio_codecs_av.end());
//    if(m_containers_av.empty()) {
//        Logger::LogError("[OutputInit] " + tr("Error: Could not find any suitable container in libavformat!"));
//        throw LibavException();
//    }
//    if(m_video_codecs_av.empty()) {
//        Logger::LogError("[OutputInit] " + tr("Error: Could not find any suitable video codec in libavcodec!"));
//        throw LibavException();
//    }
//    if(m_audio_codecs_av.empty()) {
//        Logger::LogError("[OutputInit] " + tr("Error: Could not find any suitable audio codec in libavcodec!"));
//        throw LibavException();
//    }

//    m_lineedit_audio_options_not_shown = new QLineEdit;

}

void ssrtools::Prepare_Record()
{
    //gui init
    m_timer_update_info = new QTimer(this);
    connect(m_timer_update_info, SIGNAL(timeout()), this, SLOT(OnUpdateInformation()));

    connect(ui->m_pushButton_cancel, SIGNAL(clicked()), this, SLOT(OnRecordCancel()));
    connect(ui->m_pushButton_save, SIGNAL(clicked()), this, SLOT(OnRecordSave()));
    GroupVisible({ui->m_pushButton_cancel, ui->m_pushButton_save}, false);

    //record init
    m_output_started =false;
    m_input_started = false;
    m_schedule_active = false;
    m_recorded_something = false;

    //get the audio input settings
    m_audio_enabled =  true;
    m_audio_channels = 2;
    m_audio_sample_rate = 48000;


    //bybobbi
    m_video_frame_rate = 30;
    m_video_in_width = GetVideoW();
    m_video_in_height = GetVideoH();

#if SSR_USE_PULSEAUDIO
    m_pulseaudio_source = GetPulseAudioSourceName();
#endif

    m_separate_files = true;

    //get the output settings
    m_output_settings.file = QString();	//will be set later
    m_output_settings.container_avname  = mp->GetContainerAVName();
    m_output_settings.video_codec_avname = mp->GetVideoCodecAVName();
//    m_output_settings.video_kbit_rate = mp->GetVideoKBitRate();
    m_output_settings.video_kbit_rate = 5000;
    m_output_settings.video_options.clear();
    m_output_settings.video_width = 0;
    m_output_settings.video_height = 0;
    m_output_settings.video_frame_rate = m_video_frame_rate;
//    m_output_settings.video_allow_frame_skipping = mp->GetVideoAllowFrameSkipping();
    m_output_settings.video_allow_frame_skipping = true;

    m_output_settings.audio_codec_avname = (m_audio_enabled)? mp->GetAudioCodecAVName() : QString();
//    m_output_settings.audio_kbit_rate = mp->GetAudioKBitRate();
    m_output_settings.audio_kbit_rate = 128;
    m_output_settings.audio_options.clear();
    m_output_settings.audio_channels = m_audio_channels;
    m_output_settings.audio_sample_rate = m_audio_sample_rate;


//    m_output_settings.video_options.push_back(std::make_pair(QString("crf"), QString::number(GetH264CRF())));
//    m_output_settings.video_options.push_back(std::make_pair(QString("preset"), EnumToString(GetH264Preset())));
    m_output_settings.video_options.push_back(std::make_pair(QString("crf"), "23"));
    m_output_settings.video_options.push_back(std::make_pair(QString("preset"), "superfast"));

//    switch(mp->GetAudioCodec()) {
//        case ssr::enum_audio_codec::AUDIO_CODEC_OTHER: {
//            m_output_settings.audio_options = GetOptionsFromString(mp->GetAudioOptions());
//            break;
//        }
//        default: break; // to keep GCC happy
//    }


    Logger::LogInfo("[PageRecord::StartPage] " + tr("Starting page ..."));

    Logger::LogInfo("[PageRecord::StartPage] " + tr("Started page."));


    m_recorded_something = false;


    UpdateInput();

    OnUpdateInformation();
    UpdateRecordButton();
    m_timer_update_info->start(1000);

    m_schedule_active = false;
    UpdateSchedule();

}

void ssrtools::LoadSettings()
{
    QSettings settings(CommandLineOptions::GetSettingsFile(), QSettings::IniFormat);

    LoadInputSettings(&settings);
    LoadOutputSettings(&settings);

}

void ssrtools::LoadInputSettings(QSettings *settings)
{
    LoadInputProfileSettings(settings);
}

void ssrtools::LoadOutputSettings(QSettings *settings)
{
    LoadOutputProfileSettings(settings);
}

void ssrtools::StopPage(bool save)
{
    m_schedule_active = false;
    UpdateSchedule();

    StopOutput(true);
    StopInput();

    Logger::LogInfo("[PageRecord::StopPage] " + tr("Stopping page ..."));

    if(m_output_manager != NULL) {

        // stop the output
        if(save)
            FinishOutput();
        m_output_manager.reset();

        // delete the file if it isn't needed
        if(!save && m_file_protocol.isNull()) {
            if(QFileInfo(m_output_settings.file).exists())
                QFile(m_output_settings.file).remove();
        }

    }

#if SSR_USE_OPENGL_RECORDING
    // stop GLInject input
    m_gl_inject_input.reset();
#endif

#if SSR_USE_JACK
    // stop JACK input
    m_jack_input.reset();
#endif

    Logger::LogInfo("[PageRecord::StopPage] " + tr("Stopped page."));


    m_timer_update_info->stop();
    OnUpdateInformation();

}

void ssrtools::StartOutput()
{
    if (m_output_started) return;
    try {
        Logger::LogInfo("[Begin Record: StartOutput]" + tr("Starting output ..."));

        if (m_output_manager == NULL) {
            //set the file name
            m_output_settings.file = GetNewSegmentFile(m_file_base, m_add_timestamp);
            qDebug() << "[record debug] m_output_settings.file: "  << m_output_settings.file;

            //for X11 recording, update the video size (if possible)
            if (m_x11_input != NULL) {
                m_x11_input->GetCurrentSize(&m_video_in_width, &m_video_in_height);
            }

            // calculate the output width and height
//            if(m_video_scaling) {
                // Only even width and height is allowed because some pixel formats (e.g. YUV420) require this.
//                m_output_settings.video_width = m_video_scaled_width / 2 * 2;
//                m_output_settings.video_height = m_video_scaled_height / 2 * 2;
//            } else {
                // If the user did not explicitly select scaling, then don't force scaling just because the recording area is one pixel too large.
                // One missing row/column of pixels is probably better than a blurry video (and scaling is SLOW).
            m_video_in_width = m_video_in_width / 2 * 2;
            m_video_in_height = m_video_in_height / 2 * 2;
            m_output_settings.video_width = m_video_in_width;
            m_output_settings.video_height = m_video_in_height;
//            }

        //start the output
            m_output_manager.reset(new OutputManager(m_output_settings));

        } else {

            // start a new segment
            m_output_manager->GetSynchronizer()->NewSegment();
        }

        Logger::LogInfo("[PageRecord::StartOutput] " + tr("Started output."));

        m_output_started = true;
        m_recorded_something = true;
//        UpdateSysTray();
//        UpdateRecordButton();
        UpdateInput();
        UpdateRecordButton();

    } catch(...) {
        Logger::LogError("[PageRecord::StartOutput] " + tr("Error: Something went wrong during initialization."));
    }
}

void ssrtools::StopOutput(bool final)
{
//    assert(m_page_started);

    if(!m_output_started)
        return;

    Logger::LogInfo("[PageRecord::StopOutput] " + tr("Stopping output ..."));

    // if final, then StopPage will stop the output (and delete the file if needed)
//    if(m_separate_files && !final) {
    if(m_separate_files && !final) {

        // stop the output
        FinishOutput();
        m_output_manager.reset();

        // change the file name
        m_output_settings.file = QString();

        // reset the output video size
        m_output_settings.video_width = 0;
        m_output_settings.video_height = 0;

    }

    Logger::LogInfo("[PageRecord::StopOutput] " + tr("Stopped output."));

#if SSR_USE_ALSA
    // if final, don't play the notification (it would get interrupted anyway)
    if(m_simple_synth != NULL && !final)
        m_simple_synth->PlaySequence(SEQUENCE_RECORD_STOP.data(), SEQUENCE_RECORD_STOP.size());
#endif

    m_output_started = false;
//    UpdateSysTray();
    UpdateRecordButton();
    UpdateInput();
}

void ssrtools::StartInput()
{
   if (m_input_started) return;

   assert(m_x11_input == NULL);
#if SSR_USE_PULSEAUDIO
    assert(m_pulseaudio_input == NULL);
#endif

    try {
        Logger::LogInfo("[PageRecord::StartInput] " + tr("Starting input ..."));

        // start the video input
        m_x11_input.reset(new X11Input(m_video_x, m_video_y, m_video_in_width, m_video_in_height, m_video_record_cursor,
                                       m_video_record_cursor, m_video_area_follow_fullscreen));
//                                       m_video_area == ssr::enum_video_area::VIDEO_AREA_CURSOR, m_video_area_follow_fullscreen));

        //start the audio input
#if SSR_USE_PULSEAUDIO
//            if(m_audio_backend == ssr::enum_audio_backend::AUDIO_BACKEND_PULSEAUDIO)
                m_pulseaudio_input.reset(new PulseAudioInput(m_pulseaudio_source, m_audio_sample_rate));
#endif

        Logger::LogInfo("[PageRecord::StartInput] " + tr("Started input."));

        m_input_started = true;
    } catch (...) {
        Logger::LogError("[PageRecord::StartInput] " + tr("Error: Something went wrong during initialization."));
        m_x11_input.reset();
#if SSR_USE_PULSEAUDIO
        m_pulseaudio_input.reset();
#endif
        return;
    }
}

void ssrtools::StopInput()
{
    if(!m_input_started)
        return;

    Logger::LogInfo("[PageRecord::StopInput] " + tr("Stopping input ..."));

    m_x11_input.reset();
#if SSR_USE_PULSEAUDIO
    m_pulseaudio_input.reset();
#endif

    Logger::LogInfo("[PageRecord::StopInput] " + tr("Stopped input."));

    m_input_started = false;
}


void ssrtools::on_m_toolButton_options_clicked()
{
   QRect og = ui->m_toolButton_options->geometry();
   QPoint pos = this->pos();
   mp->move(og.x() + pos.x(), og.y() + pos.y() + 250);
   if (!options_show) {
       options_show = true;
       mp->show();
   } else {
       options_show = false;
       mp->hide();
   }
}


void ssrtools::on_m_pushButton_start_clicked()
{
//    if(m_page_started && m_output_started) {
    static bool cancel_save_show = false;
    if (!cancel_save_show) {
        GroupVisible({ui->m_pushButton_cancel, ui->m_pushButton_save}, true);
        cancel_save_show = true;
    }
    if(m_output_started) {
        OnRecordPause();
    } else {
        OnRecordStart();
    }
}



void ssrtools::OnUpdateRecordingFrame()
{
    if(ui->m_spinbox_video_x->hasFocus() || ui->m_spinbox_video_y->hasFocus() || ui->m_spinbox_video_w->hasFocus() || ui->m_spinbox_video_h->hasFocus()) {
        if(m_recording_frame == NULL)
            m_recording_frame.reset(new RecordingFrameWindow(this));
        QRect rect = MapToLogicalCoordinates(ValidateRubberBandRectangle(QRect(GetVideoX(), GetVideoY(), GetVideoW(), GetVideoH())));
        if(rect.isNull()) {
            m_recording_frame->hide();
        } else {
            m_recording_frame->setGeometry(rect);
            m_recording_frame->show();
        }
    } else {
        m_recording_frame.reset();
    }
}

void ssrtools::OnUpdateVideoAreaFields()
{
    switch(GetVideoArea()) {
    case ssr::enum_video_area::VIDEO_AREA_SCREEN: {
        GroupVisible({ui->m_pushbutton_video_select_rectangle, ui->m_pushbutton_video_select_window}, false);
        int sc;
//        mp->OnUpdateVideoAreaFields(sc);
        std::vector<QRect> screen_geometries = GetScreenGeometries();
        QRect rect;
        if (sc > 0 && sc <= (int)screen_geometries.size()) {
            rect = screen_geometries[sc - 1];
        } else {
            rect = CombineScreenGeometries(screen_geometries);
        }
        SetVideoX(rect.left());
        SetVideoY(rect.top());
        SetVideoW(rect.width());
        SetVideoH(rect.height());
        break;
    }
    case ssr::enum_video_area::VIDEO_AREA_FIXED: {
        GroupVisible({ui->m_pushbutton_video_select_rectangle, ui->m_pushbutton_video_select_window}, true);
        break;
    }
    case ssr::enum_video_area::VIDEO_AREA_CURSOR: {
        GroupVisible({ui->m_pushbutton_video_select_rectangle, ui->m_pushbutton_video_select_window}, true);
        break;
    }
    default:
        break;
    }
    //    mp->OnUpdateVideoAreaFields();
}
void ssrtools::OnUpdateVideoAreaFields_(QRect &rect)
{
    SetVideoX(rect.left());
    SetVideoY(rect.top());
    SetVideoW(rect.width());
    SetVideoH(rect.height());
}

void ssrtools::OnRecordCancel()
{
    StopPage(false);
}

void ssrtools::OnRecordSave()
{
    StopPage(true);
}

void ssrtools::OnScheduleTimer()
{
/*    if(!m_page_started)
        return;
    if(m_schedule_active) {
        QDateTime now = QDateTime::currentDateTimeUtc();
        while(m_schedule_position < m_schedule_entries.size()) {
            ScheduleEntry &entry = m_schedule_entries[m_schedule_position];
            if(now < entry.time)
                break;
            Logger::LogInfo("[PageRecord::OnScheduleTimer] " + tr("Triggering scheduled action '%1' ...").arg(EnumToString(entry.action)));
            switch(entry.action) {
                case SCHEDULE_ACTION_START: OnRecordStart(); break;
                case SCHEDULE_ACTION_PAUSE: OnRecordPause(); break;
                default: break; // to keep GCC happy
            }
            ++m_schedule_position;
        }
        if(m_schedule_position < m_schedule_entries.size()) {
            ScheduleEntry &entry = m_schedule_entries[m_schedule_position];
            int64_t msec = now.msecsTo(entry.time);
            m_label_schedule_status->setText(tr("Schedule: %1 in %2").arg(SCHEDULE_ACTION_TEXT[entry.action]).arg(ReadableTime(msec * 1000)));
            if(msec < 1000) {
                m_timer_schedule->start(msec);
            } else {
                m_timer_schedule->start((msec - 100) % 1000 + 100);
            }
        } else {
            m_label_schedule_status->setText(tr("Schedule: (none)"));
            m_timer_schedule->stop();
        }
    } else {
        m_label_schedule_status->setText(tr("Schedule: (inactive)"))*/;
//        m_timer_schedule->stop();
    //    }
}


void ssrtools::mousePressEvent(QMouseEvent *event)
{
    if (m_grabbing) {
        if (event->button() == Qt::LeftButton) {
            if (IsPlatformX11()) {
                QPoint mouse_physical = GetMousePhysicalCoordinates();
                if (m_selecting_window) {
                    // As expected, Qt does not provide any functions to find the window at a specific position, so I have to use Xlib directly.
                    // I'm not completely sure whether this is the best way to do this, but it appears to work. XQueryPointer returns the window
                    // currently below the mouse along with the mouse position, but apparently this may not work correctly when the mouse pointer
                    // is also grabbed (even though it works fine in my test), so I use XTranslateCoordinates instead. Originally I wanted to
                    // show the rubber band when the mouse hovers over a window (instead of having to click it), but this doesn't work correctly
                    // since X will simply return a handle the rubber band itself (even though it should be transparent to mouse events).
                    Window selected_window;
                    int x, y;
                    if (XTranslateCoordinates(QX11Info::display(), QX11Info::appRootWindow(), QX11Info::appRootWindow(), mouse_physical.x(), mouse_physical.y(), &x, &y, &selected_window)) {
                        XWindowAttributes attributes;
                        if (selected_window != None && XGetWindowAttributes(QX11Info::display(), selected_window, &attributes)) {
                            // naive outer/inner rectangle, this won't work for window decorations
                            m_select_window_outer_rect = QRect(attributes.x, attributes.y, attributes.width + 2 * attributes.border_width, attributes.height + 2 * attributes.border_width);
                            m_select_window_inner_rect = QRect(attributes.x + attributes.border_width, attributes.y + attributes.border_width, attributes.width, attributes.height);

                            // try to find the real window (rather than the decorations added by the window manager)
                            Window real_window = X11FindRealWindow(QX11Info::display(), selected_window);
                            if (real_window != None) {
                                Atom actual_type;
                                int actual_format;
                                unsigned long items, bytes_left;
                                long *data = NULL;
                                int result = XGetWindowProperty(QX11Info::display(), real_window, XInternAtom(QX11Info::display(), "_NET_FRAME_EXTENTS", true),
                                                                 0, 4, false, AnyPropertyType, &actual_type, &actual_format, &items, &bytes_left, (unsigned char**)&data);
                                if (result == Success) {
                                    if (items == 4 && bytes_left == 0 && actual_format == 32) { // format 32 means 'long', even if long is 64-bit ...
                                        Window child;
                                        // the attributes of the real window only store the *relative* position which is not what we need, so use XTranslateCoordinates again
                                        if (XTranslateCoordinates(QX11Info::display(), real_window, QX11Info::appRootWindow(), 0, 0, &x, &y, &child)
                                                && XGetWindowAttributes(QX11Info::display(), real_window, &attributes)) {
                                            //finally
                                            m_select_window_inner_rect = QRect(x, y, attributes.width, attributes.height);
                                            m_select_window_outer_rect = m_select_window_inner_rect.adjusted(-data[0], -data[2], data[1], data[3]);
                                        } else {
                                            // I doubt this will ever be needed, but do it anyway
                                            m_select_window_inner_rect = m_select_window_outer_rect.adjusted(data[0], data[2], -data[1], -data[3]);
                                        }
                                    }
                                }
                                if (data != NULL)
                                    XFree(data);
                            }

                             // pick the inner rectangle if the users clicks inside the window, or the outer rectangle otherwise
                            m_rubber_band_rect = (m_select_window_inner_rect.contains(mouse_physical)) ? m_select_window_inner_rect : m_select_window_outer_rect;
                            UpdateRubberBand();
                        }
                    }
                } else {
                    m_rubber_band_rect = QRect(mouse_physical, mouse_physical);
                    UpdateRubberBand();
                }

            }
        } else {
            StopGrabbing();
        }
        event->accept();
        return;
    }
    event->ignore();
}

void ssrtools::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_grabbing) {
        if(event->button() == Qt::LeftButton) {
            if(m_rubber_band != NULL) {
                SetVideoAreaFromRubberBand();
            }
        }
        StopGrabbing();
        event->accept();
        return;
    }
    event->ignore();
}

void ssrtools::mouseMoveEvent(QMouseEvent *event)
{
    if(m_grabbing) {
        if(m_rubber_band != NULL && IsPlatformX11()) {
            QPoint mouse_physical = GetMousePhysicalCoordinates();
            if(m_selecting_window) {
                // pick the inner rectangle if the user clicks inside the window, or the outer rectangle otherwise
                m_rubber_band_rect = (m_select_window_inner_rect.contains(mouse_physical))? m_select_window_inner_rect : m_select_window_outer_rect;
            } else {
                m_rubber_band_rect.setBottomRight(mouse_physical);
            }
            UpdateRubberBand();
        }
        event->accept();
        return;
    }
    event->ignore();
}

void ssrtools::keyPressEvent(QKeyEvent *event)
{
    if(m_grabbing) {
        if(event->key() == Qt::Key_Escape) {
            StopGrabbing();
            return;
        }
        event->accept();
        return;
    }
    event->ignore();
}

void ssrtools::SetVideoArea(ssr::enum_video_area area)
{
    QAbstractButton *b = m_buttongroup_video_area->button(int(area));
    if(b != NULL) b->setChecked(true);
}

//void ssrtools::SetVideoAreaScreen(unsigned int screen)
//{
//    ui->m_combobox_frate->setCurrentIndex(clamp(screen, 0u, (unsigned int) m_combobox_screens->count() - 1));
//}

void ssrtools::SetVideoX(unsigned int x)
{
    ui->m_spinbox_video_x->setValue(x);
}

void ssrtools::SetVideoY(unsigned int y)
{
    ui->m_spinbox_video_y->setValue(y);
}

void ssrtools::SetVideoW(unsigned int w)
{
    ui->m_spinbox_video_w->setValue(w);
}

void ssrtools::SetVideoH(unsigned int h)
{
    ui->m_spinbox_video_h->setValue(h);
}

unsigned int ssrtools::GetVideoX()
{
    return ui->m_spinbox_video_x->value();
}

unsigned int ssrtools::GetVideoY()
{
    return ui->m_spinbox_video_y->value();
}

unsigned int ssrtools::GetVideoW()
{
    return ui->m_spinbox_video_w->value();
}

unsigned int ssrtools::GetVideoH()
{
    return ui->m_spinbox_video_h->value();
}

void ssrtools::on_m_pushbutton_video_select_rectangle_clicked()
{
    m_selecting_window = false;
    StartGrabbing();
}

void ssrtools::on_m_pushbutton_video_select_window_clicked()
{
    m_selecting_window = true;
    StartGrabbing();
}

void ssrtools::OnUpdateInformation()
{
    int64_t total_time = 0;
    double fps_in = 0.0;
    double fps_out = 0.0;
    uint64_t bit_rate = 0, total_bytes = 0;

#if SSR_USE_OPENGL_RECORDING
    if(m_gl_inject_input != NULL)
        fps_in = m_gl_inject_input->GetFPS();
#endif
    if(m_x11_input != NULL)
        fps_in = m_x11_input->GetFPS();

    if(m_output_manager != NULL) {
        total_time = (m_output_manager->GetSynchronizer() == NULL)? 0 : m_output_manager->GetSynchronizer()->GetTotalTime();
        fps_out = m_output_manager->GetActualFrameRate();
        bit_rate = (uint64_t) (m_output_manager->GetActualBitRate() + 0.5);
        total_bytes = m_output_manager->GetTotalBytes();
    }

    QString file_name;
    if(m_file_protocol.isNull())
        file_name = (m_output_settings.file.isNull())? "?" : QFileInfo(m_output_settings.file).fileName();
    else
        file_name = "(" + m_file_protocol + ")";

    // for X11 recording, update the video size
    if(m_x11_input != NULL)
        m_x11_input->GetCurrentSize(&m_video_in_width, &m_video_in_height);

#if SSR_USE_OPENGL_RECORDING
    // for OpenGL recording, update the video size
    if(m_gl_inject_input != NULL)
        m_gl_inject_input->GetCurrentSize(&m_video_in_width, &m_video_in_height);
#endif

    ui->m_label_info_total_time->setText(ReadableTime(total_time));
    ui->m_label_info_frame_rate_in->setText(QString::number(fps_in, 'f', 2));
    ui->m_label_info_frame_rate_out->setText(QString::number(fps_out, 'f', 2));
    ui->m_label_info_size_in->setText(ReadableWidthHeight(m_video_in_width, m_video_in_height));
    ui->m_label_info_size_out->setText(ReadableWidthHeight(m_output_settings.video_width, m_output_settings.video_height));
    ui->m_label_info_file_name->setText(file_name);
    ui->m_label_info_file_size->setText(ReadableSizeIEC(total_bytes, "B"));
    ui->m_label_info_bit_rate->setText(ReadableSizeSI(bit_rate, "bit/s"));

    if(!CommandLineOptions::GetStatsFile().isNull()) {
        QString str = QString() +
                "capturing\t" + ((m_input_started)? "1" : "0") + "\n"
                "recording\t" + ((m_output_started)? "1" : "0") + "\n"
                "total_time\t" + QString::number(total_time) + "\n"
                "frame_rate_in\t" + QString::number(fps_in, 'f', 8) + "\n"
                "frame_rate_out\t" + QString::number(fps_out, 'f', 8) + "\n"
                "size_in_width\t" + QString::number(m_video_in_width) + "\n"
                "size_in_height\t" + QString::number(m_video_in_height) + "\n"
                "size_out_width\t" + QString::number(m_output_settings.video_width) + "\n"
                "size_out_height\t" + QString::number(m_output_settings.video_height) + "\n"
                "file_name\t" + file_name + "\n"
                "file_size\t" + QString::number(total_bytes) + "\n"
                "bit_rate\t" + QString::number(bit_rate) + "\n";
        QByteArray data = str.toUtf8();
        QByteArray old_file = QFile::encodeName(CommandLineOptions::GetStatsFile());
        QByteArray new_file = QFile::encodeName(CommandLineOptions::GetStatsFile() + "-new");
        // Qt doesn't get the permissions right (you can only change the permissions after creating the file, that's too late),
        // and it doesn't allow renaming a file over another file, so don't bother with QFile and just use POSIX and C functions.
        int fd = open(new_file.constData(), O_WRONLY | O_CREAT | O_CLOEXEC, 0600);
        if(fd != -1) {
            ssize_t b = write(fd, data.constData(), data.size()); Q_UNUSED(b);
            ::close(fd);
            rename(new_file.constData(), old_file.constData());
        }
    }
}

void ssrtools::LoadInputProfileSettings(QSettings *settings)
{
    //load settings
#if 0
    SetVideoArea(StringToEnum(settings->value("input/video_area", QString()).toString(), VIDEO_AREA_SCREEN));
#else
    ssr::enum_video_area area = StringToEnum(settings->value("input/video_area", QString()).toString(), ssr::enum_video_area::VIDEO_AREA_SCREEN);
    SetVideoArea(area);
#endif
//    SetVideoAreaScreen(settings->value("input/video_area_screen", 0).toUInt());

    SetVideoX(settings->value("input/video_x", 0).toUInt());
    SetVideoY(settings->value("input/video_y", 0).toUInt());
    SetVideoW(settings->value("input/video_w", 800).toUInt());
    SetVideoH(settings->value("input/video_h", 600).toUInt());
//    SetVideoFrameRate(settings->value("input/video_frame_rate", 30).toUInt());

    //update things
    OnUpdateRecordingFrame();

}

void ssrtools::LoadOutputProfileSettings(QSettings *settings)
{
    // choose default container and codecs
    ssr::enum_container default_container = (ssr::enum_container) 0;
    auto containers = mp->GetContainers();
    for(unsigned int i = 0; i < int(ssr::enum_container::CONTAINER_OTHER); ++i) {
        if(AVFormatIsInstalled(containers[i].avname)) {
            default_container = (ssr::enum_container) i;
            break;
        }
    }
//    ssr::enum_video_codec default_video_codec = (ssr::enum_video_codec) 0;
//    for(unsigned int i = 0; i < int(ssr::enum_video_codec::VIDEO_CODEC_OTHER); ++i) {
//        if(AVCodecIsInstalled(m_video_codecs[i].avname) && m_containers[int(default_container)].supported_video_codecs.count((ssr::enum_video_codec) i)) {
//            default_video_codec = (ssr::enum_video_codec) i;
//            break;
//        }
//    }
//    ssr::enum_audio_codec default_audio_codec = (ssr::enum_audio_codec) 0;
//    for(unsigned int i = 0; i < int(ssr::enum_video_codec::VIDEO_CODEC_OTHER); ++i) {
//        if(AVCodecIsInstalled(m_audio_codecs[i].avname) && m_containers[int(default_container)].supported_audio_codecs.count((ssr::enum_audio_codec) i)) {
//            default_audio_codec = (ssr::enum_audio_codec) i;
//            break;
//        }
//    }

    // choose default file name
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString dir_videos = QDesktopServices::storageLocation(QDesktopServices::MoviesLocation);
    QString dir_documents = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#else
    QString dir_videos = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    QString dir_documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
    QString dir_home = QDir::homePath();
    QString best_dir = (QDir(dir_videos).exists())? dir_videos : (QDir(dir_documents).exists())? dir_documents : dir_home;
    QString default_file = best_dir + "/simplescreenrecorder." + containers[int(default_container)].suffixes[0];

}

void ssrtools::OnRecordPause()
{
    StopOutput(false);
}

void ssrtools::OnRecordStart()
{
    m_file_base = mp->GetFile();
    m_video_x = GetVideoX();
    m_video_y = GetVideoY();
    m_video_in_width = GetVideoW();
    m_video_in_height = GetVideoH();

    StartOutput();
}

void ssrtools::RecordStartOrPause()
{
    //    if(m_page_started && m_output_started) {
//        static bool cancel_save_show = false;
//        if (!cancel_save_show) {
//            GroupVisible({ui->m_pushButton_cancel, ui->m_pushButton_save}, true);
//            cancel_save_show = true;
//        }
        if(m_output_started) {

            OnRecordPause();
        } else {
//            Logger::LogInfo("[Recording] " + tr("will Start Record!"));
            syslog(LOG_INFO, "bybobbi: Will Start Record!");
            OnRecordStart();
        }
}

#if SSR_USE_PULSEAUDIO
QString ssrtools::GetPulseAudioSourceName() {
    return QString::fromStdString(m_pulseaudio_sources[GetPulseAudioSource()].m_name);
}
#endif




void ssrtools::StartGrabbing()
{
    m_grabbing = true;
    if (m_selecting_window) {
        ui->m_pushbutton_video_select_window->setDown(true);
    } else {
        ui->m_pushbutton_video_select_rectangle->setDown(true);
    }
    this->lower();
    grabMouse(Qt::CrossCursor);
    grabKeyboard();
    setMouseTracking(true);
}

void ssrtools::StopGrabbing()
{
    m_rubber_band.reset();
    setMouseTracking(false);
    releaseKeyboard();
    releaseMouse();
    this->raise();
    this->activateWindow();
    if(m_selecting_window)
        ui->m_pushbutton_video_select_window->setDown(false);
    else
        ui->m_pushbutton_video_select_rectangle->setDown(false);
    m_grabbing = false;
}

void ssrtools::UpdateRubberBand()
{
    if(m_rubber_band == NULL)
        m_rubber_band.reset(new RecordingFrameWindow(this));
    QRect rect = MapToLogicalCoordinates(ValidateRubberBandRectangle(m_rubber_band_rect));
    if(rect.isNull()) {
        m_rubber_band->hide();
    } else {
        m_rubber_band->setGeometry(rect);
        m_rubber_band->show();
    }
}

void ssrtools::SetVideoAreaFromRubberBand()
{
    QRect r = m_rubber_band_rect.normalized();
    if(GetVideoArea() == ssr::enum_video_area::VIDEO_AREA_CURSOR) {
        SetVideoX(0);
        SetVideoY(0);
    } else {
        SetVideoX(r.x());
        SetVideoY(r.y());
    }
    SetVideoW(r.width());
    SetVideoH(r.height());
}

#if SSR_USE_PULSEAUDIO
void ssrtools::LoadPulseAudioSources() {
    m_pulseaudio_sources = PulseAudioInput::GetSourceList();
    if(m_pulseaudio_sources.empty()) {
        m_pulseaudio_available = false;
        m_pulseaudio_sources.push_back(PulseAudioInput::Source("", "(no sources found)"));
    } else {
        m_pulseaudio_available = true;
    }
    m_combobox_pulseaudio_source->clear();
    for(unsigned int i = 0; i < m_pulseaudio_sources.size(); ++i) {
        QString elided = m_combobox_pulseaudio_source->fontMetrics().elidedText(QString::fromStdString(m_pulseaudio_sources[i].m_description), Qt::ElideMiddle, 400);
        m_combobox_pulseaudio_source->addItem("\u200e" + elided + "\u200e");
    }
}
#endif

void ssrtools::FinishOutput()
{
    assert(m_output_manager != NULL);

    // tell the output manager to finish
    m_output_manager->Finish();

    // wait until it has actually finished
//    m_wait_saving = true;
    //unsigned int frames_left = m_output_manager->GetVideoEncoder()->GetFrameLatency();
    unsigned int frames_done = 0, frames_total = 0;
    QProgressDialog dialog(tr("Encoding remaining data ..."), QString(), 0, frames_total, this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setCancelButton(NULL);
    dialog.setMinimumDuration(500);
    while(!m_output_manager->isFinished()) {
        unsigned int frames = m_output_manager->GetTotalQueuedFrameCount();
        if(frames > frames_total)
            frames_total = frames;
        if(frames_total - frames > frames_done)
            frames_done = frames_total - frames;
        //qDebug() << "frames_done" << frames_done << "frames_total" << frames_total << "frames" << frames;
        dialog.setMaximum(frames_total);
        dialog.setValue(frames_done);
        usleep(20000);
    }
//    m_wait_saving = false;

}

void ssrtools::UpdateInput()
{
    if (m_output_started) {	//m_previewing
        StartInput();
    } else {
        StopInput();
    }

    //get source
    VideoSource *video_source = NULL;
    AudioSource *audio_source = NULL;
#if SSR_USE_OPENGL_RECORDING
    if(m_video_area == PageInput::VIDEO_AREA_GLINJECT) {
        video_source = m_gl_inject_input.get();
    } else {
#else
    {
#endif
        video_source = m_x11_input.get();
    }
    if(m_audio_enabled) {
#if SSR_USE_ALSA
        if(m_audio_backend == PageInput::AUDIO_BACKEND_ALSA)
            audio_source = m_alsa_input.get();
#endif
#if SSR_USE_PULSEAUDIO
//        if(m_audio_backend == ssr::enum_audio_backend::AUDIO_BACKEND_PULSEAUDIO)
            audio_source = m_pulseaudio_input.get();
#endif
#if SSR_USE_JACK
        if(m_audio_backend == PageInput::AUDIO_BACKEND_JACK)
            audio_source = m_jack_input.get();
#endif
    }

    // connect sinks
    if(m_output_manager != NULL) {
        if(m_output_started) {
            m_output_manager->GetSynchronizer()->ConnectVideoSource(video_source, PRIORITY_RECORD);
            m_output_manager->GetSynchronizer()->ConnectAudioSource(audio_source, PRIORITY_RECORD);
        } else {
            m_output_manager->GetSynchronizer()->ConnectVideoSource(NULL);
            m_output_manager->GetSynchronizer()->ConnectAudioSource(NULL);
        }
    }
//    if(m_previewing) {
//        m_video_previewer->ConnectVideoSource(video_source, PRIORITY_PREVIEW);
//        m_audio_previewer->ConnectAudioSource(audio_source, PRIORITY_PREVIEW);
//    } else {
//        m_video_previewer->ConnectVideoSource(NULL);
//        m_audio_previewer->ConnectAudioSource(NULL);
//    }

}

void ssrtools::UpdateRecordButton()
{
    if(m_output_started) {
//        ui->m_pushButton_start->setIcon(g_icon_pause);
        ui->m_pushButton_start->setText(tr("Pause recording"));
    } else {
//        ui->m_pushButton_start->setIcon(g_icon_record);
        ui->m_pushButton_start->setText(tr("Start recording"));
    }
}

void ssrtools::UpdateSchedule()
{
//    if(!m_page_started)
//        return;
//    if(m_schedule_active) {
//        m_pushbutton_schedule_activate->setText(tr("Deactivate schedule"));
//        m_schedule_position = 0;
//        QDateTime now = QDateTime::currentDateTimeUtc();
//        while(m_schedule_position < m_schedule_entries.size()) {
//            ScheduleEntry &entry = m_schedule_entries[m_schedule_position];
//            if(now < entry.time)
//                break;
//            ++m_schedule_position;
//        }
//    } else {
//        m_pushbutton_schedule_activate->setText(tr("Activate schedule"));
//    }
    OnScheduleTimer();
}
