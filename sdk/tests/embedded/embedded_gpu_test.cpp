// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

#include "ui/embedded_gpu.h"
#include "ui/embedded_window.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/styled_label.h"
#include "ui/text_field.h"
#include "ui/view.h"

using namespace Lingmo::UI;

namespace {

// A GLES context as the compositor makes one: client version 2, no surface.
class GlHost
{
public:
    GlHost()
    {
        const auto getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));
        if (!getPlatformDisplay)
            return;
        m_display = getPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
        if (m_display == EGL_NO_DISPLAY || !eglInitialize(m_display, nullptr, nullptr))
            return;
        eglBindAPI(EGL_OPENGL_ES_API);
        const EGLint attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        m_context = eglCreateContext(m_display, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attributes);
        if (m_context == EGL_NO_CONTEXT)
            return;
        m_valid = makeCurrent();
        if (m_valid)
            fprintf(stderr, "GL: %s\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    }

    ~GlHost()
    {
        if (m_display == EGL_NO_DISPLAY)
            return;
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_context != EGL_NO_CONTEXT)
            eglDestroyContext(m_display, m_context);
        eglTerminate(m_display);
    }

    bool isValid() const { return m_valid; }
    bool makeCurrent() { return eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_context); }
    void release() { eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); }

    // As the compositor's uploader makes one.
    static GLuint texture(int width, int height)
    {
        GLuint name = 0;
        glGenTextures(1, &name);
        glBindTexture(GL_TEXTURE_2D, name);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        return name;
    }

    static std::vector<uint8_t> read(GLuint texture, int width, int height)
    {
        GLint previous = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous);
        GLuint framebuffer = 0;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        std::vector<uint8_t> pixels(size_t(width) * size_t(height) * 4);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glBindFramebuffer(GL_FRAMEBUFFER, GLuint(previous));
        glDeleteFramebuffers(1, &framebuffer);
        return pixels;
    }

private:
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLContext m_context = EGL_NO_CONTEXT;
    bool m_valid = false;
};

class Panel : public View
{
public:
    Panel()
    {
        m_title = addChild<StyledLabel>("Thu Oct 15");
        m_title->setFontSize(20);
        m_title->setWeight(600);
        m_title->setColor(Color::rgb(0xffffff));
        m_title->setStyle(StyledLabel::Style::Raised, Color{0, 0, 0, 77});
        m_name = addChild<Label>("tester");
        m_name->setFontSize(13);
        m_field = addChild<TextField>("secret");
        m_field->setEchoMode(TextField::EchoMode::Password);
    }

    Color fill = Color{255, 255, 255, 41};

protected:
    void layout() override
    {
        m_title->setGeometry({20, 10, 200, 28});
        m_name->setGeometry({20, 50, 100, 18});
        m_field->setGeometry({20, 80, 200, 35});
    }

    void paint(Painter& painter) override
    {
        painter.drawRoundedRect({10, 130, 120, 32}, {16}, fill, 1, Color{255, 255, 255, 82});
        painter.drawRoundedRect({150, 125, 64, 64}, {32}, Color{40, 120, 220, 255});
        painter.drawSmoothRectangle({230, 20, 40, 24}, 12, Color{20, 200, 90, 200});
        painter.fillRect({230, 60, 40.5, 10.25}, Color{200, 30, 30, 255});
        painter.fillEllipse({230, 80, 30, 30}, Color{250, 200, 20, 255});
        painter.drawShadow({20, 170, 60, 20}, 10, 4, Color{0, 0, 0, 80});
        painter.strokeRoundedRect({100, 170, 60, 20}, 10, Color{0, 0, 0, 160}, 2);
    }

private:
    StyledLabel* m_title;
    Label* m_name;
    TextField* m_field;
};

struct Difference
{
    int maximum = 0;
    // Pixels with a channel more than 8 levels apart.
    size_t large = 0;
};

static Difference compare(const uint8_t* a, const uint8_t* b, size_t pixels)
{
    Difference difference;
    for (size_t i = 0; i < pixels; ++i) {
        int pixel = 0;
        for (int c = 0; c < 4; ++c)
            pixel = std::max(pixel, std::abs(int(a[i * 4 + c]) - int(b[i * 4 + c])));
        difference.maximum = std::max(difference.maximum, pixel);
        if (pixel > 8)
            ++difference.large;
    }
    return difference;
}

} // namespace

TEST(EmbeddedGpuTest, MatchesTheCpuFrames)
{
    GlHost host;
    if (!host.isValid())
        GTEST_SKIP() << "no surfaceless EGL";
    std::unique_ptr<EmbeddedGpu> gpu = EmbeddedGpu::createForCurrentContext();
    ASSERT_TRUE(gpu);

    for (const double scale : {1.0, 1.5, 1.75}) {
        EmbeddedWindow window;
        auto content = std::make_unique<Panel>();
        Panel* panel = content.get();
        window.setContentView(std::move(content));
        window.setScale(scale);
        window.setSize({300, 200});
        const int width = window.pixelWidth();
        const int height = window.pixelHeight();
        const GLuint texture = GlHost::texture(width, height);

        const EmbeddedWindow::Frame* gpuFrame = window.render(*gpu, {texture, width, height, false}, 1000000000);
        ASSERT_NE(gpuFrame, nullptr);
        EXPECT_EQ(gpuFrame->pixels, nullptr);
        ASSERT_EQ(gpuFrame->damage.size(), 1u);
        const std::vector<uint8_t> gpuPixels = GlHost::read(texture, width, height);
        window.requestPaint();
        const EmbeddedWindow::Frame& cpuFrame = window.render(1016000000);
        Difference difference = compare(gpuPixels.data(), cpuFrame.pixels, size_t(width) * size_t(height));
        fprintf(stderr, "scale %g: largest difference %d, %zu of %d pixels over 8\n", scale, difference.maximum,
                difference.large, width * height);
        EXPECT_LT(difference.large, size_t(width * height) / 200) << scale;

        // An incremental GPU frame into the kept texture, against a full CPU one.
        window.render(*gpu, {texture, width, height, false}, 1033000000);
        panel->fill = Color{255, 255, 255, 120};
        panel->requestPaint();
        gpuFrame = window.render(*gpu, {texture, width, height, true}, 1050000000);
        ASSERT_NE(gpuFrame, nullptr);
        const std::vector<uint8_t> incremental = GlHost::read(texture, width, height);
        window.requestPaint();
        const EmbeddedWindow::Frame& full = window.render(1066000000);
        difference = compare(incremental.data(), full.pixels, size_t(width) * size_t(height));
        EXPECT_LT(difference.large, size_t(width * height) / 200) << scale;
        glDeleteTextures(1, &texture);
    }
}

TEST(EmbeddedGpuTest, KeepsTheStateContract)
{
    GlHost host;
    if (!host.isValid())
        GTEST_SKIP() << "no surfaceless EGL";
    std::unique_ptr<EmbeddedGpu> gpu = EmbeddedGpu::createForCurrentContext();
    ASSERT_TRUE(gpu);
    EmbeddedWindow window;
    window.setContentView(std::make_unique<Panel>());
    window.setScale(1.5);
    window.setSize({300, 200});
    const GLuint texture = GlHost::texture(window.pixelWidth(), window.pixelHeight());

    // The host's own target and state around the render.
    const GLuint target = GlHost::texture(64, 64);
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
    glViewport(3, 4, 50, 60);
    glScissor(1, 2, 30, 40);
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0.25f, 0.5f, 0.75f, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    glActiveTexture(GL_TEXTURE3);

    ASSERT_NE(window.render(*gpu, {texture, window.pixelWidth(), window.pixelHeight(), false}), nullptr);
    EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));

    GLint value = -1;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
    EXPECT_EQ(value, GLint(framebuffer));
    GLint box[4] = {};
    glGetIntegerv(GL_VIEWPORT, box);
    EXPECT_EQ(std::vector<GLint>(box, box + 4), (std::vector<GLint>{3, 4, 50, 60}));
    glGetIntegerv(GL_SCISSOR_BOX, box);
    EXPECT_EQ(std::vector<GLint>(box, box + 4), (std::vector<GLint>{1, 2, 30, 40}));
    EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));
    GLfloat clear[4] = {};
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clear);
    EXPECT_FLOAT_EQ(clear[1], 0.5f);

    glGetIntegerv(GL_CURRENT_PROGRAM, &value);
    EXPECT_EQ(value, 0);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &value);
    EXPECT_EQ(value, 0);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &value);
    EXPECT_EQ(value, GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &value);
    EXPECT_EQ(value, 0);
    EXPECT_FALSE(glIsEnabled(GL_BLEND));
    glGetIntegerv(GL_BLEND_SRC_RGB, &value);
    EXPECT_EQ(value, GL_ONE);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &value);
    EXPECT_EQ(value, GL_ONE_MINUS_SRC_ALPHA);
    EXPECT_FALSE(glIsEnabled(GL_STENCIL_TEST));
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &value);
    EXPECT_EQ(value, 4);
    if (const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION)); version[10] >= '3') {
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &value);
        EXPECT_EQ(value, 0);
        glGetIntegerv(GL_SAMPLER_BINDING, &value);
        EXPECT_EQ(value, 0);
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &value);
        EXPECT_EQ(value, 0);
        GLint attributes = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attributes);
        for (GLint attribute = 0; attribute < attributes; ++attribute) {
            glGetVertexAttribiv(GLuint(attribute), GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &value);
            EXPECT_EQ(value, 0) << attribute;
            glGetVertexAttribiv(GLuint(attribute), GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &value);
            EXPECT_EQ(value, 0) << attribute;
            glGetVertexAttribiv(GLuint(attribute), GL_VERTEX_ATTRIB_ARRAY_ENABLED, &value);
            EXPECT_EQ(value, 0) << attribute;
        }
        EXPECT_FALSE(glIsEnabled(GL_PRIMITIVE_RESTART_FIXED_INDEX));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &target);
    glDeleteTextures(1, &texture);
}

// As the compositor draws: a program, client-side vertex arrays, glDrawArrays.
static GLuint compile(GLenum type, const char* source)
{
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

TEST(EmbeddedGpuTest, HostDrawsWithClientArraysAfterwards)
{
    GlHost host;
    if (!host.isValid())
        GTEST_SKIP() << "no surfaceless EGL";
    std::unique_ptr<EmbeddedGpu> gpu = EmbeddedGpu::createForCurrentContext();
    ASSERT_TRUE(gpu);
    EmbeddedWindow window;
    auto content = std::make_unique<Panel>();
    Panel* panel = content.get();
    window.setContentView(std::move(content));
    window.setScale(1.5);
    window.setSize({300, 200});
    const int width = window.pixelWidth();
    const int height = window.pixelHeight();
    const GLuint texture = GlHost::texture(width, height);

    // The host samples the frame onto its own target, as the compositor
    // composites it.
    const GLuint program = glCreateProgram();
    glAttachShader(program, compile(GL_VERTEX_SHADER, "attribute vec2 position; attribute vec2 coord;"
                                                      "varying vec2 v; void main() { v = coord;"
                                                      " gl_Position = vec4(position, 0.0, 1.0); }"));
    glAttachShader(program, compile(GL_FRAGMENT_SHADER, "precision mediump float; varying vec2 v;"
                                                        "uniform sampler2D frame;"
                                                        "void main() { gl_FragColor = texture2D(frame, v); }"));
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "coord");
    glLinkProgram(program);
    const GLuint target = GlHost::texture(width, height);
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);

    for (int frame = 0; frame < 3; ++frame) {
        panel->fill = Color{255, 255, 255, uint8_t(41 + 60 * frame)};
        panel->requestPaint();
        // The host's own program and texture are bound when it asks for the frame.
        glUseProgram(program);
        glBindTexture(GL_TEXTURE_2D, target);
        ASSERT_NE(window.render(*gpu, {texture, width, height, frame > 0}, 1000000000 + uint64_t(frame) * 16000000),
                  nullptr);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
        glViewport(0, 0, width, height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUniform1i(glGetUniformLocation(program, "frame"), 0);
        const GLfloat positions[] = {-1, -1, 1, -1, -1, 1, 1, 1};
        const GLfloat coords[] = {0, 0, 1, 0, 0, 1, 1, 1};
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, positions);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, coords);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));

        const std::vector<uint8_t> composited = GlHost::read(target, width, height);
        window.requestPaint();
        const EmbeddedWindow::Frame& cpu = window.render(1000000000 + uint64_t(frame) * 16000000 + 8000000);
        const Difference difference = compare(composited.data(), cpu.pixels, size_t(width) * size_t(height));
        EXPECT_LT(difference.large, size_t(width * height) / 200) << frame;
        // Some of it is drawn: the panel is not empty.
        size_t inked = 0;
        for (size_t i = 3; i < composited.size(); i += 4)
            inked += composited[i] > 0;
        EXPECT_GT(inked, size_t(width * height) / 50) << frame;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteProgram(program);
    glDeleteTextures(1, &target);
    glDeleteTextures(1, &texture);
}

TEST(EmbeddedGpuTest, CpuFrameAfterGpuFramesIsCurrent)
{
    GlHost host;
    if (!host.isValid())
        GTEST_SKIP() << "no surfaceless EGL";
    std::unique_ptr<EmbeddedGpu> gpu = EmbeddedGpu::createForCurrentContext();
    ASSERT_TRUE(gpu);
    EmbeddedWindow window;
    auto content = std::make_unique<Panel>();
    Panel* panel = content.get();
    window.setContentView(std::move(content));
    window.setScale(1.5);
    window.setSize({300, 200});
    const int width = window.pixelWidth();
    const int height = window.pixelHeight();
    const GLuint texture = GlHost::texture(width, height);

    // CPU first, so its buffer holds an older frame.
    window.render(1000000000);
    ASSERT_NE(window.render(*gpu, {texture, width, height, false}, 1016000000), nullptr);
    panel->fill = Color{255, 0, 0, 255};
    panel->requestPaint();
    ASSERT_NE(window.render(*gpu, {texture, width, height, true}, 1033000000), nullptr);
    // Nothing changed since: the CPU frame must still show the red fill.
    const EmbeddedWindow::Frame& after = window.render(1050000000);
    const std::vector<uint8_t> cpu(after.pixels, after.pixels + size_t(width) * size_t(height) * 4);

    EmbeddedWindow fresh;
    auto freshContent = std::make_unique<Panel>();
    freshContent->fill = Color{255, 0, 0, 255};
    fresh.setContentView(std::move(freshContent));
    fresh.setScale(1.5);
    fresh.setSize({300, 200});
    const EmbeddedWindow::Frame& reference = fresh.render(1050000000);
    EXPECT_EQ(compare(cpu.data(), reference.pixels, size_t(width) * size_t(height)).maximum, 0);
    glDeleteTextures(1, &texture);
}

// Large text at 2 and 3: Ganesh would draw it as distance fields.
class Clock : public View
{
public:
    Clock()
    {
        Label* time = addChild<Label>("09:41");
        time->setFontSize(84);
        time->setWeight(600);
        time->setGeometry({10, 0, 260, 115});
        Label* date = addChild<Label>("Thu Oct 15");
        date->setFontSize(20);
        date->setWeight(600);
        date->setGeometry({10, 120, 200, 28});
    }
};

TEST(EmbeddedGpuTest, LargeTextMatchesTheCpu)
{
    GlHost host;
    if (!host.isValid())
        GTEST_SKIP() << "no surfaceless EGL";
    std::unique_ptr<EmbeddedGpu> gpu = EmbeddedGpu::createForCurrentContext();
    ASSERT_TRUE(gpu);
    for (const double scale : {2.0, 3.0}) {
        EmbeddedWindow window;
        window.setContentView(std::make_unique<Clock>());
        window.setScale(scale);
        window.setSize({280, 150});
        const int width = window.pixelWidth();
        const int height = window.pixelHeight();
        const GLuint texture = GlHost::texture(width, height);
        ASSERT_NE(window.render(*gpu, {texture, width, height, false}, 1000000000), nullptr);
        const std::vector<uint8_t> gpuPixels = GlHost::read(texture, width, height);
        window.requestPaint();
        const EmbeddedWindow::Frame& cpu = window.render(1016000000);
        const Difference difference = compare(gpuPixels.data(), cpu.pixels, size_t(width) * size_t(height));
        fprintf(stderr, "large text at %g: largest difference %d, %zu pixels over 8\n", scale, difference.maximum,
                difference.large);
        EXPECT_LE(difference.maximum, 8) << scale;
        glDeleteTextures(1, &texture);
    }
}

TEST(EmbeddedGpuTest, FallsBackWhenItCannotDraw)
{
    GlHost host;
    if (!host.isValid())
        GTEST_SKIP() << "no surfaceless EGL";
    std::unique_ptr<EmbeddedGpu> gpu = EmbeddedGpu::createForCurrentContext();
    ASSERT_TRUE(gpu);
    EmbeddedWindow window;
    window.setContentView(std::make_unique<Panel>());
    window.setSize({100, 50});
    const GLuint texture = GlHost::texture(100, 50);

    // A texture of another size.
    EXPECT_EQ(window.render(*gpu, {texture, 100, 51, false}), nullptr);
    // No context current.
    host.release();
    EXPECT_EQ(window.render(*gpu, {texture, 100, 50, false}), nullptr);
    host.makeCurrent();
    EXPECT_NE(window.render(*gpu, {texture, 100, 50, false}), nullptr);
    // A lost context: the CPU still renders.
    gpu->release();
    EXPECT_TRUE(gpu->isAbandoned());
    window.requestPaint();
    EXPECT_EQ(window.render(*gpu, {texture, 100, 50, true}), nullptr);
    EXPECT_NE(window.render().pixels, nullptr);
    glDeleteTextures(1, &texture);
}

TEST(EmbeddedGpuTest, BatchRendersSeveralWindowsWithOneReset)
{
    GlHost host;
    if (!host.isValid())
        GTEST_SKIP() << "no surfaceless EGL";
    std::unique_ptr<EmbeddedGpu> gpu = EmbeddedGpu::createForCurrentContext();
    ASSERT_TRUE(gpu);

    struct Target
    {
        std::unique_ptr<EmbeddedWindow> window;
        Panel* panel = nullptr;
        GLuint texture = 0;
    };
    std::vector<Target> targets;
    for (const double scale : {1.0, 1.5, 1.75}) {
        Target target;
        target.window = std::make_unique<EmbeddedWindow>();
        auto content = std::make_unique<Panel>();
        target.panel = content.get();
        target.window->setContentView(std::move(content));
        target.window->setScale(scale);
        target.window->setSize({300, 200});
        target.texture = GlHost::texture(target.window->pixelWidth(), target.window->pixelHeight());
        targets.push_back(std::move(target));
    }

    // The host's own state, which only the batch's end restores.
    const GLuint own = GlHost::texture(64, 64);
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, own, 0);
    glViewport(3, 4, 50, 60);
    glEnable(GL_SCISSOR_TEST);
    glScissor(1, 2, 30, 40);

    for (int pass = 0; pass < 2; ++pass) {
        ASSERT_TRUE(gpu->beginBatch());
        EXPECT_TRUE(gpu->inBatch());
        for (Target& target : targets) {
            if (pass == 1) {
                target.panel->fill = Color{255, 255, 255, 120};
                target.panel->requestPaint();
            }
            const int width = target.window->pixelWidth();
            const int height = target.window->pixelHeight();
            const EmbeddedWindow::GpuTexture texture{target.texture, width, height, pass == 1};
            const EmbeddedWindow::Frame* frame = target.window->render(*gpu, texture, 1000000000 + pass * 16000000);
            ASSERT_NE(frame, nullptr);
            // The damage is filled for the host's scene.
            EXPECT_FALSE(frame->damage.empty());
        }
        gpu->endBatch();
        EXPECT_FALSE(gpu->inBatch());
        EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));

        GLint value = -1;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
        EXPECT_EQ(value, GLint(framebuffer));
        GLint box[4] = {};
        glGetIntegerv(GL_VIEWPORT, box);
        EXPECT_EQ(std::vector<GLint>(box, box + 4), (std::vector<GLint>{3, 4, 50, 60}));
        glGetIntegerv(GL_SCISSOR_BOX, box);
        EXPECT_EQ(std::vector<GLint>(box, box + 4), (std::vector<GLint>{1, 2, 30, 40}));
        glGetIntegerv(GL_CURRENT_PROGRAM, &value);
        EXPECT_EQ(value, 0);
        EXPECT_FALSE(glIsEnabled(GL_BLEND));

        for (Target& target : targets) {
            const int width = target.window->pixelWidth();
            const int height = target.window->pixelHeight();
            const std::vector<uint8_t> pixels = GlHost::read(target.texture, width, height);
            target.window->requestPaint();
            const EmbeddedWindow::Frame& cpu = target.window->render(1100000000 + pass * 16000000);
            const Difference difference = compare(pixels.data(), cpu.pixels, size_t(width) * size_t(height));
            EXPECT_LT(difference.large, size_t(width * height) / 200) << target.window->scale();
        }
    }

    // Not current: no batch, and renders refuse as before.
    host.release();
    EXPECT_FALSE(gpu->beginBatch());
    gpu->endBatch();
    host.makeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &own);
    for (Target& target : targets)
        glDeleteTextures(1, &target.texture);
    targets.clear();
}

TEST(EmbeddedGpuTest, ReportsAndTrimsItsCache)
{
    GlHost host;
    if (!host.isValid())
        GTEST_SKIP() << "no surfaceless EGL";
    std::unique_ptr<EmbeddedGpu> gpu = EmbeddedGpu::createForCurrentContext();
    ASSERT_TRUE(gpu);
    {
        EmbeddedWindow window;
        window.setContentView(std::make_unique<Panel>());
        window.setScale(1.5);
        window.setSize({300, 200});
        const GLuint texture = GlHost::texture(window.pixelWidth(), window.pixelHeight());
        ASSERT_NE(window.render(*gpu, {texture, window.pixelWidth(), window.pixelHeight(), false}), nullptr);
        glDeleteTextures(1, &texture);
    }
    const EmbeddedGpu::CacheUsage used = gpu->cacheUsage();
    EXPECT_GT(used.resources, 0);
    EXPECT_GT(used.bytes, 0u);
    EXPECT_LE(used.purgeableBytes, used.bytes);
    gpu->performDeferredCleanup(std::chrono::milliseconds(0));
    EXPECT_LE(gpu->cacheUsage().bytes, used.bytes);
    gpu->setCacheLimit(64 << 20);
    gpu->release();
    EXPECT_EQ(gpu->cacheUsage().bytes, 0u);
}
