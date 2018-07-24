
// Copyright (C) 2018 R. Keith Morley
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// (MIT/X11 License)

#pragma once

#include <Legion/Common/Util/Preprocessor.hpp>
#include <Legion/Common/Util/SharedObject.hpp>
#include <Legion/Common/Util/Singleton.hpp>

#ifdef __APPLE__
#    include <OpenGL/gl.h>
#else
#    include <GL/gl.h>
#endif


namespace legion
{


class LCLASSAPI GL : public Singleton<GL>
{
    friend class Singleton<GL>;

public:
    LAPI ~GL();

    static void   checkError();

    // GL API entries
    LAPI static GLenum GetError();

    LAPI static void GenTextures( GLsizei n, GLuint* ids );
    LAPI static void TexParameteri( GLenum target, GLenum pname, GLint param );
    LAPI static void BindTexture( GLenum target, GLuint texture );
    LAPI static void TexImage2D(
            GLenum        target,
            GLint         level,
            GLint         internalformat,
            GLsizei       width,
            GLsizei       height,
            GLint         border,
            GLenum        format,
            GLenum        type,
            const GLvoid* data
            );
    LAPI static void PixelStorei( GLenum pname, GLint param );

    LAPI static void GenBuffers( GLsizei n, GLuint* ids );
    LAPI static void BindBuffer( GLenum target, GLuint buffer );
    LAPI static void BufferData( GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage );

    LAPI static void Enable( GLenum cap );
    LAPI static void Disable( GLenum cap );
    LAPI static void Begin( GLenum mode );
    LAPI static void End();
    LAPI static void TexCoord2f( GLfloat s, GLfloat t );
    LAPI static void Vertex2f( GLfloat x, GLfloat y );

private:
    typedef GLenum ( *glGetError_t   )();

    typedef void ( *glGenTextures_t )( GLsizei n, GLuint* ids );
    typedef void ( *glTexParameteri_t )( GLenum target, GLenum pname, GLint param );
    typedef void ( *glBindTexture_t )( GLenum target, GLuint texture );
    typedef void ( *glTexImage2D_t )(
            GLenum        target,
            GLint         level,
            GLint         internalformat,
            GLsizei       width,
            GLsizei       height,
            GLint         border,
            GLenum        format,
            GLenum        type,
            const GLvoid* data
            );
    typedef void ( *glPixelStorei_t )( GLenum pname, GLint param );

    typedef void ( *glGenBuffers_t )( GLsizei n, GLuint* ids );
    typedef void ( *glBindBuffer_t )( GLenum target, GLuint buffer );
    typedef void ( *glBufferData_t )( GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage );

    typedef void ( *glEnable_t     )( GLenum cap );
    typedef void ( *glDisable_t    )( GLenum cap );
    typedef void ( *glBegin_t      )( GLenum mode );
    typedef void ( *glEnd_t        )();
    typedef void ( *glTexCoord2f_t )( GLfloat s, GLfloat t );
    typedef void ( *glVertex2f_t   )( GLfloat x, GLfloat y );


    glGetError_t        p_glGetError        = 0;

    glGenTextures_t     p_glGenTextures     = 0;
    glTexParameteri_t   p_glTexParameteri   = 0;
    glBindTexture_t     p_glBindTexture     = 0;
    glTexImage2D_t      p_glTexImage2D      = 0;
    glPixelStorei_t     p_glPixelStorei     = 0;

    glGenBuffers_t      p_glGenBuffers      = 0;
    glBindBuffer_t      p_glBindBuffer      = 0;
    glBufferData_t      p_glBufferData      = 0;

    glEnable_t          p_glEnable          = 0;
    glDisable_t         p_glDisable         = 0;
    glBegin_t           p_glBegin           = 0;
    glEnd_t             p_glEnd             = 0;
    glTexCoord2f_t      p_glTexCoord2f      = 0;
    glVertex2f_t        p_glVertex2f        = 0;

    SharedObject        m_gl_lib;

    GL();
};

}

