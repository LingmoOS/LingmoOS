INCLUDEPATH += $$PWD
#            += src/record/AV src/record/AV/output src/record/AV/input src/record/common src/record/GUI

DEFINES += SSR_USE_PULSEAUDIO

HEADERS += \
#    $$PWD/tools/CaptureToolCap.h \
#    $$PWD/tools/screencut/screencut.h \
    $$PWD/AV/AVWrapper.h \
    $$PWD/AV/FastResampler.h \
    $$PWD/AV/FastScaler.h \
    $$PWD/AV/FastScaler_Scale_Generic.h \
    $$PWD/AV/SourceSink.h \
    $$PWD/AV/input/PulseAudioInput.h \
    $$PWD/AV/input/X11Input.h \
    $$PWD/AV/input/input_widgets.h \
    $$PWD/AV/output/AudioEncoder.h \
    $$PWD/AV/output/BaseEncoder.h \
    $$PWD/AV/output/Muxer.h \
    $$PWD/AV/output/OutputManager.h \
    $$PWD/AV/output/SyncDiagram.h \
    $$PWD/AV/output/Synchronizer.h \
    $$PWD/AV/output/VideoEncoder.h \
    $$PWD/AV/output/X264Presets.h \
    $$PWD/common/CPUFeatures.h \
    $$PWD/common/WidgetWrapper.h \
    $$PWD/common/utils.h \
    $$PWD/../tools/cut/cut.h \
    $$PWD/../tools/luping/luping.h \
    $$PWD/../tools/cursor_record/cursor_record_tool.h \
    $$PWD/../tools/audio_record/audio_record_tool.h \
    $$PWD/../tools/follow_mouse_record/follow_mouse_record_tool.h \
    $$PWD/../tools/option_record/option_record_tool.h \
#    $$PWD/tools/close_record/close_record_tool.h \
    $$PWD/../tools/start_record/start_record_tool.h \
#    $$PWD/../tools/toolfactorycap.h \
#    $$PWD/../widgets/capture/buttonhandercap.h \
#    $$PWD/../widgets/capture/capturebuttoncap.h \
    $$PWD/mypopup.h \
    $$PWD/recorder.h \
    $$PWD/ssrtools.h \
    $$PWD/common/X_utils.hpp \
 \#    $$PWD/myrecord.h

SOURCES += \
#    $$PWD/tools/screencap/screencap.cpp \
#    $$PWD/tools/screencapexit/screencapexit.cpp \
#    $$PWD/tools/screencapoption/screencapoption.cpp \
#    $$PWD/tools/screencapstart/screencapstart.cpp \
#    $$PWD/tools/screencut/screencut.cpp \
#    $$PWD/tools/screenmouse/screenmouse.cpp \
#    $$PWD/tools/screenmousecursor/screenmousecursor.cpp \
#    $$PWD/tools/screenvoice/screenvoice.cpp \
    $$PWD/AV/AVWrapper.cpp \
    $$PWD/AV/FastResampler.cpp \
    $$PWD/AV/FastResamper_FirFilter_Fallback.cpp \
#    $$PWD/AV/FastResampler_FirFilter.cpp \
    $$PWD/AV/FastScaler.cpp \
    $$PWD/AV/FastScaler_Convert_SSSE3.cpp \
#    $$PWD/AV/FastScaler_Scale.cpp \
    $$PWD/AV/FastScaler_Scale_Generic.cpp \
    $$PWD/AV/FastScaler_Scale_Fallback.cpp \
    $$PWD/AV/FastScaler_Convert_Fallback.cpp \
#    $$PWD/AV/SampleCast.cpp \
    $$PWD/AV/SourceSink.cpp \
    $$PWD/AV/input/PulseAudioInput.cpp \
    $$PWD/AV/input/X11Input.cpp \
    $$PWD/AV/input/input_widgets.cpp \
    $$PWD/AV/output/AudioEncoder.cpp \
    $$PWD/AV/output/BaseEncoder.cpp \
    $$PWD/AV/output/Muxer.cpp \
    $$PWD/AV/output/OutputManager.cpp \
    $$PWD/AV/output/SyncDiagram.cpp \
    $$PWD/AV/output/Synchronizer.cpp \
    $$PWD/AV/output/VideoEncoder.cpp \
    $$PWD/AV/output/X264Presets.cpp \
    $$PWD/common/CPUFeatures.cpp \
    $$PWD/common/WidgetWrapper.cpp \
    $$PWD/common/utils.cpp \
    $$PWD/../tools/cut/cut.cpp \
    $$PWD/../tools/luping/luping.cpp \
    $$PWD/../tools/cursor_record/cursor_record_tool.cpp \
    $$PWD/../tools/audio_record/audio_record_tool.cpp \
    $$PWD/../tools/follow_mouse_record/follow_mouse_record_tool.cpp \
    $$PWD/../tools/option_record/option_record_tool.cpp \
#    $$PWD/tools/close_record/close_record_tool.cpp \
    $$PWD/../tools/start_record/start_record_tool.cpp \
#    $$PWD/../tools/toolfactorycap.cpp \
#    $$PWD/../widgets/capture/buttonhandercap.cpp \
#    $$PWD/../widgets/capture/capturebuttoncap.cpp \
    $$PWD/../record/mypopup.cpp \
    $$PWD/../record/ssrtools.cpp \
    $$PWD/recorder.cpp

FORMS += \
    $$PWD/mypopup.ui \
    $$PWD/ssrtools.ui
