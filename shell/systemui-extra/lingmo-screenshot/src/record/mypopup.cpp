#include "mypopup.h"
#include "ui_mypopup.h"
//#include "Options_Widgets.h"

//#include "WidgetWrapper.h"

#include <QFileDialog>
#include <QGroupBox>
#include "Logger.h"

#include "common/utils.h"

#include "common/EnumStrings.h"
#include "CommandLineOptions.h"
#include "ssrtools.h"
#include "../widgets/capture/capturewidget.h"

//mypopup::mypopup(ssrtools *ssr, QWidget *parent) :
//    QWidget(parent),
//    ui(new Ui::mypopup)
//{
//    ui->setupUi(this);
//    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint|Qt::WindowStaysOnTopHint);

//    this->ssr = static_cast<ssrtools*>(ssr);

//    InputInit();

//    OutputInit();

//}

static CaptureWidget *cw = nullptr;

mypopup::mypopup(QWidget *parent):
    QWidget(parent),
    ui(new Ui::mypopup)
{
    cw = dynamic_cast<CaptureWidget*>(parent);

    setCursor(Qt::ArrowCursor);
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint|Qt::WindowStaysOnTopHint);

    InputInit();

    OutputInit(parent);
}

mypopup::~mypopup()
{
    delete ui;
}

void mypopup::InputInit()
{

    connect(ui->m_comboBox_videores, SIGNAL(activated(int)), this, SLOT(OnUpdateVideoAreaFields()));
    connect(ui->m_comboBox_videores, SIGNAL(popupShown()), this, SLOT(OnIdentifyScreens()));
    connect(ui->m_comboBox_videores, SIGNAL(popupHidden()), this, SLOT(OnStopIdentifyScreens()));
    LoadScreenConfigurations();
}

void mypopup::OutputInit(QWidget *parent)
{

    m_old_container = (ssr::enum_container)0;
    m_old_container_av = 0;

//    connect(ui->m_pushButton_storelocation, SIGNAL(clicked()), this, SLOT(OnBrowse()));
    m_combobox_container_av_not_shown = new QComboBox;
    connect(ui->m_comboBox_container, SIGNAL(activated(int)), this, SLOT(OnUpdateSuffixAndContainerFields()));
#if 1
    connect(ui->m_pushButton_storelocation, SIGNAL(clicked()), this, SLOT(OnBrowse()));
#else
    CaptureWidget *cw = dynamic_cast<CaptureWidget*>(parent);
    connect(ui->m_pushButton_storelocation, SIGNAL(clicked()), cw, SLOT(ClickedSavedir()));
#endif

    m_lineedit_file_not_shown = new QLineEdit;

    m_containers = {
        ContainerData({"Matroska (MKV)", "matroska", QStringList({"mkv"}), tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg("Matroska") + " (*.mkv)",
            {ssr::enum_video_codec::VIDEO_CODEC_H264, ssr::enum_video_codec::VIDEO_CODEC_VP8, ssr::enum_video_codec::VIDEO_CODEC_THEORA},
            {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS, ssr::enum_audio_codec::AUDIO_CODEC_MP3,ssr::enum_audio_codec:: AUDIO_CODEC_AAC, ssr::enum_audio_codec::AUDIO_CODEC_UNCOMPRESSED}}),
        ContainerData({"MP4", "mp4", QStringList({"mp4"}), tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg("MP4") + " (*.mp4)",
            {ssr::enum_video_codec::VIDEO_CODEC_H264},
            {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS, ssr::enum_audio_codec::AUDIO_CODEC_MP3, ssr::enum_audio_codec::AUDIO_CODEC_AAC}}),
        ContainerData({"WebM", "webm", QStringList({"webm"}), tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg("WebM") + " (*.webm)",
            {ssr::enum_video_codec::VIDEO_CODEC_VP8},
            {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS}}),
        ContainerData({"OGG", "ogg", QStringList({"ogg"}), tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg("OGG") + " (*.ogg)",
            {ssr::enum_video_codec::VIDEO_CODEC_THEORA},
            {ssr::enum_audio_codec::AUDIO_CODEC_VORBIS}}),
        ContainerData({tr("Other..."), "other", QStringList(), "", std::set<ssr::enum_video_codec>({}), std::set<ssr::enum_audio_codec>({})}),
    };
    //video
    m_video_codecs = {
        {"H.264"       , "libx264"  },
        {"VP8"         , "libvpx"   },
        {"Theora"      , "libtheora"},
        {tr("Other..."), "other"    },
    };

    m_audio_codecs = {
        {"Vorbis"          , "libvorbis"   },
        {"MP3"             , "libmp3lame"  },
        {"AAC"             , "libvo_aacenc"},
        {tr("Uncompressed"), "pcm_s16le"   },
        {tr("Other...")    , "other"       },
    };

    m_audio_kbit_rates = {
        {"32"},
        {"96"},
        {"128"},
        {"160"},
        {"192"}
    };

    m_containers_av.clear();
#if SSR_USE_AV_MUXER_ITERATE
    const AVOutputFormat *format;
    void *format_opaque = NULL;
    while((format = av_muxer_iterate(&format_opaque)) != NULL) {
#else
    for(AVOutputFormat *format = av_oformat_next(NULL); format != NULL; format = av_oformat_next(format)) {
#endif

        if (format->video_codec == AV_CODEC_ID_NONE)
            continue;
        ContainerData c;
        c.name = format->long_name;
        c.avname = format->name;
        c.suffixes = QString(format->extensions).split(',', QString::SkipEmptyParts);
        if (c.suffixes.isEmpty()) {
            c.filter = "";
        } else {
            c.filter = tr("%1 files", "This appears in the file dialog, e.g. 'MP4 files'").arg(c.avname) + "(*." + c.suffixes[0];
            for (int i = 1; i < c.suffixes.size(); ++i) {
                c.suffixes[i] = c.suffixes[i].trimmed();// needed because libav/ffmpeg isn't very consistent when they say 'comma-separated'
                c.filter += "*." + c.suffixes[i];
            }
            c.filter += ")";
        }
        m_containers_av.push_back(c);
    }
    std::sort(m_containers_av.begin(), m_containers_av.end());

    //load AV codec list
    m_video_codecs_av.clear();
    m_audio_codecs_av.clear();
#if SSR_USE_AV_MUXER_ITERATE
    const AVCodec *codec;
    void *codec_opaque = NULL;
    while((codec = av_codec_iterate(&codec_opaque)) != NULL) {
#else
    for(AVCodec *codec = av_codec_next(NULL); codec != NULL; codec = av_codec_next(codec)) {
#endif
        if(!av_codec_is_encoder(codec))
            continue;
        if(codec->type == AVMEDIA_TYPE_VIDEO && VideoEncoder::AVCodecIsSupported(codec->name)) {
            VideoCodecData c;
            c.name = codec->long_name;
            c.avname = codec->name;
            m_video_codecs_av.push_back(c);
        }
        if(codec->type == AVMEDIA_TYPE_AUDIO && AudioEncoder::AVCodecIsSupported(codec->name)) {
            AudioCodecData c;
            c.name = codec->long_name;
            c.avname = codec->name;
            m_audio_codecs_av.push_back(c);
        }
    }
    std::sort(m_video_codecs_av.begin(), m_video_codecs_av.end());
    std::sort(m_audio_codecs_av.begin(), m_audio_codecs_av.end());
    if(m_containers_av.empty()) {
        Logger::LogError("[OutputInit] " + tr("Error: Could not find any suitable container in libavformat!"));
        throw LibavException();
    }
    if(m_video_codecs_av.empty()) {
        Logger::LogError("[OutputInit] " + tr("Error: Could not find any suitable video codec in libavcodec!"));
        throw LibavException();
    }
    if(m_audio_codecs_av.empty()) {
        Logger::LogError("[OutputInit] " + tr("Error: Could not find any suitable audio codec in libavcodec!"));
        throw LibavException();
    }

    m_lineedit_audio_options_not_shown = new QLineEdit;


    for (int i = 0; i < int(ssr::enum_video_codec::VIDEO_CODEC_COUNT); ++i) {
        ui->m_comboBox_video_codec->addItem(m_video_codecs[i].name);
    }
    //audio
    for (int i = 0; i < int(ssr::enum_audio_codec::AUDIO_CODEC_COUNT); ++i) {
        ui->m_comboBox_audio_codec->addItem(m_audio_codecs[i].name);
    }
    //audio kb rate
    for (unsigned int i = 0; i < sizeof(m_audio_kbit_rates)/sizeof(m_audio_kbit_rates[0]); ++i) {
        ui->m_comboBox_audiorate->addItem(m_audio_kbit_rates[i].rate);
    }


    QSettings settings(CommandLineOptions::GetSettingsFile(), QSettings::IniFormat);
    LoadAVProfileSettings(&settings);

}

void mypopup::LoadScreenConfigurations()
{
    std::vector<QRect> screen_geometries = GetScreenGeometries();
    QRect combined_geometry = CombineScreenGeometries(screen_geometries);
    ui->m_comboBox_videores->clear();
    ui->m_comboBox_videores->addItem(tr("All screens: %1x%2", "This appears in the screen selection combobox")
                                .arg(combined_geometry.width()).arg(combined_geometry.height()));
    for(size_t i = 0; i < screen_geometries.size(); ++i) {
        QRect &geometry = screen_geometries[i];
        ui->m_comboBox_videores->addItem(tr("Screen %1: %2x%3 at %4,%5", "This appears in the screen selection combobox")
                                    .arg(i + 1).arg(geometry.width()).arg(geometry.height()).arg(geometry.x()).arg(geometry.y()));
    }
    // update the video x/y/w/h in case the position or size of the selected screen changed
    //    OnUpdateVideoAreaFields();
}

void mypopup::LoadAVProfileSettings(QSettings *settings)
{
    // choose default container and codecs
    ssr::enum_container default_container = (ssr::enum_container) 0;
    for(unsigned int i = 0; i < int(ssr::enum_container::CONTAINER_OTHER); ++i) {
        if(AVFormatIsInstalled(m_containers[i].avname)) {
            default_container = (ssr::enum_container) i;
            break;
        }
    }
    ssr::enum_video_codec default_video_codec = (ssr::enum_video_codec) 0;
    for(unsigned int i = 0; i < int(ssr::enum_video_codec::VIDEO_CODEC_OTHER); ++i) {
        if(AVCodecIsInstalled(m_video_codecs[i].avname) && m_containers[int(default_container)].supported_video_codecs.count((ssr::enum_video_codec) i)) {
            default_video_codec = (ssr::enum_video_codec) i;
            break;
        }
    }
    ssr::enum_audio_codec default_audio_codec = (ssr::enum_audio_codec) 0;
    for(unsigned int i = 0; i < int(ssr::enum_video_codec::VIDEO_CODEC_OTHER); ++i) {
        if(AVCodecIsInstalled(m_audio_codecs[i].avname) && m_containers[int(default_container)].supported_audio_codecs.count((ssr::enum_audio_codec) i)) {
            default_audio_codec = (ssr::enum_audio_codec) i;
            break;
        }
    }
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
    QString default_file = best_dir + "/simplescreenrecorder." + m_containers[int(default_container)].suffixes[0];

    // load settings
    SetFile(settings->value("output/file", default_file).toString());
//    SetSeparateFiles(settings->value("output/separate_files", false).toBool());
//    SetAddTimestamp(settings->value("output/add_timestamp", true).toBool());
    SetContainer(StringToEnum(settings->value("output/container", QString()).toString(), default_container));
//    mp->SetContainerAV(FindContainerAV(settings->value("output/container_av", QString()).toString()));

    SetVideoCodec(StringToEnum(settings->value("output/video_codec", QString()).toString(), default_video_codec));
//    SetVideoCodecAV(FindVideoCodecAV(settings->value("output/video_codec_av", QString()).toString()));
//    SetVideoKBitRate(settings->value("output/video_kbit_rate", 5000).toUInt());
//    SetH264CRF(settings->value("output/video_h264_crf", 23).toUInt());
//    SetH264Preset((enum_h264_preset) settings->value("output/video_h264_preset", H264_PRESET_SUPERFAST).toUInt());
//    SetVP8CPUUsed(settings->value("output/video_vp8_cpu_used", 5).toUInt());
//    SetVideoOptions(settings->value("output/video_options", "").toString());
//    SetVideoAllowFrameSkipping(settings->value("output/video_allow_frame_skipping", true).toBool());

    SetAudioCodec(StringToEnum(settings->value("output/audio_codec", QString()).toString(), default_audio_codec));
//    SetAudioCodecAV(FindAudioCodecAV(settings->value("output/audio_codec_av", QString()).toString()));
//    ssr->SetAudioKBitRate(settings->value("output/audio_kbit_rate", 128).toUInt());
    m_lineedit_audio_options_not_shown->setText(settings->value("output/audio_options", "").toString());

    // update things
    OnUpdateContainerFields();
//    mp->OnUpdateVideoCodecFields();
//    mp->OnUpdateAudioCodecFields();
}


QString mypopup::GetFileProtocol()
{
    QRegExp protocol_regex("^([a-z0-9]+)://", Qt::CaseInsensitive, QRegExp::RegExp);
    if(protocol_regex.indexIn(GetFile()) < 0) {
        return QString();
    }
    return protocol_regex.cap(1);
}

QString mypopup::GetContainerAVName() {
    ssr::enum_container container = GetContainer();
    if(container != ssr::enum_container::CONTAINER_OTHER)
        return m_containers[int(container)].avname;
    else
        return m_containers_av[GetContainerAV()].avname;
}

QString mypopup::GetVideoCodecAVName() {
    ssr::enum_video_codec video_codec = GetVideoCodec();
    if(video_codec != ssr::enum_video_codec::VIDEO_CODEC_OTHER)
        return m_video_codecs[int(video_codec)].avname;
//    else
//        return ssr_video_codecs_av[GetVideoCodecAV()].avname;
    return NULL;
}

QString mypopup::GetAudioCodecAVName() {
    ssr::enum_audio_codec audio_codec = GetAudioCodec();
    if(audio_codec != ssr::enum_audio_codec::AUDIO_CODEC_OTHER)
        return m_audio_codecs[int(audio_codec)].avname;
//    else
//        return m_audio_codecs_av[GetAudioCodecAV()].avname;
    return NULL;
}

QString mypopup::GetAudioKBitRateName()
{
    int audio_kbirt_rate = GetAudioKBitRate();
    return m_audio_kbit_rates[audio_kbirt_rate].rate;
}

//void mypopup::OnUpdateVideoAreaFields()
//{
//    switch(ssr->GetVideoArea()) {
//    case ssr::enum_video_area::VIDEO_AREA_SCREEN: {
//        int sc = ui->m_comboBox_videores->currentIndex();
//        std::vector<QRect> screen_geometries = GetScreenGeometries();
//        QRect rect;
//        if (sc > 0 && sc <= (int)screen_geometries.size()) {
//            rect = screen_geometries[sc - 1];
//        } else {
//            rect = CombineScreenGeometries(screen_geometries);
//        }
//        ssr->SetVideoX(rect.left());
//        ssr->SetVideoY(rect.top());
//        ssr->SetVideoW(rect.width());
//        ssr->SetVideoH(rect.height());
//        break;
//    }
//    case ssr::enum_video_area::VIDEO_AREA_FIXED: {
//        break;
//    }
//    case ssr::enum_video_area::VIDEO_AREA_CURSOR: {
//        break;
//    }
//    default:
//        break;
//    }
//}


//void mypopup::OnUpdateVideoAreaFields(int &sc)
//{
//    switch(ssr->GetVideoArea()) {
//    case ssr::enum_video_area::VIDEO_AREA_SCREEN: {
//        sc = ui->m_comboBox_videores->currentIndex();
//        std::vector<QRect> screen_geometries = GetScreenGeometries();
//        QRect rect;
//        if (sc > 0 && sc <= (int)screen_geometries.size()) {
//            rect = screen_geometries[sc - 1];
//        } else {
//            rect = CombineScreenGeometries(screen_geometries);
//        }
//        ssr->SetVideoX(rect.left());
//        ssr->SetVideoY(rect.top());
//        ssr->SetVideoW(rect.width());
//        ssr->SetVideoH(rect.height());
//        break;
//    }
//    case ssr::enum_video_area::VIDEO_AREA_FIXED: {
//        break;
//    }
//    case ssr::enum_video_area::VIDEO_AREA_CURSOR: {
//        break;
//    }
//    default:
//        break;
//    }
//}

static bool MatchSuffix(const QString& suffix, const QStringList& suffixes) {
    return ((suffix.isEmpty() && suffixes.isEmpty()) || suffixes.contains(suffix, Qt::CaseInsensitive));
}

void mypopup::OnUpdateSuffixAndContainerFields()
{
    // change file extension
    ssr::enum_container new_container = GetContainer();
    unsigned int new_container_av = GetContainerAV();

    if(GetFileProtocol().isNull()) {
        QStringList old_suffixes = (m_old_container == ssr::enum_container::CONTAINER_OTHER)?
                    m_containers_av[m_old_container_av].suffixes : m_containers[int(m_old_container)].suffixes;
        QStringList new_suffixes = (new_container == ssr::enum_container::CONTAINER_OTHER)?
                    m_containers_av[new_container_av].suffixes : m_containers[int(new_container)].suffixes;
        QString file = GetFile();
        if(!file.isEmpty()) {
            QFileInfo fi(file);
            if(MatchSuffix(fi.suffix(), old_suffixes) && !MatchSuffix(fi.suffix(), new_suffixes)) {
                if(new_suffixes.isEmpty())
                    m_lineedit_file_not_shown->setText(fi.path() + "/" + fi.completeBaseName());
                else
                    m_lineedit_file_not_shown->setText(fi.path() + "/" + fi.completeBaseName() + "." + new_suffixes[0]);
            }
        }
    }

    // update fields
    OnUpdateContainerFields();
}

void mypopup::OnUpdateContainerFields()
{
    ssr::enum_container container = GetContainer();
    unsigned int container_av = GetContainerAV();

    // show/hide fields
//    GroupVisible({m_label_container_av_not_shown, m_combobox_container_av_not_shown}, (container == ssr::enum_container::CONTAINER_OTHER));

    // show/hide warning
//    m_label_container_warning->setVisible(GetContainerAVName() == "mp4");

    // mark uninstalled or unsupported codecs
    for(unsigned int i = 0; i < int(ssr::enum_video_codec::VIDEO_CODEC_OTHER); ++i) {
        QString name = m_video_codecs[i].name;
        if(!AVCodecIsInstalled(m_video_codecs[i].avname))
            name += " (" + tr("not installed") + ")";
        else if(container != ssr::enum_container::CONTAINER_OTHER && !m_containers[int(container)].supported_video_codecs.count((ssr::enum_video_codec) i))
            name += " (" + tr("not supported by container") + ")";
        ui->m_comboBox_video_codec->setItemText(i, name);
    }
    for(unsigned int i = 0; i < int(ssr::enum_audio_codec::AUDIO_CODEC_OTHER); ++i) {
        QString name = m_audio_codecs[i].name;
        if(!AVCodecIsInstalled(m_audio_codecs[i].avname))
            name += " (" + tr("not installed") + ")";
        else if(container != ssr::enum_container::CONTAINER_OTHER && !m_containers[int(container)].supported_audio_codecs.count((ssr::enum_audio_codec) i))
            name += " (" + tr("not supported by container") + ")";
        ui->m_comboBox_audio_codec->setItemText(i, name);
    }

    m_old_container = container;
    m_old_container_av = container_av;
}

void mypopup::OnIdentifyScreens()
{
    OnStopIdentifyScreens();
    std::vector<QRect> screen_geometries = GetScreenGeometries();
    for (size_t i = 0; i < screen_geometries.size(); ++i) {
        QRect &rect = screen_geometries[i];
        ScreenLabelWindow *label = new ScreenLabelWindow(this, tr("Screen %1", "This appears in the screen labels").arg(i + 1));
        label->move(rect.x(), rect.y());
        label->show();
         m_screen_labels.push_back(label);
    }
}

void mypopup::OnStopIdentifyScreens()
{
    for (unsigned int i = 0; i < m_screen_labels.size(); ++i) {
        delete m_screen_labels[i];
    }
    m_screen_labels.clear();
}

static void hide_window(QWidget *parent)
    {
        parent->setWindowFlags(Qt::BypassWindowManagerHint
                    | Qt::FramelessWindowHint
                    | Qt::Tool);
       parent->hide();
    }
static void show_window(QWidget *parent)
    {
        parent->setWindowFlags(Qt::BypassWindowManagerHint
                       | Qt::WindowStaysOnTopHint
                       | Qt::FramelessWindowHint
                       | Qt::Tool);
        parent->show();

    }
void mypopup::OnBrowse()
{
    qDebug() << "bybobbi will hide window";
    hide_window(cw);
    qDebug() << "bybobbi after hide window";
    QString filters;
    for (int i = 0; i < int(ssr::enum_container::CONTAINER_OTHER); ++i) {
        if (i != 0)
            filters += m_containers[i].filter;
    }

    for (unsigned int i = 0; i < m_containers_av.size(); ++i) {
        if (!m_containers_av[i].filter.isEmpty())
            filters += ";;" + m_containers_av[i].filter;
    }

    ssr::enum_container container = GetContainer();
    unsigned int container_av = GetContainerAV();
    QString selected_filter = (container == ssr::enum_container::CONTAINER_OTHER) ?
                m_containers_av[container_av].filter : m_containers[int(container)].filter;
    QString selected_file = QFileDialog::getSaveFileName(this, tr("Save recording as"),
                                                         GetFile(), filters, &selected_filter, QFileDialog::DontConfirmOverwrite);
    if (selected_file.isNull()) {
        return;
    }
    show_window(cw);
    m_lineedit_file_not_shown->clear();
    QFileInfo fi(selected_file);
    if(fi.suffix().isEmpty()) {
        QStringList suffixes = (container == ssr::enum_container::CONTAINER_OTHER)? m_containers_av[container_av].suffixes : m_containers[int(container)].suffixes;
        if(!suffixes.isEmpty())
            selected_file += "." + suffixes[0];
    } else {
        bool found = false;
        for(int i = 0; i < int(ssr::enum_container::CONTAINER_OTHER); ++i) {
            if(m_containers[i].suffixes.contains(fi.suffix(), Qt::CaseInsensitive)) {
                SetContainer((ssr::enum_container) i);
                found = true;
                break;
            }
        }
        if(!found) {
            for(unsigned int i = 0; i < m_containers_av.size(); ++i) {
                if(m_containers_av[i].suffixes.contains(fi.suffix(), Qt::CaseInsensitive)) {
                    SetContainer(ssr::enum_container::CONTAINER_OTHER);
                    SetContainerAV(i);
                    break;
                }
            }
        }
    }
    SetFile(selected_file);

    OnUpdateContainerFields();

}

ssr::enum_container mypopup::GetContainer()
{
    return (ssr::enum_container) clamp(ui->m_comboBox_container->currentIndex(), 0, int(ssr::enum_container::CONTAINER_COUNT) - 1);
}

#if 1
unsigned int mypopup::GetContainerAV()
{
    return clamp(m_combobox_container_av_not_shown->currentIndex(), 0, int(m_containers.size()) - 1);
}
#endif

void mypopup::SetContainer(ssr::enum_container container)
{
    ui->m_comboBox_container->setCurrentIndex(clamp((unsigned int) container, 0u, (unsigned int) ssr::enum_container::CONTAINER_COUNT - 1));
}

void mypopup::SetContainerAV(unsigned int container)
{
    m_combobox_container_av_not_shown->setCurrentIndex(clamp((unsigned int)container, 0u, (unsigned int) m_containers_av.size() - 1));
}

ssr::enum_video_codec mypopup::GetVideoCodec()
{
     return (ssr::enum_video_codec) clamp(ui->m_comboBox_video_codec->currentIndex(), 0, int(ssr::enum_video_codec::VIDEO_CODEC_COUNT) - 1);
}

void mypopup::SetVideoCodec(ssr::enum_video_codec video_codec)
{
    ui->m_comboBox_video_codec->setCurrentIndex(clamp((unsigned int) video_codec,
                                                      0u, (unsigned int) ssr::enum_video_codec::VIDEO_CODEC_COUNT - 1));
}

ssr::enum_audio_codec mypopup::GetAudioCodec()
{
     return (ssr::enum_audio_codec) clamp(ui->m_comboBox_audio_codec->currentIndex(), 0, int(ssr::enum_audio_codec::AUDIO_CODEC_COUNT) - 1);
}

//ssr::enum_video_codec mypopup::GetVideoCodecAV()
//{
//    return clamp(ui->m_comboBox_video_codec_av->currentIndex(), 0, (int) m_video_codecs_av.size() - 1);
//}

void mypopup::SetAudioCodec(ssr::enum_audio_codec audio_codec)
{
    ui->m_comboBox_audio_codec->setCurrentIndex(clamp((unsigned int) audio_codec, 0u, (unsigned int) ssr::enum_audio_codec::AUDIO_CODEC_COUNT - 1));
}

int mypopup::GetAudioKBitRate()
{
    int len = sizeof(m_audio_kbit_rates)/sizeof(m_audio_kbit_rates[0]) - 1;
    return clamp(ui->m_comboBox_audiorate->currentIndex(), 0,  len);
}

//void mypopup::SetVideoKBitRate(unsigned int kbit_rate)
//{
//    ui->m_lineedit_video_kbit_rate->setCurrentIndex(clamp((unsigned int) audio_codec, 0u, (unsigned int) ssr::enum_audio_codec::AUDIO_CODEC_COUNT - 1));
//     m_lineedit_video_kbit_rate->setText(QString::number(kbit_rate));
//}
