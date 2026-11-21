// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/gl_program.h"

#include <utility>

#include "core/logging.h"
#include "render_shaders.h"

namespace Lingmo::Render {

namespace {

GLuint compileShader(GLenum type, const char *source)
{
    const GLuint shader = glCreateShader(type);
    if (!shader)
        return 0;
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[512] = {};
        glGetShaderInfoLog(shader, sizeof(log) - 1, nullptr, log);
        LM_CRITICAL(lingmoRender).printf("Shader compilation failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint linkProgram(const char *vertexSource, const char *fragmentSource)
{
    const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertex)
        return 0;
    const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragment) {
        glDeleteShader(vertex);
        return 0;
    }

    const GLuint program = glCreateProgram();
    if (!program) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return 0;
    }
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    // Fixed locations, so the state a nested pass saves and restores stops at
    // GlProgram::kAttributeCount instead of every slot the GPU has.
    glBindAttribLocation(program, GlProgram::kPositionAttribute, "aPosition");
    glBindAttribLocation(program, GlProgram::kTexCoordAttribute, "aTexCoord");
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char log[512] = {};
        glGetProgramInfoLog(program, sizeof(log) - 1, nullptr, log);
        LM_CRITICAL(lingmoRender).printf("Program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

} // namespace

GlProgram GlProgram::create(const char *vertex, const char *fragment)
{
    GlProgram program;
    program.m_name = linkProgram(vertex, fragment);
    return program;
}

GlProgram::~GlProgram()
{
    reset();
}

GlProgram::GlProgram(GlProgram &&other) noexcept
    : m_name(std::exchange(other.m_name, 0))
    , m_uniforms(std::move(other.m_uniforms))
    , m_attributes(std::move(other.m_attributes))
{
}

GlProgram &GlProgram::operator=(GlProgram &&other) noexcept
{
    if (this != &other) {
        reset();
        m_name = std::exchange(other.m_name, 0);
        m_uniforms = std::move(other.m_uniforms);
        m_attributes = std::move(other.m_attributes);
    }
    return *this;
}

void GlProgram::reset()
{
    if (m_name)
        glDeleteProgram(std::exchange(m_name, 0));
    m_uniforms.clear();
    m_attributes.clear();
}

GLint GlProgram::uniform(const char *name) const
{
    // Looked up by view, so a hit allocates nothing.
    if (const auto entry = m_uniforms.find(std::string_view(name)); entry != m_uniforms.end())
        return entry->second;
    return m_uniforms.emplace(name, glGetUniformLocation(m_name, name)).first->second;
}

GLint GlProgram::attribute(const char *name) const
{
    if (const auto entry = m_attributes.find(std::string_view(name)); entry != m_attributes.end())
        return entry->second;
    return m_attributes.emplace(name, glGetAttribLocation(m_name, name)).first->second;
}

std::string clippedFragment(const char *body, const char *extensions)
{
    return std::string(extensions) + "\n" + shaders::clipped_precision + shaders::rounded_clip + body
            + shaders::clipped_main;
}

} // namespace Lingmo::Render
