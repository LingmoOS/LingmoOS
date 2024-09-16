# Deepin Camera

Deepin camera is Deepin Desktop Environment Camera Software.

## 依赖

### 构建依赖

* qmake
* qt5
* ffmpeg
* guvc

### 运行依赖

* Qt5 (>= 5.3)
* Qt5-X11extras
* Dtk

##安装

### 构建源码

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd deepin-camera
$ mkdir Build
$ cd Build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr ..
$ make
```

3. Install:
```
$ sudo make install
```

When install complete, the executable binary file is placed into `/usr/bin/deepin-camera`.

## 帮助

Any usage issues can ask for help via
* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC Channel](https://webchat.freenode.net/?channels=deepin)
* [Official Forum](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)

## 参与贡献

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 协议

deepin-camera 根据 [GPL-3.0-or-later]（许可证）获得许可.



## 第三方库

Deepin Camera  is based on Guvcview which is a excellent software uses USB-camera to take photos & videos.

Reference documents include：

config.c config.h options.c options.h audio.c audio.h audio_fx.c audio_fx.h audio_portaudio.c audio_portaudio.h audio_pulseaudio.c audio_pulseaudio.h audio_codecs.c audio_codecs.h encoder.c encoder.h file_io.c file_io.h libav_encoder.c libav_encoder.h gview.h gviewencoder.hmatroska.h stream_io.h matroska.c muxer.c stream_io.c video_codecs.c render.h render_sdl2.h render.c render_fx.c render_osd_crosshair.c render_osd_vu_meter.c render_sdl2.c colorspaces.h control_profile.h core_io.h core_time.h dct.h frame_decoder.h gui.h gview.h gviewv4l2core.h jpeg_decoder.h save_image.h soft_autofocus.h uvc_h264.h v4l2_comtrols.h v4l2_core.h v4l2_devices.h v4l2_formats.h v4l2_xu_ctrls.h colorspaces.c control_profile.c core_io.c core_time.c dct.c frame_decoder.c gui.c jpeg_decoder.c save_image.c soft_autofocus.c uvc_h264.c v4l2_comtrols.c v4l2_core.c v4l2_devices.c v4l2_formats.c v4l2_xu_ctrls.c save_image_jpeg.c save_image_bmp.c save_image_png.c