
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

#include <Legion/Common/Util/Assert.hpp>
#include <Legion/Common/Util/GL.hpp>

#include <iostream>
#include <sstream>

using namespace legion;

#define GL_CHECK_CALL( call )                                                  \
do {                                                                           \
    call;                                                                      \
    GL::checkError();                                                          \
} while( false )


namespace
{
const char* getGLErrorString( GLenum error )
{
  switch( error )
  {
    case GL_NO_ERROR:
      return "No error";
    case GL_INVALID_ENUM:
      return "Invalid enum";
    case GL_INVALID_VALUE:
      return "Invalid value";
    case GL_INVALID_OPERATION:
      return "Invalid operation";
    case GL_STACK_OVERFLOW:
      return "Stack overflow";
    case GL_STACK_UNDERFLOW:
      return "Stack underflow";
    case GL_OUT_OF_MEMORY:
      return "Out of memory";
    case GL_TABLE_TOO_LARGE:
      return "Table too large";
    default:
      return "Unknown GL error";
  }
}

} // namespace anon


void GL::checkError()
{
    GLenum err = GL::GetError();
    std::cerr << "ERROR: " << err << std::endl;
    if( err != GL_NO_ERROR )
    {
        std::ostringstream oss;
        do
        {
            oss << "GL error: " << getGLErrorString( err ) << "\n";
            err = GL::GetError();
        } 
        while( err != GL_NO_ERROR );

        throw legion::AssertionFailure( oss.str() );
    }
}


GL::GL()
{
#if defined( _WIN32 )
#    define GET_PROC_ADDR( name ) ( ( PROC( * )( LPCSTR ) )( getProcAddress ) )( #name )

  char system_dir_path[MAX_PATH];
  if( !GetSystemDirectory( system_dir_path, MAX_PATH ) )
      throw legion::AssertionFailure( "GetSystemDirectory failed" );

  const std::string gl_path = std::string( system_dir_path ) + "/Opengl32.dll";
  if( !m_gl_lib.open( path.str().c_str() ) )
      throw legion::AssertionFailure( "Failed to open GL lib: '" + gl_path + "'" );

  const void* getProcAddress = m_gl_lib.getSymbol( "wglGetProcAddress" );
  if( !getProcAddress )
      throw legion::AssertionFailure( "Failed to load 'wglGetProcAddress'" );

#elif defined( __linux__ )
#    define GET_PROC_ADDR( name ) ( ( void* (*)(const GLubyte*))( getProcAddress ) )( (const GLubyte*)#name )


//  if( !m_gl_lib.open( "libGL.so" ) )
  if( !m_gl_lib.open( "/usr/lib/x86_64-linux-gnu/libGL.so" ) )
      throw legion::AssertionFailure( "Failed to open GL lib: 'libGL.so'" );

  const void* getProcAddress = m_gl_lib.getSymbol( "glXGetProcAddressARB" );
  if( !getProcAddress )
      throw legion::AssertionFailure( "Failed to load 'glXGetProcAddressARB'" );
#else  // MAC
#    define GET_PROC_ADDR( name ) (::name )
#endif

      p_glGetError        = ( glGetError_t )( GET_PROC_ADDR( glGetError ) );

      p_glGenTextures     = ( glGenTextures_t   )( GET_PROC_ADDR( glGenTextures   ) );
      p_glTexParameteri   = ( glTexParameteri_t )( GET_PROC_ADDR( glTexParameteri ) );
      p_glBindTexture     = ( glBindTexture_t   )( GET_PROC_ADDR( glBindTexture   ) );
      p_glTexImage2D      = ( glTexImage2D_t    )( GET_PROC_ADDR( glTexImage2D    ) );
      p_glPixelStorei     = ( glPixelStorei_t   )( GET_PROC_ADDR( glPixelStorei   ) );

      p_glGenBuffers      = ( glGenBuffers_t    )( GET_PROC_ADDR( glGenBuffers    ) );
      p_glBindBuffer      = ( glBindBuffer_t    )( GET_PROC_ADDR( glBindBuffer    ) );
      p_glBufferData      = ( glBufferData_t    )( GET_PROC_ADDR( glBufferData    ) );

      p_glEnable          = ( glEnable_t        )( GET_PROC_ADDR( glEnable        ) );
      p_glDisable         = ( glDisable_t       )( GET_PROC_ADDR( glDisable       ) );
      p_glBegin           = ( glBegin_t         )( GET_PROC_ADDR( glBegin         ) );
      p_glEnd             = ( glEnd_t           )( GET_PROC_ADDR( glEnd           ) );
      p_glTexCoord2f      = ( glTexCoord2f_t    )( GET_PROC_ADDR( glTexCoord2f    ) );
      p_glVertex2f        = ( glVertex2f_t      )( GET_PROC_ADDR( glVertex2f      ) );

#undef GET_PROC_ADDR
}


GL::~GL()
{
}


GLenum GL::GetError()
{
    GLenum result = instance().p_glGetError();
    return result;
}


void GL::GenTextures( GLsizei n, GLuint* ids )
{
    GL_CHECK_CALL( instance().p_glGenTextures( n, ids ) );
}


void GL::TexParameteri( GLenum target, GLenum pname, GLint param )
{
    GL_CHECK_CALL( instance().p_glTexParameteri( target, pname, param ) );
}


void GL::BindTexture( GLenum target, GLuint texture )
{
    GL_CHECK_CALL( instance().p_glBindTexture( target, texture ) );
}


void GL::TexImage2D(
        GLenum target,
        GLint level,
        GLint internalformat,
        GLsizei width,
        GLsizei height,
        GLint border,
        GLenum format,
        GLenum type,
        const GLvoid* data
        )
{
    GL_CHECK_CALL( instance().p_glTexImage2D( target, level, internalformat, width, height, border, format, type, data ) );
}


void GL::PixelStorei( GLenum pname, GLint param )
{
    GL_CHECK_CALL( instance().p_glPixelStorei( pname, param ) );
}


void GL::GenBuffers( GLsizei n, GLuint* ids )
{
    std::cerr << "Call glGenBuffers: " << (void*)(instance().p_glGenBuffers) << std::endl;
//    GL_CHECK_CALL( instance().p_glGenBuffers( -1, 0 ) );
    GL_CHECK_CALL( instance().p_glGenBuffers( n, ids ) );
    std::cerr << "IDs: " << *ids << std::endl;
}


void GL::BindBuffer( GLenum target, GLuint buffer )
{
    GL_CHECK_CALL( instance().p_glBindBuffer( target, buffer ) );
}


void GL::BufferData( GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage )
{
    GL_CHECK_CALL( instance().p_glBufferData( target, size, data, usage ) );
}

    
void GL::Enable( GLenum cap )
{
    GL_CHECK_CALL( instance().p_glEnable( cap ) );
}


void GL::Disable( GLenum cap )
{
    GL_CHECK_CALL( instance().p_glDisable( cap ) );
}


void GL::Begin( GLenum mode )
{
    GL_CHECK_CALL( instance().p_glBegin( mode ) );
}


void GL::End()
{
    GL_CHECK_CALL( instance().p_glEnd() );
}


void GL::TexCoord2f( GLfloat s, GLfloat t )
{
    GL_CHECK_CALL( instance().p_glTexCoord2f( s, t ) );
}
