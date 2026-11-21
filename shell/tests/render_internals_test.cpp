// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GLES3/gl3.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#include "egl_test_util.h"
#include "pixel_test_util.h"
#include "render/gl_program.h"
#include "base/region.h"
#include "base/strings.h"
#include "ui/geometry_conversions.h"

using namespace Lingmo;
using namespace Lingmo::Render;

namespace {

constexpr char vertex[] =
        "attribute vec2 position; void main() { gl_Position = vec4(position, 0., 1.); }";
constexpr char fragment[] = "precision mediump float; void main() { gl_FragColor = vec4(1.); }";
constexpr char invalid[] = "this is not GLSL";

class RenderInternals : public testing::Test
{
protected:
    void SetUp() override
    {
        renderer = EglRendererTestAccess::create();
        ASSERT_TRUE(renderer);
        const EGLint attributes[] = {EGL_WIDTH, 32, EGL_HEIGHT, 32, EGL_NONE};
        const auto surface =
                eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
        ASSERT_NE(surface, EGL_NO_SURFACE);
        target = std::make_unique<EglRenderTarget>(renderer->eglDisplay(), surface, nullptr,
                                                   base::Size(32, 32));
        renderer->setOutputContext({"test", base::Rect(10, 20, 32, 32), {32, 32}, 1, 0, 1});
        ASSERT_TRUE(renderer->beginFrame(target.get()));
    }

    void TearDown() override
    {
        if (renderer && target)
            renderer->releaseTarget(target.get());
        target.reset();
        renderer.reset();
    }

    std::unique_ptr<EglRenderer> renderer;
    std::unique_ptr<EglRenderTarget> target;
};

TEST(ProgramInitialization, CompileLinkFailureAndMoveOwnership)
{
    auto context = EglContext::createSurfaceless();
    ASSERT_TRUE(context);
    EXPECT_FALSE(GlProgram::create(invalid, fragment));
    EXPECT_FALSE(GlProgram::create(vertex, invalid));
    EXPECT_FALSE(GlProgram::create(
            "varying vec2 value; void main() { value = vec2(1.); gl_Position = vec4(0.); }",
            "precision mediump float; varying vec3 value; void main() { gl_FragColor = vec4(value, "
            "1.); }"));
    auto program = GlProgram::create(vertex, fragment);
    ASSERT_TRUE(program);
    const GLuint name = program;
    EXPECT_GE(program.attribute("position"), 0);
    EXPECT_EQ(program.attribute("position"), glGetAttribLocation(name, "position"));
    EXPECT_EQ(program.uniform("absent"), -1);
    auto moved = std::move(program);
    EXPECT_FALSE(program);
    EXPECT_TRUE(glIsProgram(name));
    moved.reset();
    EXPECT_FALSE(glIsProgram(name));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST(ProgramInitialization, PartialFailureCanRetryAndTeardownWithoutCurrentContext)
{
    auto renderer = EglRendererTestAccess::uninitialized();
    ASSERT_TRUE(renderer);
    for (int attempt = 0; attempt < 3; ++attempt) {
        EXPECT_FALSE(EglRendererTestAccess::initialize(*renderer, invalid));
        EXPECT_FALSE(EglRendererTestAccess::hasPrograms(*renderer));
    }
    ASSERT_TRUE(EglRendererTestAccess::initialize(*renderer));
    EXPECT_TRUE(EglRendererTestAccess::hasPrograms(*renderer));
    ASSERT_TRUE(
            eglMakeCurrent(renderer->eglDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    renderer.reset();
    EXPECT_EQ(eglGetCurrentContext(), EGL_NO_CONTEXT);

    renderer = EglRendererTestAccess::uninitialized();
    ASSERT_TRUE(renderer);
    EXPECT_FALSE(EglRendererTestAccess::initialize(*renderer, invalid));
    ASSERT_TRUE(
            eglMakeCurrent(renderer->eglDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    renderer.reset();
    EXPECT_EQ(eglGetCurrentContext(), EGL_NO_CONTEXT);
}

TEST(ProgramInitialization, BlurShaderFailureStillGivesARenderer)
{
    auto renderer = EglRendererTestAccess::uninitialized();
    ASSERT_TRUE(renderer);
    ASSERT_TRUE(EglRendererTestAccess::initialize(*renderer, nullptr, nullptr, invalid));
    EXPECT_FALSE(EglRendererTestAccess::hasBackdrop(*renderer));
    const EGLint attributes[] = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE};
    const auto surface =
            eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attributes);
    ASSERT_NE(surface, EGL_NO_SURFACE);
    EglRenderTarget target(renderer->eglDisplay(), surface, nullptr, base::Size(16, 16));
    renderer->setOutputContext({"test", base::Rect(0, 0, 16, 16), {16, 16}, 1, 0, 1});
    ASSERT_TRUE(renderer->beginFrame(&target));
    const auto image = renderer->captureFrame([&] {
        renderer->clear({0, 1, 0, 1});
        renderer->drawBackgroundBlur({0, 0, 16, 16}, 8, {0, 0, 16, 16}, {}, {});
    });
    renderer->releaseTarget(&target);
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(image.pixel(0, 0), test::kGreen);
}

TEST(ProgramInitialization, InvalidConfigDoesNotPreventLaterInitialization)
{
    EXPECT_FALSE(EglContext::createSurfaceless(1000));
    auto renderer = EglRendererTestAccess::create();
    ASSERT_TRUE(renderer);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, FailedReplacementPreservesDrawingAndOptionalSamplerCanFail)
{
    EXPECT_FALSE(EglRendererTestAccess::initialize(*renderer, invalid));
    EXPECT_TRUE(EglRendererTestAccess::hasPrograms(*renderer));
    auto image = renderer->captureFrame([&] {
        renderer->clear({0, 1, 0, 1});
    });
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(image.pixel(0, 0), test::kGreen);
    ASSERT_TRUE(EglRendererTestAccess::initialize(*renderer, nullptr, invalid));
    EXPECT_FALSE(EglRendererTestAccess::hasExternalProgram(*renderer));
    const uint8_t pixel[] = {255, 0, 0, 255};
    auto texture = renderer->createTexture({1, 1}, pixel);
    ASSERT_TRUE(texture);
    image = renderer->captureFrame([&] {
        renderer->clear({0, 0, 0, 1});
        renderer->drawTexture(texture.get(), {10, 20, 32, 32}, 1);
    });
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(image.pixel(16, 16), test::kRed);
    EXPECT_FALSE(renderer->importDmaBuf({}));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, RenderToImageReadsUprightPixelsAndKeepsTheFrame)
{
    const uint8_t pixels[] = {255, 0, 0, 255, 0, 0, 255, 255};
    auto texture = renderer->createTexture({2, 1}, pixels);
    ASSERT_TRUE(texture);
    const auto image = renderer->renderToImage({8, 4}, [&] {
        renderer->drawTextureF(texture.get(), base::RectF(0, 0, 8, 2), 1);
    });
    ASSERT_EQ(image.size(), base::Size(8, 4));
    EXPECT_EQ(image.format(), ui::Bitmap::Format::Argb32Premultiplied);
    EXPECT_EQ(image.pixel(1, 0), test::kRed);
    EXPECT_EQ(image.pixel(6, 1), test::kBlue);
    EXPECT_EQ(test::alpha(image.pixel(6, 3)), 0);
    EXPECT_EQ(renderer->outputContext().logicalViewport, base::Rect(10, 20, 32, 32));
    EXPECT_EQ(renderer->viewportSize(), base::Size(32, 32));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, APartialUploadTakesOnlyItsDamageFromPaddedRows)
{
    // Eight pixels a row, ten apart, the way clients pad their rows.
    constexpr int stride = 10 * 4;
    std::vector<uint8_t> red(stride * 4), blue(stride * 4);
    for (size_t i = 0; i < red.size(); i += 4) {
        red[i] = 255;
        red[i + 3] = 255;
        blue[i + 2] = 255;
        blue[i + 3] = 255;
    }
    auto texture = renderer->createTexture({8, 4}, red.data(), stride, PixelFormat::Rgba8888);
    ASSERT_TRUE(texture);
    const base::Region damage = base::Region(base::Rect(2, 1, 3, 2)) + base::Rect(7, 3, 1, 1);
    ASSERT_TRUE(renderer->updateTexture(texture.get(), blue.data(), stride, PixelFormat::Rgba8888,
                                        damage));
    const auto image = renderer->renderToImage({8, 4}, [&] {
        renderer->drawTextureF(texture.get(), base::RectF(0, 0, 8, 4), 1);
    });
    ASSERT_EQ(image.size(), base::Size(8, 4));
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 8; ++x) {
            EXPECT_EQ(image.pixel(x, y), damage.contains(base::Point(x, y)) ? test::kBlue : test::kRed)
                    << x << "," << y;
        }
    }
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, TransformedTextureKeepsAFrameUprightAcrossOutputRotations)
{
    const uint8_t pixels[] = {255, 0, 0, 255, 0, 0, 255, 255};
    auto texture = renderer->createTexture({2, 1}, pixels);
    ASSERT_TRUE(texture);
    const base::RectF viewport(10, 20, 32, 32);
    // A frame captured unrotated, turned by the difference as ScreenTransitionNode does.
    for (const auto &[transform, degrees] : {std::pair {1u, 90}, {2u, -180}, {3u, -90}}) {
        renderer->setOutputContext({"test", base::Rect(10, 20, 32, 32), {32, 32}, 1, transform, 1});
        base::AffineTransform turn;
        turn.translate(viewport.center().x(), viewport.center().y());
        turn.rotate(degrees);
        turn.translate(-viewport.center().x(), -viewport.center().y());
        const auto image = renderer->captureFrame([&] {
            renderer->clear({0, 0, 0, 1});
            renderer->drawTextureTransformed(texture.get(), viewport, turn, 1);
        });
        ASSERT_FALSE(image.isNull());
        EXPECT_EQ(image.pixel(4, 16), test::kRed) << transform;
        EXPECT_EQ(image.pixel(28, 16), test::kBlue) << transform;
    }
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, BackdropSamplesWhatIsUnderItOnARotatedOutput)
{
    const base::Rect layout(0, 0, 48, 32);
    const auto paint = [&](const GlassParams &glass) {
        renderer->clear({0, 0, 0, 1});
        renderer->fillRect({0, 0, 24, 16}, {1, 0, 0, 1});
        renderer->fillRect({24, 0, 24, 16}, {0, 1, 0, 1});
        renderer->fillRect({0, 16, 24, 16}, {0, 0, 1, 1});
        renderer->fillRect({30, 20, 6, 12}, {1, 1, 1, 1});
        renderer->drawBackgroundBlur({8, 4, 28, 22}, 4, layout, glass, {});
    };
    GlassParams glass;
    glass.enabled = true;
    for (const auto &params : {GlassParams {}, glass}) {
        renderer->setOutputContext({"test", layout, layout.size(), 1, 0, 1});
        const auto upright = renderer->captureImage(renderer->outputContext(), layout.size(),
                                                    [&] { paint(params); });
        ASSERT_FALSE(upright.isNull());
        for (const uint32_t transform : {1u, 2u, 3u}) {
            const base::Size physical = transform % 2 ? layout.size().transposed() : layout.size();
            renderer->setOutputContext({"test", layout, physical, 1, transform, 1});
            const auto turned = renderer->captureImage(renderer->outputContext(), physical,
                                                       [&] { paint(params); });
            ASSERT_FALSE(turned.isNull());
            int worst = 0;
            for (int y = 0; y < layout.height(); ++y) {
                for (int x = 0; x < layout.width(); ++x) {
                    const base::Point at = transform == 1
                            ? base::Point(y, layout.width() - 1 - x)
                            : transform == 2
                            ? base::Point(layout.width() - 1 - x, layout.height() - 1 - y)
                            : base::Point(layout.height() - 1 - y, x);
                    const uint32_t expected = upright.pixel(x, y);
                    const uint32_t actual = test::pixelAt(turned, at);
                    worst = std::max({worst, std::abs(test::red(expected) - test::red(actual)),
                                      std::abs(test::green(expected) - test::green(actual)),
                                      std::abs(test::blue(expected) - test::blue(actual))});
                }
            }
            EXPECT_LE(worst, 2) << "transform " << transform << " glass " << params.enabled;
        }
    }
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, CachedBackdropMasksAndScratchMatchAFreshDraw)
{
    const base::Rect layout(0, 0, 64, 48);
    const OutputRenderContext context {"test", layout, {80, 60}, 1.25, 0, 1};
    GlassParams glass;
    glass.enabled = true;
    const auto frame = [&](int shift) {
        return renderer->captureImage(context, {80, 60}, [&] {
            renderer->clear({0, 0, 0, 1});
            renderer->fillRect({0, 0, 32, 24}, {1, 0, 0, 1});
            renderer->fillRect({32, 24, 32, 24}, {0, 0, 1, 1});
            ui::Path ellipse;
            const base::Rect lens(4 + shift, 6 + shift, 22, 16);
            ellipse.addEllipse(ui::fromBase(base::RectF(lens)));
            renderer->drawBackgroundBlur(lens, 0, layout, {}, ellipse);
            ui::Path triangle;
            triangle.moveTo(36, 30);
            triangle.lineTo(60, 30);
            triangle.lineTo(48, 46);
            triangle.close();
            renderer->drawBackgroundBlur({36, 30, 24, 16}, 0, layout, glass, triangle);
        });
    };
    const auto fresh = [&](int shift) {
        renderer->releaseTarget(target.get());
        EXPECT_TRUE(renderer->beginFrame(target.get()));
        return frame(shift);
    };
    renderer->setOutputContext(context);
    const ui::Bitmap still = fresh(0);
    const ui::Bitmap moved = fresh(3);
    ASSERT_FALSE(still.isNull());
    ASSERT_NE(still, moved);
    for (const int shift : {0, 3, 0, 3, 0})
        EXPECT_EQ(frame(shift), shift ? moved : still) << shift;
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, APartialPassRedrawsOnlyItsRectAndKeepsTheRest)
{
    std::unique_ptr<Texture> texture;
    const base::Rect bounds(10, 20, 16, 8);
    ASSERT_TRUE(renderer->renderToTexture(texture, bounds, [&] {
        renderer->fillRect(bounds, {1, 0, 0, 1});
    }));
    // The top right quarter only, which also pins the bottom-up row order.
    ASSERT_TRUE(renderer->renderToTexture(texture, bounds, base::Rect(18, 20, 8, 4), [&] {
        renderer->fillRect(bounds, {0, 0, 1, 1});
    }));
    const auto image = renderer->captureFrame([&] {
        renderer->clear({0, 0, 0, 1});
        renderer->drawTexture(texture.get(), bounds, 1);
    });
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(image.pixel(12, 1), test::kBlue);
    EXPECT_EQ(image.pixel(12, 6), test::kRed);
    EXPECT_EQ(image.pixel(2, 1), test::kRed);
    EXPECT_EQ(image.pixel(2, 6), test::kRed);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

// A nested pass gives back the target it drew over: framebuffer, viewport and
// scissor, with the renderer's own geometry and presentation state. Every
// draw sets its program, textures, blend and attributes itself, so a pass
// neither reads nor restores those.
TEST_F(RenderInternals, NestedPassAndExceptionRestoreGeometryPresentationAndTarget)
{
    PresentationSubmission submission;
    renderer->presentationSubmission = &submission;
    renderer->setRasterScale(1.25);
    const auto context = renderer->outputContext();
    glViewport(2, 3, 23, 17);
    glEnable(GL_SCISSOR_TEST);
    glScissor(4, 5, 6, 7);

    const auto check = [&] {
        EXPECT_EQ(renderer->viewportSize(), base::Size(32, 32));
        EXPECT_EQ(renderer->outputContext().logicalViewport, context.logicalViewport);
        EXPECT_EQ(renderer->rasterScale(), 1.25);
        EXPECT_EQ(renderer->presentationSubmission, &submission);
        EXPECT_FALSE(renderer->capturing);
        GLint value, values[4];
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
        EXPECT_EQ(value, 0);
        glGetIntegerv(GL_VIEWPORT, values);
        EXPECT_EQ(values[0], 2);
        EXPECT_EQ(values[1], 3);
        EXPECT_EQ(values[2], 23);
        EXPECT_EQ(values[3], 17);
        glGetIntegerv(GL_SCISSOR_BOX, values);
        for (int i = 0; i < 4; ++i)
            EXPECT_EQ(values[i], 4 + i);
        EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    };

    std::unique_ptr<Texture> outer, inner;
    ASSERT_TRUE(renderer->renderToTexture(outer, {11, 21, 12, 12}, [&] {
        EXPECT_EQ(renderer->presentationSubmission, nullptr);
        const base::Size viewport = renderer->viewportSize();
        GLint framebuffer;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
        ASSERT_TRUE(renderer->renderToTexture(inner, {12, 22, 4, 4}, [&] {
            renderer->fillRect({12, 22, 4, 4}, {0, 1, 0, 1});
        }));
        GLint restored;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &restored);
        EXPECT_EQ(restored, framebuffer);
        EXPECT_EQ(renderer->viewportSize(), viewport);
    }));
    check();
    EXPECT_THROW(renderer->renderToTexture(outer, {11, 21, 12, 12},
                                           [&] {
                                               renderer->renderToTexture(inner, {12, 22, 4, 4}, [] {
                                                   throw std::runtime_error("paint");
                                               });
                                           }),
                 std::runtime_error);
    check();
    EXPECT_THROW(renderer->captureFrame([] {
        throw std::runtime_error("capture");
    }),
                 std::runtime_error);
    check();
    glDisable(GL_SCISSOR_TEST);
    renderer->presentationSubmission = nullptr;
}

// A pixel transfer buffer left bound would turn the pass's uploads and
// readbacks into buffer offsets, so a pass drops both bindings. The array
// buffer and attributes beyond GlProgram::kAttributeCount are never touched.
TEST_F(RenderInternals, ReadbackUnbindsTransferBuffersAndKeepsForeignState)
{
    if (!renderer->glesVersion().starts_with("OpenGL ES 3"))
        GTEST_SKIP() << "ES3 transfer-buffer state requires an ES3 context";
    const uint8_t pixel[] = {255, 0, 0, 255};
    auto texture = renderer->createTexture({1, 1}, pixel);
    ASSERT_TRUE(texture);
    GLuint readFramebuffer = 0;
    glGenFramebuffers(1, &readFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           static_cast<GlTexture *>(texture.get())->name(), 0);
    ASSERT_EQ(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
    GLuint buffers[3];
    glGenBuffers(3, buffers);
    const GLenum targets[] = {GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_ARRAY_BUFFER};
    for (int i = 0; i < 3; ++i) {
        glBindBuffer(targets[i], buffers[i]);
        glBufferData(targets[i], 4096, nullptr, GL_STREAM_DRAW);
    }
    glVertexAttribIPointer(7, 2, GL_INT, 0, nullptr);
    glEnableVertexAttribArray(7);
    auto image = renderer->captureFrame([&] {
        renderer->clear({0, 1, 0, 1});
    });
    ASSERT_EQ(image.size(), base::Size(32, 32));
    EXPECT_EQ(image.pixel(16, 16), test::kGreen);
    GLint value = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &value);
    EXPECT_EQ(GLuint(value), readFramebuffer);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &value);
    EXPECT_EQ(value, 0);
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &value);
    EXPECT_EQ(value, 0);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &value);
    EXPECT_EQ(value, 0);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &value);
    EXPECT_EQ(GLuint(value), buffers[2]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGetVertexAttribiv(7, GL_VERTEX_ATTRIB_ARRAY_INTEGER, &value);
    EXPECT_TRUE(value);
    glGetVertexAttribiv(7, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &value);
    EXPECT_TRUE(value);
    glDisableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &readFramebuffer);
    glDeleteBuffers(3, buffers);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, IncompleteAndOversizedTargetsRestoreStateWithoutPainting)
{
    PresentationSubmission submission;
    renderer->presentationSubmission = &submission;
    glEnable(GL_SCISSOR_TEST);
    glScissor(1, 2, 3, 4);
    std::unique_ptr<Texture> texture = std::make_unique<GlTexture>(0, base::Size(8, 8));
    bool painted = false;
    EXPECT_FALSE(renderer->renderToTexture(texture, {0, 0, 8, 8}, [&] {
        painted = true;
    }));
    GLint limit = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
    EXPECT_FALSE(renderer->renderToTexture(texture, {0, 0, limit + 1, 1}, [&] {
        painted = true;
    }));
    EXPECT_FALSE(painted);
    EXPECT_EQ(renderer->presentationSubmission, &submission);
    EXPECT_EQ(renderer->viewportSize(), base::Size(32, 32));
    GLint framebuffer = -1, scissor[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
    EXPECT_EQ(framebuffer, 0);
    glGetIntegerv(GL_SCISSOR_BOX, scissor);
    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(scissor[i], i + 1);
    EXPECT_TRUE(glIsEnabled(GL_SCISSOR_TEST));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
    renderer->presentationSubmission = nullptr;
}

TEST_F(RenderInternals, TextureOutlivesRendererAndRetirementRestoresForeignContext)
{
    const uint8_t pixel[] = {255, 0, 0, 255};
    auto texture = renderer->createTexture({1, 1}, pixel);
    ASSERT_TRUE(texture);
    renderer->releaseTarget(target.get());
    target.reset();
    const EGLDisplay display = renderer->eglDisplay();
    const EGLint attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    const EGLContext foreign =
            eglCreateContext(display, renderer->eglConfig(), EGL_NO_CONTEXT, attributes);
    ASSERT_NE(foreign, EGL_NO_CONTEXT);
    ASSERT_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, foreign));
    GLuint sentinel;
    glGenTextures(1, &sentinel);
    glBindTexture(GL_TEXTURE_2D, sentinel);
    renderer.reset();
    EXPECT_EQ(eglGetCurrentContext(), foreign);
    EXPECT_TRUE(glIsTexture(sentinel));
    glDeleteTextures(1, &sentinel);
    ASSERT_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    ASSERT_TRUE(eglDestroyContext(display, foreign));
    texture.reset();
    EXPECT_EQ(eglGetCurrentContext(), EGL_NO_CONTEXT);
}

TEST_F(RenderInternals, UploadsWorkAfterTheLastTargetRetires)
{
    renderer->releaseTarget(target.get());
    target.reset();
    ASSERT_TRUE(eglMakeCurrent(renderer->eglDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE,
                               EGL_NO_CONTEXT));
    const uint8_t red[] = {255, 0, 0, 255};
    auto texture = renderer->createTexture({1, 1}, red);
    ASSERT_TRUE(texture);
    const uint8_t green[] = {0, 255, 0, 255};
    EXPECT_TRUE(renderer->updateTexture(texture.get(), green, 4, PixelFormat::Rgba8888));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST(ProgramInitialization, RetiredTextureCannotTerminateAReplacementDisplay)
{
    auto oldContext = EglContext::createSurfaceless();
    ASSERT_TRUE(oldContext);
    const uint8_t pixel[] = {255, 0, 0, 255};
    std::unique_ptr<Texture> oldTexture;
    {
        GlTextureUploader oldUploader(oldContext);
        oldTexture = oldUploader.create({1, 1}, pixel, 4, PixelFormat::Rgba8888);
    }
    ASSERT_TRUE(oldTexture);
    auto replacement = EglRendererTestAccess::create();
    ASSERT_TRUE(replacement);
    EXPECT_EQ(replacement->eglDisplay(), oldContext->display());
    const EGLContext current = eglGetCurrentContext();
    oldContext.reset();
    oldTexture.reset();
    EXPECT_EQ(eglGetCurrentContext(), current);
    EXPECT_FALSE(EglContext::createSurfaceless(1000));
    EXPECT_EQ(eglGetCurrentContext(), current);
    EXPECT_TRUE(GlProgram::create(vertex, fragment));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(RenderInternals, AMarkTellsWhenTheGpuGotPastTheFrame)
{
    if (!renderer->marksDrawn())
        GTEST_SKIP() << "No GL_EXT_disjoint_timer_query";
    EXPECT_FALSE(renderer->drawnAt({}));
    const auto before = std::chrono::steady_clock::now();
    renderer->clear({0, 1, 0, 1});
    const auto mark = renderer->markDrawn();
    ASSERT_TRUE(mark.isValid());
    glFinish();
    const auto after = std::chrono::steady_clock::now();
    const auto drawn = renderer->drawnAt(mark);
    ASSERT_TRUE(drawn);
    // The two clocks are matched by reading both, which takes a little while.
    const auto slack = std::chrono::milliseconds(2);
    EXPECT_GE(*drawn, before - slack);
    EXPECT_LE(*drawn, after + slack);
    // Outside a frame there is nothing to mark.
    renderer->cancelFrame();
    EXPECT_FALSE(renderer->markDrawn().isValid());
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST(EglNativeFence, AcquirePreservesDescriptorsAndReleaseRequiresAFrame)
{
    const char *path = std::getenv("LINGMO_TEST_DRM_DEVICE");
    if (!path || !*path)
        GTEST_SKIP() << "Set LINGMO_TEST_DRM_DEVICE to test native EGL fences";
    Lingmo::ScopedFd fd(open(path, O_RDWR | O_CLOEXEC));
    ASSERT_TRUE(fd.isValid());
    std::unique_ptr<gbm_device, decltype(&gbm_device_destroy)> device(gbm_create_device(fd.get()),
                                                                      gbm_device_destroy);
    ASSERT_TRUE(device);
    auto renderer = EglRenderer::create(device.get(), DRM_FORMAT_XRGB8888);
    ASSERT_TRUE(renderer);
    const auto extensions =
            base::strings::split(eglQueryString(renderer->eglDisplay(), EGL_EXTENSIONS), ' ');
    if (std::find(extensions.begin(), extensions.end(), "EGL_ANDROID_native_fence_sync")
        == extensions.end())
        GTEST_SKIP() << "EGL native fences are unavailable";
    std::unique_ptr<gbm_surface, decltype(&gbm_surface_destroy)> surface(
            gbm_surface_create(device.get(), 32, 32, DRM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING),
            gbm_surface_destroy);
    ASSERT_TRUE(surface);
    auto target = renderer->createTarget(surface.get(), {32, 32});
    ASSERT_TRUE(target);
    TargetRetirement retire {renderer.get(), target.get()};
    EXPECT_FALSE(renderer->createReleaseFence().isValid());
    ASSERT_TRUE(renderer->beginFrame(target.get()));
    renderer->clear({0, 1, 0, 1});
    auto fence = renderer->createReleaseFence();
    ASSERT_TRUE(fence.isValid());
    auto duplicate = fence.duplicate();
    ASSERT_TRUE(duplicate.isValid());
    EXPECT_NE(duplicate.get(), fence.get());
    renderer->waitFence(fence);
    EXPECT_GE(fcntl(fence.get(), F_GETFD), 0);
    EXPECT_TRUE(duplicate.wait(500));
    renderer->cancelFrame();
    EXPECT_FALSE(renderer->createReleaseFence().isValid());
    renderer->waitFence(fence);
    EXPECT_GE(fcntl(fence.get(), F_GETFD), 0);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

} // namespace

TEST(EglExtensions, WholeNamesOnlyAsTheSplitListWould)
{
    auto context = EglContext::createSurfaceless();
    ASSERT_TRUE(context);
    ASSERT_TRUE(context->makeCurrent());
    const std::vector<std::string> names =
            base::strings::split(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)), ' ');
    ASSERT_GT(names.size(), 5u);
    for (const std::string &name : names) {
        if (name.empty())
            continue;
        EXPECT_TRUE(context->supportsGl(name.c_str())) << name;
        // A prefix of a name is not that name, unless it is one itself.
        const std::string prefix = name.substr(0, name.size() - 1);
        EXPECT_EQ(std::find(names.begin(), names.end(), prefix) != names.end(),
                  context->supportsGl(prefix.c_str()))
                << prefix;
    }
    EXPECT_FALSE(context->supportsGl(""));
    EXPECT_FALSE(context->supportsGl("GL_LINGMO_not_an_extension"));
}
