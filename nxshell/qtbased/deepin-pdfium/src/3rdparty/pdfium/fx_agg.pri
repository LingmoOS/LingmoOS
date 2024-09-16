# module fx_agg
SOURCE_DIR = $$PWD/pdfium/third_party

HEADERS += \
    $$PWD/pdfium/third_party/agg23/agg_array.h \
    $$PWD/pdfium/third_party/agg23/agg_basics.h \
    $$PWD/pdfium/third_party/agg23/agg_clip_liang_barsky.h \
    $$PWD/pdfium/third_party/agg23/agg_color_gray.h \
    $$PWD/pdfium/third_party/agg23/agg_conv_adaptor_vcgen.h \
    $$PWD/pdfium/third_party/agg23/agg_conv_dash.h \
    $$PWD/pdfium/third_party/agg23/agg_conv_stroke.h \
    $$PWD/pdfium/third_party/agg23/agg_curves.h \
    $$PWD/pdfium/third_party/agg23/agg_math.h \
    $$PWD/pdfium/third_party/agg23/agg_math_stroke.h \
    $$PWD/pdfium/third_party/agg23/agg_path_storage.h \
    $$PWD/pdfium/third_party/agg23/agg_pixfmt_gray.h \
    $$PWD/pdfium/third_party/agg23/agg_rasterizer_scanline_aa.h \
    $$PWD/pdfium/third_party/agg23/agg_render_scanlines.h \
    $$PWD/pdfium/third_party/agg23/agg_renderer_base.h \
    $$PWD/pdfium/third_party/agg23/agg_renderer_scanline.h \
    $$PWD/pdfium/third_party/agg23/agg_rendering_buffer.h \
    $$PWD/pdfium/third_party/agg23/agg_scanline_u.h \
    $$PWD/pdfium/third_party/agg23/agg_shorten_path.h \
    $$PWD/pdfium/third_party/agg23/agg_vcgen_dash.h \
    $$PWD/pdfium/third_party/agg23/agg_vcgen_stroke.h \
    $$PWD/pdfium/third_party/agg23/agg_vertex_sequence.h

SOURCES += \
    $$PWD/pdfium/third_party/agg23/agg_curves.cpp \
    $$PWD/pdfium/third_party/agg23/agg_path_storage.cpp \
    $$PWD/pdfium/third_party/agg23/agg_rasterizer_scanline_aa.cpp \
    $$PWD/pdfium/third_party/agg23/agg_vcgen_dash.cpp \
    $$PWD/pdfium/third_party/agg23/agg_vcgen_stroke.cpp


