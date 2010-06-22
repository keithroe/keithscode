#include <ruby.h>
#include "optix.h"


static VALUE module_optix;
static VALUE class_variable;
static VALUE class_context;
static VALUE class_program;
static VALUE class_group;
static VALUE class_selector;
static VALUE class_transform;
static VALUE class_geometrygroup;
static VALUE class_acceleration;
static VALUE class_geometryinstance;
static VALUE class_geometry;
static VALUE class_material;
static VALUE class_buffer;
static VALUE class_texturesampler;
static VALUE class_mappedbuffer;


typedef struct { RTvariable variable;                 } VariablePtr;
typedef struct { RTcontext context;                   } ContextPtr;
typedef struct { RTprogram program;                   } ProgramPtr;
typedef struct { RTgroup group;                       } GroupPtr;
typedef struct { RTselector selector;                 } SelectorPtr;
typedef struct { RTtransform transform;               } TransformPtr;
typedef struct { RTgeometrygroup geometrygroup;       } GeometryGroupPtr;
typedef struct { RTacceleration acceleration;         } AccelerationPtr;
typedef struct { RTgeometryinstance geometryinstance; } GeometryInstancePtr;
typedef struct { RTgeometry geometry;                 } GeometryPtr;
typedef struct { RTmaterial material;                 } MaterialPtr;
typedef struct { RTbuffer buffer;                     } BufferPtr;
typedef struct { RTtexturesampler texturesampler;     } TextureSamplerPtr;
typedef struct { RTobject object;                     } ObjectPtr;
typedef struct { void* data; unsigned size;           } MappedBufferPtr;


static RTcontext current_context; // TODO: Make this a list later

static VALUE bufferSetFormat( VALUE self, VALUE format );
static VALUE bufferSetSize1D( VALUE self, VALUE width );
static VALUE bufferSetSize2D( VALUE self, VALUE width, VALUE height );
static VALUE bufferSetSize3D( VALUE self, VALUE width, VALUE height, VALUE depth );

/******************************************************************************\
 *
 * Helpers
 *
\******************************************************************************/

#define RT_CHECK_ERROR( func )                                       \
  do {                                                               \
    RTresult code = func;                                            \
    if( code != RT_SUCCESS )                                         \
      handleError( current_context, code, __FILE__, __LINE__ );      \
  } while(0)

void handleError( RTcontext context, RTresult code, const char* file, int line )
{
  const char* message;
  rtContextGetErrorString(context, code, &message);
  rb_raise(rb_eException, "RT_CHECK_ERROR: %s\n(%s:%d)", message, file, line);
}

void numArray2FltArray( int num_elem, VALUE m, float* mm )
{
  int i;
  VALUE* varray;

  if( RARRAY_LEN( m ) != num_elem ) {
    rb_raise( rb_eException, "setMatrix passed array with bad length: expected %d, got %d\n", num_elem, RARRAY_LEN(m) );
  }

  varray = RARRAY_PTR( m );
  for( i = 0; i < num_elem; ++i ) {
    mm[i] = NUM2DBL( varray[i] );
  }
}

static VALUE dataWrapRTObject( RTobject object, RTobjecttype type )
{
  switch( type ) {
    //case RT_OBJECTTYPE_GROUP:
    case 'a': {
      GroupPtr* group_ptr = malloc( sizeof( GroupPtr ) );
      return Data_Wrap_Struct( class_group, 0, 0, group_ptr );
    } case RT_OBJECTTYPE_GEOMETRY_GROUP: {
      GeometryGroupPtr* geometrygroup_ptr = malloc( sizeof( GeometryGroupPtr ) );
      return Data_Wrap_Struct( class_geometrygroup, 0, 0, geometrygroup_ptr );
    } case RT_OBJECTTYPE_TRANSFORM: {
      TransformPtr* transform_ptr = malloc( sizeof( TransformPtr ) );
      return Data_Wrap_Struct( class_transform, 0, 0, transform_ptr );
    } case RT_OBJECTTYPE_SELECTOR: {
      SelectorPtr* selector_ptr = malloc( sizeof( SelectorPtr ) );
      return Data_Wrap_Struct( class_selector, 0, 0, selector_ptr );
    } case RT_OBJECTTYPE_GEOMETRY_INSTANCE: {
      GeometryInstancePtr* geometryinstance_ptr = malloc( sizeof( GeometryInstancePtr ) );
      return Data_Wrap_Struct( class_geometryinstance, 0, 0, geometryinstance_ptr );
    } case RT_OBJECTTYPE_BUFFER: {
      BufferPtr* buffer_ptr = malloc( sizeof( BufferPtr ) );
      return Data_Wrap_Struct( class_buffer, 0, 0, buffer_ptr );
    } case RT_OBJECTTYPE_TEXTURE_SAMPLER: {
      TextureSamplerPtr* texturesampler_ptr = malloc( sizeof( TextureSamplerPtr ) );
      return Data_Wrap_Struct( class_texturesampler, 0, 0, texturesampler_ptr );
    } default: {
      rb_raise(rb_eException, "dataWrapObject passed bad RTobjecttype: %d\n(%s:%d)", type, __FILE__, __LINE__);
      break;
    }
  }
}

static RTresult
checkBuffer(RTbuffer buffer)
{
  // Check to see if the buffer is two dimensional
  unsigned int dimensionality;
  RTresult result;
  result = rtBufferGetDimensionality(buffer, &dimensionality);
  if (result != RT_SUCCESS) return result;
  if (2 != dimensionality) {
    fprintf(stderr, "Dimensionality of the buffer is %u instead of 2\n", dimensionality);
    return RT_ERROR_INVALID_VALUE;
  }
  // Check to see if the buffer is of type float{1,3,4} or uchar4
  RTformat format;
  result = rtBufferGetFormat(buffer, &format);
  if (result != RT_SUCCESS) return result;
  if (RT_FORMAT_FLOAT != format && RT_FORMAT_FLOAT4 != format && RT_FORMAT_FLOAT3 != format && RT_FORMAT_UNSIGNED_BYTE4 != format) {
    fprintf(stderr, "Buffer's format isn't float, float3, float4, or uchar4\n");
    return RT_ERROR_INVALID_VALUE;
  }

  return RT_SUCCESS;
}


static RTresult SavePPM(const unsigned char *Pix, const char *fname, int wid, int hgt, int chan)
{
  if(Pix==NULL || chan < 1 || wid < 1 || hgt < 1) {
    fprintf(stderr, "Image is not defined. Not saving.\n");
    return RT_ERROR_UNKNOWN;
  }

  if(chan < 1 || chan > 4) {
    fprintf(stderr, "Can't save a X channel image as a PPM.\n");
    return RT_ERROR_UNKNOWN;
  }

  FILE* out = fopen( fname, "w" );
  if( !out ) {
    fprintf(stderr, "Could not open file '%s' for SavePPM\n", fname );
    return RT_ERROR_UNKNOWN;
  }

  int is_float = 0;
  
  fprintf( out, "P%c\n", ( ( chan==1 ? (is_float ? 'Z' :'5') : (chan==3 ? (is_float ? '7' : '6') : '8') ) ) );
  fprintf( out, "%i %i\n", wid, hgt );
  fprintf( out, "255\n" );

  fwrite( Pix , (is_float ? 4 : 1), wid * hgt * chan, out );

  fclose( out ); 

  return RT_SUCCESS;
}
RTresult displayFilePPM(const char* filename, RTbuffer buffer)
{
  int i, j;
  RTresult result;
  int width, height;
  RTsize buffer_width, buffer_height;

  result = checkBuffer(buffer);
  if (result != RT_SUCCESS) {
    fprintf(stderr, "Buffer isn't the right format. Didn't pass.\n");
    return result;
  }

  void* imageData;
  result = rtBufferMap(buffer, &imageData);
  if (result != RT_SUCCESS) {
    // Get error from context
    RTcontext context;
    const char* error;
    rtBufferGetContext(buffer, &context);
    rtContextGetErrorString(context, result, &error);
    fprintf(stderr, "Error mapping image buffer: %s\n", error);
    exit(2);
  }
  if (0 == imageData) {
    fprintf(stderr, "data in buffer is null.\n");
    exit(2);
  }
  result = rtBufferGetSize2D(buffer, &buffer_width, &buffer_height);
  if (result != RT_SUCCESS) {
    // Get error from context
    RTcontext context;
    const char* error;
    rtBufferGetContext(buffer, &context);
    rtContextGetErrorString(context, result, &error);
    fprintf(stderr, "Error getting dimensions of buffer: %s\n", error);
    exit(2);
  }
  width  = buffer_width;
  height = buffer_height;

  unsigned char* pix = malloc( width * height * 3 );
  RTformat buffer_format;

  result = rtBufferGetFormat(buffer, &buffer_format);

  switch(buffer_format) {
    case RT_FORMAT_UNSIGNED_BYTE4:
      // Data is BGRA and upside down, so we need to swizzle to RGB
      for(j = height-1; j >= 0; --j) {
        unsigned char *dst = &pix[0] + (3*width*(height-1-j));
        unsigned char *src = ((unsigned char*)imageData) + (4*width*j);
        for(i = 0; i < width; i++) {
          *dst++ = *(src + 2);
          *dst++ = *(src + 1);
          *dst++ = *(src + 0);
          src += 4;
        }
      }
      break;

    case RT_FORMAT_FLOAT:
      // This buffer is upside down
      for(j = height-1; j >= 0; --j) {
        unsigned char *dst = &pix[0] + width*(height-1-j);
        float* src = ((float*)imageData) + (3*width*j);
        for(i = 0; i < width; i++) {
          int P = (int)((*src++) * 255.0f);
          unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;

          // write the pixel to all 3 channels
          *dst++ = (unsigned char)(Clamped);
          *dst++ = (unsigned char)(Clamped);
          *dst++ = (unsigned char)(Clamped);
        }
      }
      break;

    case RT_FORMAT_FLOAT3:
      // This buffer is upside down
      for(j = height-1; j >= 0; --j) {
        unsigned char *dst = &pix[0] + (3*width*(height-1-j));
        float* src = ((float*)imageData) + (3*width*j);
        for(i = 0; i < width; i++) {
          int elem;
          for(elem = 0; elem < 3; ++elem) {
            int P = (int)((*src++) * 255.0f);
            unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;
            *dst++ = (unsigned char)(Clamped);
          }
        }
      }
      break;

    case RT_FORMAT_FLOAT4:
      // This buffer is upside down
      for(j = height-1; j >= 0; --j) {
        unsigned char *dst = &pix[0] + (3*width*(height-1-j));
        float* src = ((float*)imageData) + (4*width*j);
        for(i = 0; i < width; i++) {
          int elem;
          for(elem = 0; elem < 3; ++elem) {
            int P = (int)((*src++) * 255.0f);
            unsigned int Clamped = P < 0 ? 0 : P > 0xff ? 0xff : P;
            *dst++ = (unsigned char)(Clamped);
          }

          // skip alpha
          src++;
        }
      }
      break;

    default:
      fprintf(stderr, "Unrecognized buffer data type or format.\n");
      exit(2);
      break;
  }

  SavePPM(&pix[0], filename, width, height, 3);

  // Now unmap the buffer
  result = rtBufferUnmap(buffer);
  if (result != RT_SUCCESS) {
    // Get error from context
    RTcontext context;
    const char* error;
    rtBufferGetContext(buffer, &context);
    rtContextGetErrorString(context, result, &error);
    fprintf(stderr, "Error unmapping image buffer: %s\n", error);
    exit(2);
  }

  return RT_SUCCESS;
}

/******************************************************************************\
 *
 * Module functions 
 *
\******************************************************************************/

static VALUE optixGetElementSize( VALUE module, VALUE format )
{
  int element_size = 0;

  switch( NUM2INT( format ) ){
  case RT_FORMAT_FLOAT:
    element_size = sizeof(float);
    break;
  case RT_FORMAT_FLOAT2:
    element_size = sizeof(float)*2;
    break;
  case RT_FORMAT_FLOAT3:
    element_size = sizeof(float)*3;
    break;
  case RT_FORMAT_FLOAT4:
    element_size = sizeof(float)*4;
    break;
  case RT_FORMAT_BYTE:
    element_size = sizeof(char);
    break;
  case RT_FORMAT_BYTE2:
    element_size = sizeof(char)*2;
    break;
  case RT_FORMAT_BYTE3:
    element_size = sizeof(char)*3;
    break;
  case RT_FORMAT_BYTE4:
    element_size = sizeof(char)*4;
    break;
  case RT_FORMAT_UNSIGNED_BYTE:
    element_size = sizeof(unsigned char);
    break;
  case RT_FORMAT_UNSIGNED_BYTE2:
    element_size = sizeof(unsigned char)*2;
    break;
  case RT_FORMAT_UNSIGNED_BYTE3:
    element_size = sizeof(unsigned char)*3;
    break;
  case RT_FORMAT_UNSIGNED_BYTE4:
    element_size = sizeof(unsigned char)*4;
    break;
  case RT_FORMAT_SHORT:
    element_size = sizeof(short);
    break;
  case RT_FORMAT_SHORT2:
    element_size = sizeof(short)*2;
    break;
  case RT_FORMAT_SHORT3:
    element_size = sizeof(short)*3;
    break;
  case RT_FORMAT_SHORT4:
    element_size = sizeof(short)*4;
    break;
  case RT_FORMAT_UNSIGNED_SHORT:
    element_size = sizeof(unsigned short);
    break;
  case RT_FORMAT_UNSIGNED_SHORT2:
    element_size = sizeof(unsigned short)*2;
    break;
  case RT_FORMAT_UNSIGNED_SHORT3:
    element_size = sizeof(unsigned short)*3;
    break;
  case RT_FORMAT_UNSIGNED_SHORT4:
    element_size = sizeof(unsigned short)*4;
    break;
  case RT_FORMAT_INT:
    element_size = sizeof(int);
    break;
  case RT_FORMAT_INT2:
    element_size = sizeof(int)*2;
    break;
  case RT_FORMAT_INT3:
    element_size = sizeof(int)*3;
    break;
  case RT_FORMAT_INT4:
    element_size = sizeof(int)*4;
    break;
  case RT_FORMAT_UNSIGNED_INT:
    element_size = sizeof(unsigned int);
    break;
  case RT_FORMAT_UNSIGNED_INT2:
    element_size = sizeof(unsigned int)*2;
    break;
  case RT_FORMAT_UNSIGNED_INT3:
    element_size = sizeof(unsigned int)*3;
    break;
  case RT_FORMAT_UNSIGNED_INT4:
    element_size = sizeof(unsigned int)*4;
    break;
  case RT_FORMAT_USER:
  case RT_FORMAT_UNKNOWN:
    // These will return 0
    break;
  }

  return INT2NUM( element_size );
}


/******************************************************************************\
 *
 * Variable
 *
\******************************************************************************/

static VALUE variableAlloc( VALUE klass )
{
  VariablePtr* variable_ptr;

  variable_ptr = malloc( sizeof( VariablePtr ) );
  variable_ptr->variable = 0;

  return Data_Wrap_Struct( klass, 0, 0, variable_ptr );
}


//TODO: add variableSet{i,u,f}( int argc, VALUE* argv, VALUE self ) 

static VALUE variableInitialize( VALUE self, VALUE context )
{
  RTresult result;
  VariablePtr* variable_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, VariablePtr, variable_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtVariableCreate( context_ptr->context, &( variable_ptr->variable ) ) );

  return self;
}

static VALUE variableSet1f( VALUE self, VALUE f1 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet1f( variable_ptr->variable, NUM2DBL( f1 ) ) );
  return Qnil;
}

static VALUE variableSet2f( VALUE self, VALUE f1, VALUE f2 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet2f( variable_ptr->variable, NUM2DBL( f1 ), NUM2DBL( f2 ) ) );
  return Qnil;
}

static VALUE variableSet3f( VALUE self, VALUE f1, VALUE f2, VALUE f3 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet3f( variable_ptr->variable, NUM2DBL( f1 ), NUM2DBL( f2 ), NUM2DBL( f3 ) ) );
  return Qnil;
}

static VALUE variableSet4f( VALUE self, VALUE f1, VALUE f2, VALUE f3, VALUE f4 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet4f( variable_ptr->variable, NUM2DBL( f1 ), NUM2DBL( f2 ), NUM2DBL( f3 ), NUM2DBL( f4 ) ) );
  return Qnil;
}

static VALUE variableSet1i( VALUE self, VALUE i1 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet1i( variable_ptr->variable, NUM2INT( i1 ) ) );
  return Qnil;
}

static VALUE variableSet2i( VALUE self, VALUE i1, VALUE i2 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet2i( variable_ptr->variable, NUM2INT( i1 ), NUM2INT( i2 ) ) );
  return Qnil;
}

static VALUE variableSet3i( VALUE self, VALUE i1, VALUE i2, VALUE i3 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet3i( variable_ptr->variable, NUM2INT( i1 ), NUM2INT( i2 ), NUM2INT( i3 ) ) );
  return Qnil;
}

static VALUE variableSet4i( VALUE self, VALUE i1, VALUE i2, VALUE i3, VALUE i4 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet4i( variable_ptr->variable, NUM2INT( i1 ), NUM2INT( i2 ), NUM2INT( i3 ), NUM2INT( i4 ) ) );
  return Qnil;
}

static VALUE variableSet1ui( VALUE self, VALUE u1 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet1ui( variable_ptr->variable, NUM2INT( u1 ) ) );
  return Qnil;
}

static VALUE variableSet2ui( VALUE self, VALUE u1, VALUE u2 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet2ui( variable_ptr->variable, NUM2INT( u1 ), NUM2INT( u2 ) ) );
  return Qnil;
}

static VALUE variableSet3ui( VALUE self, VALUE u1, VALUE u2, VALUE u3 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet3ui( variable_ptr->variable, NUM2INT( u1 ), NUM2INT( u2 ), NUM2INT( u3 ) ) );
  return Qnil;
}

static VALUE variableSet4ui( VALUE self, VALUE u1, VALUE u2, VALUE u3, VALUE u4 )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtVariableSet4ui( variable_ptr->variable, NUM2INT( u1 ), NUM2INT( u2 ), NUM2INT( u3 ), NUM2INT( u4 ) ) );
  return Qnil;
}


static VALUE variableSetMatrix2x2( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  
  float mm[4];
  numArray2FltArray( 4, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix2x2fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetMatrix2x3( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float mm[6];
  numArray2FltArray( 6, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix2x3fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetMatrix2x4( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float mm[8];
  numArray2FltArray( 8, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix2x4fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetMatrix3x2( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float mm[6];
  numArray2FltArray( 6, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix3x2fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetMatrix3x3( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float mm[9];
  numArray2FltArray( 9, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix3x3fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetMatrix3x4( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float mm[12];
  numArray2FltArray( 12, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix3x4fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetMatrix4x2( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float mm[8];
  numArray2FltArray( 8, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix4x2fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetMatrix4x3( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float mm[12];
  numArray2FltArray( 12, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix4x3fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetMatrix4x4( VALUE self, VALUE transpose, VALUE m )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float mm[16];
  numArray2FltArray( 16, m, mm );
  RT_CHECK_ERROR( rtVariableSetMatrix4x4fv( variable_ptr->variable, NUM2INT( transpose ), mm ) );
  return Qnil;
}

static VALUE variableSetObject( VALUE self, VALUE object )
{
  RTresult result;
  VariablePtr* variable_ptr;
  ObjectPtr* object_ptr;

  Data_Get_Struct( self, VariablePtr, variable_ptr );
  Data_Get_Struct( object, ObjectPtr, object_ptr );

  RT_CHECK_ERROR( rtVariableSetObject( variable_ptr->variable, object_ptr->object ) );
  return Qnil;
}

/* TODO: probably need to define float3, uint2, etc and restrict user classes to be of
 * these cuda types or basic numeric types. then can use rb_obj_instance_variables to
 * grab the variables and pack into C array. Or can we use Data_Get_STruct?
static VALUE variableSetUserData( VALUE self, VALUE size )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  const void* ptr;
  RT_CHECK_ERROR( rtVariableSetUserData( variable_ptr->variable, size, &ptr ) );
  return ptr;
}
*/

static VALUE variableGet1f( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float f1;
  RT_CHECK_ERROR( rtVariableGet1f( variable_ptr->variable, &f1 ) );
  return rb_float_new( f1 );
}

static VALUE variableGet2f( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float f1;
  float f2;
  RT_CHECK_ERROR( rtVariableGet2f( variable_ptr->variable, &f1, &f2 ) );
  return f1, f2;
}

static VALUE variableGet3f( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float f1;
  float f2;
  float f3;
  RT_CHECK_ERROR( rtVariableGet3f( variable_ptr->variable, &f1, &f2, &f3 ) );
  return f1, f2, f3;
}

static VALUE variableGet4f( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float f1;
  float f2;
  float f3;
  float f4;
  RT_CHECK_ERROR( rtVariableGet4f( variable_ptr->variable, &f1, &f2, &f3, &f4 ) );
  return f1, f2, f3, f4;
}

static VALUE variableGet1fv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float f;
  RT_CHECK_ERROR( rtVariableGet1fv( variable_ptr->variable, &f ) );
  return f;
}

static VALUE variableGet2fv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float f;
  RT_CHECK_ERROR( rtVariableGet2fv( variable_ptr->variable, &f ) );
  return f;
}

static VALUE variableGet3fv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float f;
  RT_CHECK_ERROR( rtVariableGet3fv( variable_ptr->variable, &f ) );
  return f;
}

static VALUE variableGet4fv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float f;
  RT_CHECK_ERROR( rtVariableGet4fv( variable_ptr->variable, &f ) );
  return f;
}

static VALUE variableGet1i( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  int i1;
  RT_CHECK_ERROR( rtVariableGet1i( variable_ptr->variable, &i1 ) );
  return i1;
}

static VALUE variableGet2i( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  int i1;
  int i2;
  RT_CHECK_ERROR( rtVariableGet2i( variable_ptr->variable, &i1, &i2 ) );
  return i1, i2;
}

static VALUE variableGet3i( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  int i1;
  int i2;
  int i3;
  RT_CHECK_ERROR( rtVariableGet3i( variable_ptr->variable, &i1, &i2, &i3 ) );
  return i1, i2, i3;
}

static VALUE variableGet4i( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  int i1;
  int i2;
  int i3;
  int i4;
  RT_CHECK_ERROR( rtVariableGet4i( variable_ptr->variable, &i1, &i2, &i3, &i4 ) );
  return i1, i2, i3, i4;
}

static VALUE variableGet1iv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  int i;
  RT_CHECK_ERROR( rtVariableGet1iv( variable_ptr->variable, &i ) );
  return i;
}

static VALUE variableGet2iv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  int i;
  RT_CHECK_ERROR( rtVariableGet2iv( variable_ptr->variable, &i ) );
  return i;
}

static VALUE variableGet3iv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  int i;
  RT_CHECK_ERROR( rtVariableGet3iv( variable_ptr->variable, &i ) );
  return i;
}

static VALUE variableGet4iv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  int i;
  RT_CHECK_ERROR( rtVariableGet4iv( variable_ptr->variable, &i ) );
  return i;
}

static VALUE variableGet1ui( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  unsigned int u1;
  RT_CHECK_ERROR( rtVariableGet1ui( variable_ptr->variable, &u1 ) );
  return u1;
}

static VALUE variableGet2ui( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  unsigned int u1;
  unsigned int u2;
  RT_CHECK_ERROR( rtVariableGet2ui( variable_ptr->variable, &u1, &u2 ) );
  return u1, u2;
}

static VALUE variableGet3ui( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  unsigned int u1;
  unsigned int u2;
  unsigned int u3;
  RT_CHECK_ERROR( rtVariableGet3ui( variable_ptr->variable, &u1, &u2, &u3 ) );
  return u1, u2, u3;
}

static VALUE variableGet4ui( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  unsigned int u1;
  unsigned int u2;
  unsigned int u3;
  unsigned int u4;
  RT_CHECK_ERROR( rtVariableGet4ui( variable_ptr->variable, &u1, &u2, &u3, &u4 ) );
  return u1, u2, u3, u4;
}

static VALUE variableGet1uiv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  unsigned int u;
  RT_CHECK_ERROR( rtVariableGet1uiv( variable_ptr->variable, &u ) );
  return u;
}

static VALUE variableGet2uiv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  unsigned int u;
  RT_CHECK_ERROR( rtVariableGet2uiv( variable_ptr->variable, &u ) );
  return u;
}

static VALUE variableGet3uiv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  unsigned int u;
  RT_CHECK_ERROR( rtVariableGet3uiv( variable_ptr->variable, &u ) );
  return u;
}

static VALUE variableGet4uiv( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  unsigned int u;
  RT_CHECK_ERROR( rtVariableGet4uiv( variable_ptr->variable, &u ) );
  return u;
}

static VALUE variableGetMatrix2x2fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix2x2fv( variable_ptr->variable, transpose, &m ) );
  return m;
}

static VALUE variableGetMatrix2x3fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix2x3fv( variable_ptr->variable, transpose, &m ) );
  return m;
}

static VALUE variableGetMatrix2x4fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix2x4fv( variable_ptr->variable, transpose, &m ) );
  return m;
}

static VALUE variableGetMatrix3x2fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix3x2fv( variable_ptr->variable, transpose, &m ) );
  return m;
}

static VALUE variableGetMatrix3x3fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix3x3fv( variable_ptr->variable, transpose, &m ) );
  return m;
}

static VALUE variableGetMatrix3x4fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix3x4fv( variable_ptr->variable, transpose, &m ) );
  return m;
}

static VALUE variableGetMatrix4x2fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix4x2fv( variable_ptr->variable, transpose, &m ) );
  return m;
}

static VALUE variableGetMatrix4x3fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix4x3fv( variable_ptr->variable, transpose, &m ) );
  return m;
}

static VALUE variableGetMatrix4x4fv( VALUE self, VALUE transpose )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  float m;
  RT_CHECK_ERROR( rtVariableGetMatrix4x4fv( variable_ptr->variable, transpose, &m ) );
  return m;
}


/* TODO: need to figure this one out
static VALUE variableGetObject( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  ObjectPtr* object_ptr;
  
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  object_ptr = malloc( sizeof( ObjectPtr ) );
  result     = rtVariableGetObject( variable_ptr->variable, &(object_ptr->object) );

  return Data_Wrap_Struct( klass, 0, 0, object_ptr );
}
*/

/* TODO : set variableSetUserData
static VALUE variableGetUserData( VALUE self, VALUE size )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  void ptr;
  RT_CHECK_ERROR( rtVariableGetUserData( variable_ptr->variable, size, &ptr ) );
  return ptr;
}
*/

static VALUE variableGetName( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  const char* name_return;
  RT_CHECK_ERROR( rtVariableGetName( variable_ptr->variable, &name_return ) );

  return rb_str_new2( name_return );
}

static VALUE variableGetAnnotation( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  const char* annotation_return;
  RT_CHECK_ERROR( rtVariableGetAnnotation( variable_ptr->variable, &annotation_return ) );

  return rb_str_new2( annotation_return );
}

static VALUE variableGetType( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  Data_Get_Struct( self, VariablePtr, variable_ptr );

  RTobjecttype type_return;
  RT_CHECK_ERROR( rtVariableGetType( variable_ptr->variable, &type_return ) );
  return INT2NUM( type_return );
}

static VALUE variableGetContext( VALUE self )
{
  RTresult result;
  VariablePtr* variable_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, VariablePtr, variable_ptr );
  context_ptr = malloc( sizeof( context_ptr ) );

  RT_CHECK_ERROR( rtVariableGetContext( variable_ptr->variable, &( context_ptr->context )  ) );

  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}


/******************************************************************************\
 *
 * Context 
 *
\******************************************************************************/

static VALUE contextAlloc( VALUE klass )
{
  ContextPtr* context_ptr;

  context_ptr = malloc( sizeof( ContextPtr ) );
  context_ptr->context = 0;

  return Data_Wrap_Struct( klass, 0, 0, context_ptr );
}


static VALUE contextInitialize( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextCreate( &( context_ptr->context ) ) );

  if( current_context ) {
    rb_raise(rb_eException, "Currently only single active context supported." );
  }
  current_context = context_ptr->context;

  return self;
}

static VALUE contextCreate( VALUE klass )
{
  return rb_class_new_instance( 0, 0, class_context );
}

static VALUE contextDestroy( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextDestroy( context_ptr->context ) );
  current_context = 0;

  return Qnil;
}

static VALUE contextValidate( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextValidate( context_ptr->context ) );

  return Qnil;
}

static VALUE contextSetDevices( VALUE self, VALUE count )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  const int devices;
  RT_CHECK_ERROR( rtContextSetDevices( context_ptr->context, NUM2INT( count ), &devices ) );
  return INT2NUM( devices );
}

static VALUE contextSetStackSize( VALUE self, VALUE stack_size_bytes )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextSetStackSize( context_ptr->context, NUM2INT( stack_size_bytes ) ) );
  return Qnil;
}

static VALUE contextGetStackSize( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RTsize stack_size_bytes;
  RT_CHECK_ERROR( rtContextGetStackSize( context_ptr->context, &stack_size_bytes ) );
  return INT2NUM( stack_size_bytes );
}

static VALUE contextSetEntryPointCount( VALUE self, VALUE num_entry_points )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextSetEntryPointCount( context_ptr->context, NUM2INT( num_entry_points  ) ) );
  return Qnil;
}

static VALUE contextGetEntryPointCount( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  unsigned int num_entry_points;
  RT_CHECK_ERROR( rtContextGetEntryPointCount( context_ptr->context, &num_entry_points ) );
  return INT2NUM( num_entry_points );
}

static VALUE contextSetRayGenerationProgram( VALUE self, VALUE entry_point_index, VALUE program )
{
  RTresult result;
  ContextPtr* context_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  Data_Get_Struct( program, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtContextSetRayGenerationProgram( context_ptr->context, NUM2INT( entry_point_index ), program_ptr->program ) );

  return Qnil;
}

static VALUE contextGetRayGenerationProgram( VALUE self, VALUE entry_point_index )
{
  RTresult result;
  ContextPtr* context_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  program_ptr = malloc( sizeof( ProgramPtr ) );

  RT_CHECK_ERROR( rtContextGetRayGenerationProgram( context_ptr->context, NUM2INT( entry_point_index ), &( program_ptr->program ) ) );

  return Data_Wrap_Struct( class_program, 0, 0, program_ptr );
}

static VALUE contextSetExceptionProgram( VALUE self, VALUE entry_point_index, VALUE program )
{
  RTresult result;
  ContextPtr* context_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  Data_Get_Struct( program, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtContextSetExceptionProgram( context_ptr->context, NUM2INT( entry_point_index ), program_ptr->program ) );
  return Qnil;
}

static VALUE contextGetExceptionProgram( VALUE self, VALUE entry_point_index )
{
  RTresult result;
  ContextPtr* context_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  program_ptr = malloc( sizeof( ProgramPtr ) );

  RT_CHECK_ERROR( rtContextGetExceptionProgram( context_ptr->context, NUM2INT( entry_point_index ), &(program_ptr->program) ) );
  return Data_Wrap_Struct( class_program, 0, 0, program_ptr );
}

static VALUE contextSetRayTypeCount( VALUE self, VALUE num_ray_types )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextSetRayTypeCount( context_ptr->context, NUM2INT( num_ray_types ) ) );
  return Qnil;
}

static VALUE contextGetRayTypeCount( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  unsigned int num_ray_types;
  RT_CHECK_ERROR( rtContextGetRayTypeCount( context_ptr->context, &num_ray_types ) );
  return INT2NUM( num_ray_types );
}

static VALUE contextSetMissProgram( VALUE self, VALUE ray_type_index, VALUE program )
{
  RTresult result;
  ContextPtr* context_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  Data_Get_Struct( program, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtContextSetMissProgram( context_ptr->context, NUM2INT( ray_type_index ), program_ptr->program ) );
  return Qnil;
}

static VALUE contextGetMissProgram( VALUE self, VALUE ray_type_index )
{
  RTresult result;
  ContextPtr* context_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  program_ptr = malloc( sizeof( ProgramPtr ) );

  RT_CHECK_ERROR( rtContextGetMissProgram( context_ptr->context, NUM2INT( ray_type_index ), &( program_ptr->program ) ) );
  return Data_Wrap_Struct( class_program, 0, 0, program_ptr );
}

static VALUE contextCompile( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextCompile( context_ptr->context ) );

  return Qnil;
}

static VALUE contextLaunch( int argc, VALUE* argv, VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  VALUE entry_point_index, dim0, dim1, dim2;

  Data_Get_Struct( self, ContextPtr, context_ptr );

  int num_args = rb_scan_args( argc, argv, "22", &entry_point_index, &dim0, &dim1, &dim2 );

  if( num_args == 2 ) {
    RT_CHECK_ERROR( rtContextLaunch1D( context_ptr->context, NUM2INT( entry_point_index ), NUM2INT( dim0 ) ) );
  } else if ( num_args == 3 ) {
    RT_CHECK_ERROR( rtContextLaunch2D( context_ptr->context, NUM2INT( entry_point_index ), NUM2INT( dim0 ), NUM2INT( dim1 ) ) );
  } else if ( num_args == 4 ) {
    RT_CHECK_ERROR( rtContextLaunch3D( context_ptr->context, NUM2INT( entry_point_index ), NUM2INT( dim0 ), NUM2INT( dim1 ), NUM2INT( dim2 ) ) );
  }

  return Qnil;
}

static VALUE contextGetRunningState( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  int running;
  RT_CHECK_ERROR( rtContextGetRunningState( context_ptr->context, &running ) );
  return INT2NUM( running );
}

static VALUE contextSetPrintEnabled( VALUE self, VALUE enabled )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextSetPrintEnabled( context_ptr->context, NUM2INT( enabled ) ) );
  return Qnil;
}

static VALUE contextGetPrintEnabled( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  int enabled;
  RT_CHECK_ERROR( rtContextGetPrintEnabled( context_ptr->context, &enabled ) );
  return INT2NUM( enabled );
}

static VALUE contextSetPrintBufferSize( VALUE self, VALUE buffer_size_bytes )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextSetPrintBufferSize( context_ptr->context, NUM2INT( buffer_size_bytes ) ) );
  return Qnil;
}

static VALUE contextGetPrintBufferSize( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RTsize buffer_size_bytes;
  RT_CHECK_ERROR( rtContextGetPrintBufferSize( context_ptr->context, &buffer_size_bytes ) );
  return INT2NUM( buffer_size_bytes );
}

static VALUE contextSetPrintLaunchIndex( VALUE self, VALUE x, VALUE y, VALUE z )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtContextSetPrintLaunchIndex( context_ptr->context, NUM2INT( x ), NUM2INT( y ), NUM2INT( z ) ) );
  return Qnil;
}

static VALUE contextGetPrintLaunchIndex( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  int x;
  int y;
  int z;
  RT_CHECK_ERROR( rtContextGetPrintLaunchIndex( context_ptr->context, &x, &y, &z ) );
  return x, y, z;
}

static VALUE contextDeclareVariable( VALUE self, VALUE name )
{
  RTresult result;
  ContextPtr* context_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtContextDeclareVariable( context_ptr->context, StringValueCStr( name ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}

static VALUE contextQueryVariable( VALUE self, VALUE name )
{
  RTresult result;
  ContextPtr* context_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtContextQueryVariable( context_ptr->context, StringValueCStr( name ), &( variable_ptr->variable ) ) );
  if( variable_ptr->variable )
    return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
  else
    return Qnil;
}

static VALUE contextRemoveVariable( VALUE self, VALUE v )
{
  RTresult result;
  ContextPtr* context_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  Data_Get_Struct( v, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtContextRemoveVariable( context_ptr->context, variable_ptr->variable ) );
  return Qnil;
}

static VALUE contextGetVariableCount( VALUE self )
{
  RTresult result;
  ContextPtr* context_ptr;
  Data_Get_Struct( self, ContextPtr, context_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtContextGetVariableCount( context_ptr->context, &count ) );
  return INT2NUM( count );
}

static VALUE contextGetVariable( VALUE self, VALUE index )
{
  RTresult result;
  ContextPtr* context_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, ContextPtr, context_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtContextGetVariable( context_ptr->context, NUM2INT( index ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}

static VALUE contextCreateProgramFromPTXString( VALUE self, VALUE ptx_string, VALUE program_name )
{
  VALUE args[4];
  args[0] = self;
  args[1] = ptx_string;
  args[2] = program_name;
  args[3] = INT2NUM( 1 );
  return rb_class_new_instance( 4, args, class_program );
}

static VALUE contextCreateProgramFromPTXFile( VALUE self, VALUE file_name, VALUE program_name )
{

  VALUE args[4];
  args[0] = self;
  args[1] = file_name;
  args[2] = program_name;
  args[3] = INT2NUM( 0 );
  return rb_class_new_instance( 4, args, class_program );
}

static VALUE contextCreateGroup( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_group );
}

static VALUE contextCreateVariable( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_variable );
}

static VALUE contextCreateTextureSampler( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_texturesampler );
}

static VALUE contextCreateBuffer( int argc, VALUE* argv, VALUE self )
{
  VALUE type, format, dim0, dim1, dim2;
  VALUE args[2];

  int num_args = rb_scan_args( argc, argv, "14", &type, &format, &dim0, &dim1, &dim2 );

  args[0] = self;
  args[1] = type;
  VALUE buffer = rb_class_new_instance( 2, args, class_buffer );

  if( num_args > 1 ) {
    bufferSetFormat( buffer, format );
  }

  if( num_args == 3 ) {
    bufferSetSize1D( buffer, dim0 );
  } else if ( num_args == 4 ) {
    bufferSetSize2D( buffer, dim0, dim1 );
  } else if ( num_args == 5 ) {
    bufferSetSize3D( buffer, dim0, dim1, dim2 );
  }

  return buffer;
}

static VALUE contextCreateSelector( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_selector );
}

static VALUE contextCreateTransform( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_transform );
}

static VALUE contextCreateGeometryInstance( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_geometryinstance );
}

static VALUE contextCreateGeometry( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_geometry );
}

static VALUE contextCreateGeometryGroup( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_geometrygroup );
}

static VALUE contextCreateAcceleration( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_acceleration );
}

static VALUE contextCreateMaterial( VALUE self )
{
  return rb_class_new_instance( 1, &self, class_material );
}

/******************************************************************************\
 *
 * Program 
 *
\******************************************************************************/
static VALUE programAlloc( VALUE klass )
{
  ProgramPtr* program_ptr;

  program_ptr = malloc( sizeof( ProgramPtr ) );
  program_ptr->program = 0;

  return Data_Wrap_Struct( klass, 0, 0, program_ptr );
}


static VALUE programInitialize( VALUE self, VALUE context, VALUE string, VALUE program_name, VALUE is_ptx_string )
{
  RTresult result;
  ProgramPtr* program_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, ProgramPtr, program_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  if( NUM2INT( is_ptx_string ) ) {
    RT_CHECK_ERROR( rtProgramCreateFromPTXString( context_ptr->context,
                                                  StringValueCStr( string ),
                                                  StringValueCStr( program_name ),
                                                  &( program_ptr->program ) ) );
  } else {
    RT_CHECK_ERROR( rtProgramCreateFromPTXFile( context_ptr->context,
                                                StringValueCStr( string ),
                                                StringValueCStr( program_name ),
                                                &( program_ptr->program ) ) );
  }

  return self;
}

static VALUE programDestroy( VALUE self )
{
  RTresult result;
  ProgramPtr* program_ptr;
  Data_Get_Struct( self, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtProgramDestroy( program_ptr->program ) );
  return Qnil;
}

static VALUE programValidate( VALUE self )
{
  RTresult result;
  ProgramPtr* program_ptr;
  Data_Get_Struct( self, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtProgramValidate( program_ptr->program ) );
  return Qnil;
}

static VALUE programGetContext( VALUE self )
{
  RTresult result;
  ProgramPtr* program_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, ProgramPtr, program_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtProgramGetContext( program_ptr->program, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE programDeclareVariable( VALUE self, VALUE name )
{
  RTresult result;
  ProgramPtr* program_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, ProgramPtr, program_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtProgramDeclareVariable( program_ptr->program, StringValueCStr( name ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}

static VALUE programQueryVariable( VALUE self, VALUE name )
{
  RTresult result;
  ProgramPtr* program_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, ProgramPtr, program_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtProgramQueryVariable( program_ptr->program, StringValueCStr( name ), &( variable_ptr->variable ) ) );
  if( variable_ptr->variable )
    return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
  else
    return Qnil;
}

static VALUE programRemoveVariable( VALUE self, VALUE v )
{
  RTresult result;
  ProgramPtr* program_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, ProgramPtr, program_ptr );
  Data_Get_Struct( v, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtProgramRemoveVariable( program_ptr->program, variable_ptr->variable ) );
  return Qnil;
}

static VALUE programGetVariableCount( VALUE self )
{
  RTresult result;
  ProgramPtr* program_ptr;
  Data_Get_Struct( self, ProgramPtr, program_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtProgramGetVariableCount( program_ptr->program, &count ) );
  return INT2NUM( count );
}

static VALUE programGetVariable( VALUE self, VALUE index )
{
  RTresult result;
  ProgramPtr* program_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, ProgramPtr, program_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtProgramGetVariable( program_ptr->program, NUM2INT( index ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}


/******************************************************************************\
 *
 * Group 
 *
\******************************************************************************/
static VALUE groupAlloc( VALUE klass )
{
  GroupPtr* group_ptr;

  group_ptr = malloc( sizeof( GroupPtr ) );
  group_ptr->group = 0;

  return Data_Wrap_Struct( klass, 0, 0, group_ptr );
}


static VALUE groupInitialize( VALUE self, VALUE context )
{
  RTresult result;
  GroupPtr* group_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, GroupPtr, group_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtGroupCreate( context_ptr->context, &( group_ptr->group ) ) );

  return self;
}


static VALUE groupDestroy( VALUE self )
{
  RTresult result;
  GroupPtr* group_ptr;
  Data_Get_Struct( self, GroupPtr, group_ptr );

  RT_CHECK_ERROR( rtGroupDestroy( group_ptr->group ) );
  return Qnil;
}

static VALUE groupValidate( VALUE self )
{
  RTresult result;
  GroupPtr* group_ptr;
  Data_Get_Struct( self, GroupPtr, group_ptr );

  RT_CHECK_ERROR( rtGroupValidate( group_ptr->group ) );
  return Qnil;
}

static VALUE groupGetContext( VALUE self )
{
  RTresult result;
  GroupPtr* group_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, GroupPtr, group_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtGroupGetContext( group_ptr->group, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE groupSetAcceleration( VALUE self, VALUE acceleration )
{
  RTresult result;
  GroupPtr* group_ptr;
  AccelerationPtr* acceleration_ptr;

  Data_Get_Struct( self, GroupPtr, group_ptr );
  Data_Get_Struct( acceleration, AccelerationPtr, acceleration_ptr );

  RT_CHECK_ERROR( rtGroupSetAcceleration( group_ptr->group, acceleration_ptr->acceleration ) );
  return Qnil;
}

static VALUE groupGetAcceleration( VALUE self )
{
  RTresult result;
  GroupPtr* group_ptr;
  AccelerationPtr* acceleration_ptr;

  Data_Get_Struct( self, GroupPtr, group_ptr );
  acceleration_ptr = malloc( sizeof( AccelerationPtr ) );

  RT_CHECK_ERROR( rtGroupGetAcceleration( group_ptr->group, &( acceleration_ptr->acceleration ) ) );
  return Data_Wrap_Struct( class_acceleration, 0, 0, acceleration_ptr );
}

static VALUE groupSetChildCount( VALUE self, VALUE count )
{
  RTresult result;
  GroupPtr* group_ptr;
  Data_Get_Struct( self, GroupPtr, group_ptr );

  RT_CHECK_ERROR( rtGroupSetChildCount( group_ptr->group, NUM2INT( count ) ) );
  return Qnil;
}

static VALUE groupGetChildCount( VALUE self )
{
  RTresult result;
  GroupPtr* group_ptr;
  Data_Get_Struct( self, GroupPtr, group_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtGroupGetChildCount( group_ptr->group, &count ) );
  return INT2NUM( count );
}

// TODO: test this!! may need to convert to proper c type before calling
// data_get_struct
static VALUE groupSetChild( VALUE self, VALUE index, VALUE child )
{
  RTresult result;
  GroupPtr* group_ptr;
  ObjectPtr* object_ptr;

  Data_Get_Struct( self, GroupPtr, group_ptr );
  Data_Get_Struct( child, ObjectPtr, object_ptr );

  RT_CHECK_ERROR( rtGroupSetChild( group_ptr->group, NUM2INT( index ), object_ptr->object ) );
  return Qnil;
}

// TODO: test this!! 
static VALUE groupGetChild( VALUE self, VALUE index )
{
  RTresult result;
  GroupPtr* group_ptr;
  Data_Get_Struct( self, GroupPtr, group_ptr );

  RTobject child;
  RTobjecttype child_type;
  RT_CHECK_ERROR( rtGroupGetChildType( group_ptr->group, NUM2INT( index ), &child_type ) );
  RT_CHECK_ERROR( rtGroupGetChild( group_ptr->group, NUM2INT( index ), &child ) );

  return dataWrapRTObject( child, child_type );
}

/*
static VALUE groupGetChildType( VALUE self, VALUE index )
{
  RTresult result;
  GroupPtr* group_ptr;
  Data_Get_Struct( self, GroupPtr, group_ptr );

  RTobjecttype type;
  RT_CHECK_ERROR( rtGroupGetChildType( group_ptr->group, NUM2INT( index ), &type ) );
  return INT2NUM( type );
}
*/


/******************************************************************************\
 *
 * Selector 
 *
\******************************************************************************/
static VALUE selectorAlloc( VALUE klass )
{
  SelectorPtr* selector_ptr;

  selector_ptr = malloc( sizeof( SelectorPtr ) );
  selector_ptr->selector = 0;

  return Data_Wrap_Struct( klass, 0, 0, selector_ptr );
}


static VALUE selectorInitialize( VALUE self, VALUE context )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtSelectorCreate( context_ptr->context, &( selector_ptr->selector ) ) );

  return self;
}

static VALUE selectorDestroy( VALUE self )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  Data_Get_Struct( self, SelectorPtr, selector_ptr );

  RT_CHECK_ERROR( rtSelectorDestroy( selector_ptr->selector ) );
  return Qnil;
}

static VALUE selectorValidate( VALUE self )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  Data_Get_Struct( self, SelectorPtr, selector_ptr );

  RT_CHECK_ERROR( rtSelectorValidate( selector_ptr->selector ) );
  return Qnil;
}

static VALUE selectorGetContext( VALUE self )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtSelectorGetContext( selector_ptr->selector, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE selectorSetVisitProgram( VALUE self, VALUE program )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  Data_Get_Struct( program, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtSelectorSetVisitProgram( selector_ptr->selector, program_ptr->program ) );
  return Qnil;
}

static VALUE selectorGetVisitProgram( VALUE self )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  program_ptr = malloc( sizeof( ProgramPtr ) );

  RT_CHECK_ERROR( rtSelectorGetVisitProgram( selector_ptr->selector, &( program_ptr->program ) ) );

  return Data_Wrap_Struct( class_program, 0, 0, program_ptr );
}

static VALUE selectorSetChildCount( VALUE self, VALUE count )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  Data_Get_Struct( self, SelectorPtr, selector_ptr );

  RT_CHECK_ERROR( rtSelectorSetChildCount( selector_ptr->selector, NUM2INT( count ) ) );

  return Qnil;
}

static VALUE selectorGetChildCount( VALUE self )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  Data_Get_Struct( self, SelectorPtr, selector_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtSelectorGetChildCount( selector_ptr->selector, &count ) );
  return INT2NUM( count );
}

static VALUE selectorSetChild( VALUE self, VALUE index, VALUE child )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  ObjectPtr* object_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  Data_Get_Struct( child, ObjectPtr, object_ptr );

  RT_CHECK_ERROR( rtSelectorSetChild( selector_ptr->selector, NUM2INT( index ), object_ptr->object) );
  return Qnil;
}

static VALUE selectorGetChild( VALUE self, VALUE index )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  Data_Get_Struct( self, SelectorPtr, selector_ptr );

  RTobject child;
  RTobjecttype child_type;
  RT_CHECK_ERROR( rtSelectorGetChildType( selector_ptr->selector, NUM2INT( index ), &child_type ) );
  RT_CHECK_ERROR( rtSelectorGetChild( selector_ptr->selector, NUM2INT( index ), &child ) );

  return dataWrapRTObject( child, child_type );
}

/* TODO: dont need this i think
static VALUE selectorGetChildType( VALUE self, VALUE index )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  Data_Get_Struct( self, SelectorPtr, selector_ptr );

  RTobjecttype type;
  RT_CHECK_ERROR( rtSelectorGetChildType( selector_ptr->selector, index, &type ) );
  return INT2NUM( type );
}
*/

static VALUE selectorDeclareVariable( VALUE self, VALUE name )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtSelectorDeclareVariable( selector_ptr->selector, StringValueCStr( name ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}

static VALUE selectorQueryVariable( VALUE self, VALUE name )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtSelectorQueryVariable( selector_ptr->selector, StringValueCStr( name ), &( variable_ptr->variable ) ) );
  
  if( variable_ptr->variable )
    return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
  else
    return Qnil;
}

static VALUE selectorRemoveVariable( VALUE self, VALUE v )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  Data_Get_Struct( v, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtSelectorRemoveVariable( selector_ptr->selector, variable_ptr->variable ) );
  return Qnil;
}

static VALUE selectorGetVariableCount( VALUE self )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  Data_Get_Struct( self, SelectorPtr, selector_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtSelectorGetVariableCount( selector_ptr->selector, &count ) );
  return INT2NUM( count );
}

static VALUE selectorGetVariable( VALUE self, VALUE index )
{
  RTresult result;
  SelectorPtr* selector_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, SelectorPtr, selector_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtSelectorGetVariable( selector_ptr->selector, NUM2INT( index ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}


/******************************************************************************\
 *
 * Transform 
 *
\******************************************************************************/
static VALUE transformAlloc( VALUE klass )
{
  TransformPtr* transform_ptr;

  transform_ptr = malloc( sizeof( TransformPtr ) );
  transform_ptr->transform = 0;

  return Data_Wrap_Struct( klass, 0, 0, transform_ptr );
}


static VALUE transformInitialize( VALUE self, VALUE context )
{
  RTresult result;
  TransformPtr* transform_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, TransformPtr, transform_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtTransformCreate( context_ptr->context, &( transform_ptr->transform ) ) );

  return self;
}

static VALUE transformDestroy( VALUE self )
{
  RTresult result;
  TransformPtr* transform_ptr;
  Data_Get_Struct( self, TransformPtr, transform_ptr );

  RT_CHECK_ERROR( rtTransformDestroy( transform_ptr->transform ) );
  return Qnil;
}

static VALUE transformValidate( VALUE self )
{
  RTresult result;
  TransformPtr* transform_ptr;
  Data_Get_Struct( self, TransformPtr, transform_ptr );

  RT_CHECK_ERROR( rtTransformValidate( transform_ptr->transform ) );
  return Qnil;
}

static VALUE transformGetContext( VALUE self )
{
  RTresult result;
  TransformPtr* transform_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, TransformPtr, transform_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtTransformGetContext( transform_ptr->transform, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE transformSetMatrix( VALUE self, VALUE transpose, VALUE m, VALUE inv_m )
{
  RTresult result;
  TransformPtr* transform_ptr;
  Data_Get_Struct( self, TransformPtr, transform_ptr );

  float matrix[16];
  float inverse_matrix[16];
  numArray2FltArray( 16, m, matrix );
  numArray2FltArray( 16, inv_m, inverse_matrix );
  RT_CHECK_ERROR( rtTransformSetMatrix( transform_ptr->transform, NUM2INT( transpose ), matrix, inverse_matrix ) );
  return  Qnil;
}

static VALUE transformGetMatrix( VALUE self, VALUE transpose )
{
  int i;
  RTresult result;
  TransformPtr* transform_ptr;
  Data_Get_Struct( self, TransformPtr, transform_ptr );

  float matrix[16];
  RT_CHECK_ERROR( rtTransformGetMatrix( transform_ptr->transform, NUM2INT( transpose ), matrix, 0 ) );
  VALUE ary = rb_ary_new2( 16 );
  for( i = 0; i < 16; ++i )
    rb_ary_store( ary, i, rb_float_new( matrix[i] ) );
  return ary;
}

static VALUE transformGetMatrixInverse( VALUE self, VALUE transpose )
{
  int i;
  RTresult result;
  TransformPtr* transform_ptr;
  Data_Get_Struct( self, TransformPtr, transform_ptr );

  float inverse_matrix[16];
  RT_CHECK_ERROR( rtTransformGetMatrix( transform_ptr->transform, NUM2INT( transpose ), 0, inverse_matrix ) );
  VALUE ary = rb_ary_new2( 16 );
  for( i = 0; i < 16; ++i )
    rb_ary_store( ary, i, rb_float_new( inverse_matrix[i] ) );
  return ary; 
}

static VALUE transformSetChild( VALUE self, VALUE child )
{
  RTresult result;
  TransformPtr* transform_ptr;
  ObjectPtr* object_ptr;

  Data_Get_Struct( self, TransformPtr, transform_ptr );
  Data_Get_Struct( child, ObjectPtr, object_ptr );

  RT_CHECK_ERROR( rtTransformSetChild( transform_ptr->transform, object_ptr->object) );
  return Qnil;
}

static VALUE transformGetChild( VALUE self )
{
  RTresult result;
  TransformPtr* transform_ptr;
  Data_Get_Struct( self, TransformPtr, transform_ptr );

  RTobject child;
  RTobjecttype child_type;
  RT_CHECK_ERROR( rtTransformGetChildType( transform_ptr->transform, &child_type ) );
  RT_CHECK_ERROR( rtTransformGetChild( transform_ptr->transform, &child ) );

  return dataWrapRTObject( child, child_type );
}

/*
static VALUE transformGetChildType( VALUE self )
{
  RTresult result;
  TransformPtr* transform_ptr;
  Data_Get_Struct( self, TransformPtr, transform_ptr );

  RTobjecttype type;
  RT_CHECK_ERROR( rtTransformGetChildType( transform_ptr->transform, &type ) );
  return INT2NUM( type );
}
*/


/******************************************************************************\
 *
 * GeometryGroup 
 *
\******************************************************************************/

static VALUE geometryGroupAlloc( VALUE klass )
{
  GeometryGroupPtr* geometrygroup_ptr;

  geometrygroup_ptr = malloc( sizeof( GeometryGroupPtr ) );
  geometrygroup_ptr->geometrygroup = 0;

  return Data_Wrap_Struct( klass, 0, 0, geometrygroup_ptr );
}


static VALUE geometryGroupInitialize( VALUE self, VALUE context )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtGeometryGroupCreate( context_ptr->context, &( geometrygroup_ptr->geometrygroup ) ) );

  return self;
}

static VALUE geometryGroupDestroy( VALUE self )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );

  RT_CHECK_ERROR( rtGeometryGroupDestroy( geometrygroup_ptr->geometrygroup ) );
  return Qnil;
}

static VALUE geometryGroupValidate( VALUE self )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );

  RT_CHECK_ERROR( rtGeometryGroupValidate( geometrygroup_ptr->geometrygroup ) );
  return Qnil;
}

static VALUE geometryGroupGetContext( VALUE self )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtGeometryGroupGetContext( geometrygroup_ptr->geometrygroup, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE geometryGroupSetAcceleration( VALUE self, VALUE acceleration )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  AccelerationPtr* acceleration_ptr;

  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );
  Data_Get_Struct( acceleration, AccelerationPtr, acceleration_ptr);

  RT_CHECK_ERROR( rtGeometryGroupSetAcceleration( geometrygroup_ptr->geometrygroup, acceleration_ptr->acceleration ) );
  return Qnil;
}

static VALUE geometryGroupGetAcceleration( VALUE self )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  AccelerationPtr* acceleration_ptr;

  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );
  acceleration_ptr = malloc( sizeof( AccelerationPtr ) );

  RT_CHECK_ERROR( rtGeometryGroupGetAcceleration( geometrygroup_ptr->geometrygroup,
                                                  &( acceleration_ptr->acceleration ) ) );

  return Data_Wrap_Struct( class_acceleration, 0, 0, acceleration_ptr );
}

static VALUE geometryGroupSetChildCount( VALUE self, VALUE count )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );

  RT_CHECK_ERROR( rtGeometryGroupSetChildCount( geometrygroup_ptr->geometrygroup, NUM2INT( count ) ) );
  return Qnil;
}

static VALUE geometryGroupGetChildCount( VALUE self )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtGeometryGroupGetChildCount( geometrygroup_ptr->geometrygroup, &count ) );
  return INT2NUM( count );
}

static VALUE geometryGroupSetChild( VALUE self, VALUE index, VALUE geometryinstance )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  GeometryInstancePtr* geometryinstance_ptr;

  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );
  Data_Get_Struct( geometryinstance, GeometryInstancePtr, geometryinstance_ptr );

  RT_CHECK_ERROR( rtGeometryGroupSetChild( geometrygroup_ptr->geometrygroup,
                                    NUM2INT( index ),
                                    geometryinstance_ptr->geometryinstance ) );
  return Qnil;
}

static VALUE geometryGroupGetChild( VALUE self, VALUE index )
{
  RTresult result;
  GeometryGroupPtr* geometrygroup_ptr;
  GeometryInstancePtr* geometryinstance_ptr;

  Data_Get_Struct( self, GeometryGroupPtr, geometrygroup_ptr );
  geometryinstance_ptr = malloc( sizeof( GeometryInstancePtr ) );

  RTgeometryinstance child;
  RT_CHECK_ERROR( rtGeometryGroupGetChild( geometrygroup_ptr->geometrygroup, NUM2INT( index ), &child ) );

  return Data_Wrap_Struct( class_geometrygroup, 0, 0, geometryinstance_ptr );
}

/******************************************************************************\
 *
 * Acceleration 
 *
\******************************************************************************/

static VALUE accelerationAlloc( VALUE klass )
{
  AccelerationPtr* acceleration_ptr;

  acceleration_ptr = malloc( sizeof( AccelerationPtr ) );
  acceleration_ptr->acceleration = 0;

  return Data_Wrap_Struct( klass, 0, 0, acceleration_ptr );
}


static VALUE accelerationInitialize( VALUE self, VALUE context )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtAccelerationCreate( context_ptr->context, &( acceleration_ptr->acceleration ) ) );

  return self;
}

static VALUE accelerationDestroy( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  RT_CHECK_ERROR( rtAccelerationDestroy( acceleration_ptr->acceleration ) );
  return Qnil;
}

static VALUE accelerationValidate( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  RT_CHECK_ERROR( rtAccelerationValidate( acceleration_ptr->acceleration ) );
  return Qnil;
}

static VALUE accelerationGetContext( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtAccelerationGetContext( acceleration_ptr->acceleration, &( context_ptr->context ) ) );

  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE accelerationSetBuilder( VALUE self, VALUE builder )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;

  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  RT_CHECK_ERROR( rtAccelerationSetBuilder( acceleration_ptr->acceleration, StringValueCStr( builder ) ) );

  return Qnil;
}

static VALUE accelerationGetBuilder( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;

  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  const char* return_string;
  RT_CHECK_ERROR( rtAccelerationGetBuilder( acceleration_ptr->acceleration, &return_string ) );

  return rb_str_new2( return_string );
}

static VALUE accelerationSetTraverser( VALUE self, VALUE traverser )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
 
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  RT_CHECK_ERROR( rtAccelerationSetTraverser( acceleration_ptr->acceleration, StringValueCStr( traverser ) ) );

  return Qnil;
}

static VALUE accelerationGetTraverser( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  const char* return_string;
  RT_CHECK_ERROR( rtAccelerationGetTraverser( acceleration_ptr->acceleration, &return_string ) );

  return rb_str_new2( return_string );
}

static VALUE accelerationSetProperty( VALUE self, VALUE name, VALUE value )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  RT_CHECK_ERROR( rtAccelerationSetProperty( acceleration_ptr->acceleration,
                                      StringValueCStr( name ),
                                      StringValueCStr( value ) ) );
  return Qnil;
}

static VALUE accelerationGetProperty( VALUE self, VALUE name )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;

  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  const char* return_string;
  RT_CHECK_ERROR( rtAccelerationGetProperty( acceleration_ptr->acceleration,
                                             StringValueCStr( name ),
                                             &return_string ) );

  return rb_str_new2( return_string );
}

static VALUE accelerationGetDataSize( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  RTsize size;
  RT_CHECK_ERROR( rtAccelerationGetDataSize( acceleration_ptr->acceleration, &size ) );
  return INT2NUM( size );
}

/* TODO: Figure these out later
static VALUE accelerationGetData( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  void data;
  RT_CHECK_ERROR( rtAccelerationGetData( acceleration_ptr->acceleration, &data ) );
  return data;
}

static VALUE accelerationSetData( VALUE self, VALUE size )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  const void data;
  RT_CHECK_ERROR( rtAccelerationSetData( acceleration_ptr->acceleration, size, &data ) );
  return data;
}
*/

static VALUE accelerationMarkDirty( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  RT_CHECK_ERROR( rtAccelerationMarkDirty( acceleration_ptr->acceleration ) );
  return Qnil;
}

static VALUE accelerationIsDirty( VALUE self )
{
  RTresult result;
  AccelerationPtr* acceleration_ptr;
  Data_Get_Struct( self, AccelerationPtr, acceleration_ptr );

  int dirty;
  RT_CHECK_ERROR( rtAccelerationIsDirty( acceleration_ptr->acceleration, &dirty ) );
  return INT2NUM( dirty );
}


/******************************************************************************\
 *
 * GeometryInstance 
 *
\******************************************************************************/
static VALUE geometryInstanceAlloc( VALUE klass )
{
  GeometryInstancePtr* geometryinstance_ptr;

  geometryinstance_ptr = malloc( sizeof( GeometryInstancePtr ) );
  geometryinstance_ptr->geometryinstance = 0;

  return Data_Wrap_Struct( klass, 0, 0, geometryinstance_ptr );
}


static VALUE geometryInstanceInitialize( VALUE self, VALUE context )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtGeometryInstanceCreate( context_ptr->context, &( geometryinstance_ptr->geometryinstance ) ) );

  return self;
}

static VALUE geometryInstanceDestroy( VALUE self )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );

  RT_CHECK_ERROR( rtGeometryInstanceDestroy( geometryinstance_ptr->geometryinstance ) );
  return Qnil;
}

static VALUE geometryInstanceValidate( VALUE self )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );

  RT_CHECK_ERROR( rtGeometryInstanceValidate( geometryinstance_ptr->geometryinstance ) );
  return Qnil;
}

static VALUE geometryInstanceGetContext( VALUE self )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtGeometryInstanceGetContext( geometryinstance_ptr->geometryinstance, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE geometryInstanceSetGeometry( VALUE self, VALUE geometry )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  GeometryPtr* geometry_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  Data_Get_Struct( geometry, GeometryPtr, geometry_ptr );

  RT_CHECK_ERROR( rtGeometryInstanceSetGeometry( geometryinstance_ptr->geometryinstance, geometry_ptr->geometry ) );

  return Qnil;
}

static VALUE geometryInstanceGetGeometry( VALUE self )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  GeometryPtr* geometry_ptr;
  
  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  geometry_ptr = malloc( sizeof( GeometryPtr ) );

  RT_CHECK_ERROR( rtGeometryInstanceGetGeometry( geometryinstance_ptr->geometryinstance,
                                                 &( geometry_ptr->geometry ) ) );

  return Data_Wrap_Struct( class_geometry, 0, 0, geometry_ptr );
}

static VALUE geometryInstanceSetMaterialCount( VALUE self, VALUE count )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );

  RT_CHECK_ERROR( rtGeometryInstanceSetMaterialCount( geometryinstance_ptr->geometryinstance, NUM2INT( count ) ) );
  return Qnil;
}

static VALUE geometryInstanceGetMaterialCount( VALUE self )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtGeometryInstanceGetMaterialCount( geometryinstance_ptr->geometryinstance, &count ) );
  return INT2NUM( count );
}

static VALUE geometryInstanceSetMaterial( VALUE self, VALUE idx, VALUE material )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  MaterialPtr* material_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  Data_Get_Struct( material, MaterialPtr, material_ptr );
  
  RT_CHECK_ERROR( rtGeometryInstanceSetMaterial( geometryinstance_ptr->geometryinstance,
                                                 NUM2INT( idx ),
                                                 material_ptr->material ) );

  return Qnil;
}

static VALUE geometryInstanceGetMaterial( VALUE self, VALUE idx )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  MaterialPtr* material_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  material_ptr = malloc( sizeof( material_ptr ) );

  RT_CHECK_ERROR( rtGeometryInstanceGetMaterial( geometryinstance_ptr->geometryinstance,
                                          NUM2INT( idx ),
                                          &( material_ptr->material ) ) );

  return Data_Wrap_Struct( class_material, 0, 0, material_ptr );
}

static VALUE geometryInstanceDeclareVariable( VALUE self, VALUE name )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtGeometryInstanceDeclareVariable( geometryinstance_ptr->geometryinstance,
                                              StringValueCStr( name ),
                                              &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}

static VALUE geometryInstanceQueryVariable( VALUE self, VALUE name )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtGeometryInstanceQueryVariable( geometryinstance_ptr->geometryinstance,
                                            StringValueCStr( name ),
                                            &( variable_ptr->variable ) ) );
  
  if( variable_ptr->variable )
    return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
  else
    return Qnil;
}

static VALUE geometryInstanceRemoveVariable( VALUE self, VALUE v )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  Data_Get_Struct( v, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtGeometryInstanceRemoveVariable( geometryinstance_ptr->geometryinstance, variable_ptr->variable ) );
  return Qnil;
}

static VALUE geometryInstanceGetVariableCount( VALUE self )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtGeometryInstanceGetVariableCount( geometryinstance_ptr->geometryinstance, &count ) );
  return INT2NUM( count );
}

static VALUE geometryInstanceGetVariable( VALUE self, VALUE index )
{
  RTresult result;
  GeometryInstancePtr* geometryinstance_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, GeometryInstancePtr, geometryinstance_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtGeometryInstanceGetVariable( geometryinstance_ptr->geometryinstance,
                                          NUM2INT( index ),
                                          &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}


/******************************************************************************\
 *
 * Geometry 
 *
\******************************************************************************/
static VALUE geometryAlloc( VALUE klass )
{
  GeometryPtr* geometry_ptr;

  geometry_ptr = malloc( sizeof( GeometryPtr ) );
  geometry_ptr->geometry = 0;

  return Data_Wrap_Struct( klass, 0, 0, geometry_ptr );
}


static VALUE geometryInitialize( VALUE self, VALUE context )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtGeometryCreate( context_ptr->context, &( geometry_ptr->geometry ) ) );

  return self;
}

static VALUE geometryDestroy( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  Data_Get_Struct( self, GeometryPtr, geometry_ptr );

  RT_CHECK_ERROR( rtGeometryDestroy( geometry_ptr->geometry ) );
  return Qnil;
}

static VALUE geometryValidate( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  Data_Get_Struct( self, GeometryPtr, geometry_ptr );

  RT_CHECK_ERROR( rtGeometryValidate( geometry_ptr->geometry ) );
  return Qnil;
}

static VALUE geometryGetContext( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtGeometryGetContext( geometry_ptr->geometry, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE geometrySetPrimitiveCount( VALUE self, VALUE num_primitives )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  Data_Get_Struct( self, GeometryPtr, geometry_ptr );

  RT_CHECK_ERROR( rtGeometrySetPrimitiveCount( geometry_ptr->geometry, NUM2INT( num_primitives ) ) );
  return Qnil;
}

static VALUE geometryGetPrimitiveCount( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  Data_Get_Struct( self, GeometryPtr, geometry_ptr );

  unsigned int num_primitives;
  RT_CHECK_ERROR( rtGeometryGetPrimitiveCount( geometry_ptr->geometry, &num_primitives ) );
  return INT2NUM( num_primitives );
}

static VALUE geometrySetBoundingBoxProgram( VALUE self, VALUE program )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  Data_Get_Struct( program, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtGeometrySetBoundingBoxProgram( geometry_ptr->geometry, program_ptr->program ) );
  return Qnil;
}

static VALUE geometryGetBoundingBoxProgram( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  program_ptr = malloc( sizeof( ProgramPtr ) );

  RT_CHECK_ERROR( rtGeometryGetBoundingBoxProgram( geometry_ptr->geometry, &( program_ptr->program ) ) );

  return Data_Wrap_Struct( class_program, 0, 0, program_ptr );
}

static VALUE geometrySetIntersectionProgram( VALUE self, VALUE program )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  Data_Get_Struct( program, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtGeometrySetIntersectionProgram( geometry_ptr->geometry, program_ptr->program ) );
  return Qnil;
}

static VALUE geometryGetIntersectionProgram( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  program_ptr = malloc( sizeof( ProgramPtr ) );

  RT_CHECK_ERROR( rtGeometryGetIntersectionProgram( geometry_ptr->geometry, &( program_ptr->program ) ) );

  return Data_Wrap_Struct( class_program, 0, 0, program_ptr );
}

static VALUE geometryMarkDirty( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  Data_Get_Struct( self, GeometryPtr, geometry_ptr );

  RT_CHECK_ERROR( rtGeometryMarkDirty( geometry_ptr->geometry ) );
  return Qnil;
}

static VALUE geometryIsDirty( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  Data_Get_Struct( self, GeometryPtr, geometry_ptr );

  int dirty;
  RT_CHECK_ERROR( rtGeometryIsDirty( geometry_ptr->geometry, &dirty ) );
  return INT2NUM( dirty );
}

static VALUE geometryDeclareVariable( VALUE self, VALUE name )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtGeometryDeclareVariable( geometry_ptr->geometry,
                                             StringValueCStr( name ),
                                             &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}

static VALUE geometryQueryVariable( VALUE self, VALUE name )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtGeometryQueryVariable( geometry_ptr->geometry,
                                           StringValueCStr( name ),
                                           &( variable_ptr->variable ) ) );
  
  if( variable_ptr->variable )
    return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
  else
    return Qnil;
}

static VALUE geometryRemoveVariable( VALUE self, VALUE v )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  Data_Get_Struct( v, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtGeometryRemoveVariable( geometry_ptr->geometry, variable_ptr->variable ) );
  return Qnil;
}

static VALUE geometryGetVariableCount( VALUE self )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  Data_Get_Struct( self, GeometryPtr, geometry_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtGeometryGetVariableCount( geometry_ptr->geometry, &count ) );
  return INT2NUM( count );
}

static VALUE geometryGetVariable( VALUE self, VALUE index )
{
  RTresult result;
  GeometryPtr* geometry_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, GeometryPtr, geometry_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtGeometryGetVariable( geometry_ptr->geometry, NUM2INT( index ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}


/******************************************************************************\
 *
 * Material 
 *
\******************************************************************************/
static VALUE materialAlloc( VALUE klass )
{
  MaterialPtr* material_ptr;

  material_ptr = malloc( sizeof( MaterialPtr ) );
  material_ptr->material = 0;

  return Data_Wrap_Struct( klass, 0, 0, material_ptr );
}


static VALUE materialInitialize( VALUE self, VALUE context )
{
  RTresult result;
  MaterialPtr* material_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtMaterialCreate( context_ptr->context, &( material_ptr->material ) ) );

  return self;
}

static VALUE materialDestroy( VALUE self )
{
  RTresult result;
  MaterialPtr* material_ptr;
  Data_Get_Struct( self, MaterialPtr, material_ptr );

  RT_CHECK_ERROR( rtMaterialDestroy( material_ptr->material ) );
  return Qnil;
}

static VALUE materialValidate( VALUE self )
{
  RTresult result;
  MaterialPtr* material_ptr;
  Data_Get_Struct( self, MaterialPtr, material_ptr );

  RT_CHECK_ERROR( rtMaterialValidate( material_ptr->material ) );
  return Qnil;
}

static VALUE materialGetContext( VALUE self )
{
  RTresult result;
  MaterialPtr* material_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtMaterialGetContext( material_ptr->material, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE materialSetClosestHitProgram( VALUE self, VALUE ray_type_index, VALUE program )
{
  RTresult result;
  MaterialPtr* material_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  Data_Get_Struct( program, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtMaterialSetClosestHitProgram( material_ptr->material,
                                                  NUM2INT( ray_type_index ),
                                                  program_ptr->program ) );
  return Qnil;
}

static VALUE materialGetClosestHitProgram( VALUE self, VALUE ray_type_index )
{
  RTresult result;
  MaterialPtr* material_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  program_ptr = malloc( sizeof( ProgramPtr ) );

  RT_CHECK_ERROR( rtMaterialGetClosestHitProgram( material_ptr->material,
                                           NUM2INT( ray_type_index ),
                                           &( program_ptr->program ) ) );

  return Data_Wrap_Struct( class_program, 0, 0, program_ptr );
}

static VALUE materialSetAnyHitProgram( VALUE self, VALUE ray_type_index, VALUE program )
{
  RTresult result;
  MaterialPtr* material_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  Data_Get_Struct( program, ProgramPtr, program_ptr );

  RT_CHECK_ERROR( rtMaterialSetAnyHitProgram( material_ptr->material,
                                              NUM2INT( ray_type_index ),
                                              program_ptr->program ) );

  return Qnil;
}

static VALUE materialGetAnyHitProgram( VALUE self, VALUE ray_type_index )
{
  RTresult result;
  MaterialPtr* material_ptr;
  ProgramPtr* program_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  program_ptr = malloc( sizeof( ProgramPtr ) );

  RT_CHECK_ERROR( rtMaterialGetAnyHitProgram( material_ptr->material,
                                              NUM2INT( ray_type_index ),
                                              &( program_ptr->program ) ) );

  return Data_Wrap_Struct( class_program, 0, 0, program_ptr );
}

static VALUE materialDeclareVariable( VALUE self, VALUE name )
{
  RTresult result;
  MaterialPtr* material_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtMaterialDeclareVariable( material_ptr->material, StringValueCStr( name ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}

static VALUE materialQueryVariable( VALUE self, VALUE name )
{
  RTresult result;
  MaterialPtr* material_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtMaterialQueryVariable( material_ptr->material, StringValueCStr( name ), &( variable_ptr->variable ) ) );
  
  if( variable_ptr->variable )
    return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
  else
    return Qnil;
}

static VALUE materialRemoveVariable( VALUE self, VALUE v )
{
  RTresult result;
  MaterialPtr* material_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  Data_Get_Struct( v, VariablePtr, variable_ptr );

  RT_CHECK_ERROR( rtMaterialRemoveVariable( material_ptr->material, variable_ptr->variable ) );
  return Qnil;
}

static VALUE materialGetVariableCount( VALUE self )
{
  RTresult result;
  MaterialPtr* material_ptr;
  Data_Get_Struct( self, MaterialPtr, material_ptr );

  unsigned int count;
  RT_CHECK_ERROR( rtMaterialGetVariableCount( material_ptr->material, &count ) );
  return INT2NUM( count );
}

static VALUE materialGetVariable( VALUE self, VALUE index )
{
  RTresult result;
  MaterialPtr* material_ptr;
  VariablePtr* variable_ptr;

  Data_Get_Struct( self, MaterialPtr, material_ptr );
  variable_ptr = malloc( sizeof( VariablePtr ) );

  RT_CHECK_ERROR( rtMaterialGetVariable( material_ptr->material, NUM2INT( index ), &( variable_ptr->variable ) ) );
  return Data_Wrap_Struct( class_variable, 0, 0, variable_ptr );
}

/******************************************************************************\
 *
 * MappedBuffer 
 *
\******************************************************************************/
static VALUE mappedBufferAlloc( VALUE klass )
{
  MappedBufferPtr* mapped_buffer_ptr;

  mapped_buffer_ptr = malloc( sizeof( MappedBufferPtr ) );
  mapped_buffer_ptr->data = 0;
  mapped_buffer_ptr->size = 0u;

  return Data_Wrap_Struct( klass, 0, 0, mapped_buffer_ptr );
}


static VALUE mappedBufferInitialize( VALUE self, VALUE buffer )
{
  RTresult result;
  RTsize dims[3]; 
  RTsize elem_size;

  MappedBufferPtr* mapped_buffer_ptr;
  BufferPtr*       buffer_ptr;

  Data_Get_Struct( self, MappedBufferPtr, mapped_buffer_ptr );
  Data_Get_Struct( buffer, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferMap( buffer_ptr->buffer, &( mapped_buffer_ptr->data ) ) );
  RT_CHECK_ERROR( rtBufferGetSizev( buffer_ptr->buffer, 3, dims ) );
  RT_CHECK_ERROR( rtBufferGetElementSize( buffer_ptr->buffer, &elem_size ) );
  mapped_buffer_ptr->size = elem_size *  dims[0] * ( dims[1] ? dims[1] * ( dims[2] ? dims[2] : 1 ) : 1 );

  return self;
}

static VALUE mappedBufferGet( VALUE self )
{
  MappedBufferPtr* mapped_buffer_ptr;

  Data_Get_Struct( self, MappedBufferPtr, mapped_buffer_ptr );

  return rb_str_new( (const char*)(mapped_buffer_ptr->data), mapped_buffer_ptr->size );
}

static VALUE mappedBufferSet( VALUE self, VALUE data )
{
  unsigned int len;
  MappedBufferPtr* mapped_buffer_ptr;

  Data_Get_Struct( self, MappedBufferPtr, mapped_buffer_ptr );
  if( RSTRING_LEN( data ) != mapped_buffer_ptr->size ) {
    rb_raise(rb_eException, "MappedBuffer.set: mismatch -- buffer len = %i, data len = %i",
        mapped_buffer_ptr->size, RSTRING_LEN( data ) );
  }
  memcpy( mapped_buffer_ptr->data, RSTRING_PTR( data ), mapped_buffer_ptr->size );

  return Qnil;
}



/******************************************************************************\
 *
 * Buffer 
 *
\******************************************************************************/
static VALUE bufferAlloc( VALUE klass )
{
  BufferPtr* buffer_ptr;

  buffer_ptr = malloc( sizeof( BufferPtr ) );
  buffer_ptr->buffer = 0;

  return Data_Wrap_Struct( klass, 0, 0, buffer_ptr );
}


static VALUE bufferInitialize( VALUE self, VALUE context, VALUE type )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, BufferPtr, buffer_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtBufferCreate( context_ptr->context, NUM2INT( type ), &( buffer_ptr->buffer ) ) );

  return self;
}

static VALUE bufferDestroy( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferDestroy( buffer_ptr->buffer ) );
  return Qnil;
}

static VALUE bufferValidate( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferValidate( buffer_ptr->buffer ) );
  return Qnil;
}

static VALUE bufferGetContext( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, BufferPtr, buffer_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtBufferGetContext( buffer_ptr->buffer, &( context_ptr->context ) ) );

  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE bufferSetFormat( VALUE self, VALUE format )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferSetFormat( buffer_ptr->buffer, NUM2INT( format ) ) );
  return Qnil;
}

static VALUE bufferGetFormat( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RTformat format;
  RT_CHECK_ERROR( rtBufferGetFormat( buffer_ptr->buffer, &format ) );
  return INT2NUM( format );
}

static VALUE bufferSetElementSize( VALUE self, VALUE size_of_element )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferSetElementSize( buffer_ptr->buffer, NUM2INT( size_of_element ) ) );
  return Qnil;
}

static VALUE bufferGetElementSize( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RTsize size_of_element;
  RT_CHECK_ERROR( rtBufferGetElementSize( buffer_ptr->buffer, &size_of_element ) );
  return INT2NUM( size_of_element );
}

/* TODO: make these a single function with variable number args -- have createBuffer call it */
static VALUE bufferSetSize1D( VALUE self, VALUE width )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferSetSize1D( buffer_ptr->buffer, NUM2INT( width ) ) );
  return Qnil;
}

static VALUE bufferGetSize1D( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RTsize width;
  RT_CHECK_ERROR( rtBufferGetSize1D( buffer_ptr->buffer, &width ) );
  return INT2NUM( width );
}

static VALUE bufferSetSize2D( VALUE self, VALUE width, VALUE height )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferSetSize2D( buffer_ptr->buffer, NUM2INT( width ), NUM2INT( height ) ) );
  return Qnil;
}

static VALUE bufferGetSize2D( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RTsize width;
  RTsize height;
  RT_CHECK_ERROR( rtBufferGetSize2D( buffer_ptr->buffer, &width, &height ) );
  return rb_ary_new3( 2, INT2NUM( width ), INT2NUM( height ) );
}

static VALUE bufferSetSize3D( VALUE self, VALUE width, VALUE height, VALUE depth )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferSetSize3D( buffer_ptr->buffer, NUM2INT( width ), NUM2INT( height ), NUM2INT( depth ) ) );
  return Qnil;
}

static VALUE bufferGetSize3D( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RTsize width;
  RTsize height;
  RTsize depth;
  RT_CHECK_ERROR( rtBufferGetSize3D( buffer_ptr->buffer, &width, &height, &depth ) );
  return rb_ary_new3( 3, INT2NUM( width ), INT2NUM( height ), INT2NUM( depth ) );
}

static VALUE bufferGetDimensionality( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  unsigned int dimensionality;
  RT_CHECK_ERROR( rtBufferGetDimensionality( buffer_ptr->buffer, &dimensionality ) );
  return INT2NUM( dimensionality );
}

static VALUE bufferMap( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );
  VALUE args[1];

  args[0] = self;
  return rb_class_new_instance( 1, args, class_mappedbuffer );
}

static VALUE bufferUnmap( VALUE self )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtBufferUnmap( buffer_ptr->buffer ) );
  return Qnil;
}

static VALUE bufferWriteToPPM( VALUE self, VALUE filename )
{
  RTresult result;
  BufferPtr* buffer_ptr;
  Data_Get_Struct( self, BufferPtr, buffer_ptr );

  if( displayFilePPM( StringValueCStr( filename ), buffer_ptr->buffer ) != RT_SUCCESS )
    rb_raise( rb_eException, "Buffer.writeToPPM failed for file '%s'", StringValueCStr( filename ) );
  return Qnil;
}




/******************************************************************************\
 *
 * TextureSampler 
 *
\******************************************************************************/
static VALUE textureSamplerAlloc( VALUE klass )
{
  TextureSamplerPtr* texturesampler_ptr;

  texturesampler_ptr = malloc( sizeof( TextureSamplerPtr ) );
  texturesampler_ptr->texturesampler = 0;

  return Data_Wrap_Struct( klass, 0, 0, texturesampler_ptr );
}


static VALUE textureSamplerInitialize( VALUE self, VALUE context )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );
  Data_Get_Struct( context, ContextPtr, context_ptr );

  RT_CHECK_ERROR( rtTextureSamplerCreate( context_ptr->context, &( texturesampler_ptr->texturesampler ) ) );

  return self;
}

static VALUE textureSamplerDestroy( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerDestroy( texturesampler_ptr->texturesampler ) );
  return Qnil;
}

static VALUE textureSamplerValidate( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerValidate( texturesampler_ptr->texturesampler ) );
  return Qnil;
}

static VALUE textureSamplerGetContext( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  ContextPtr* context_ptr;

  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );
  context_ptr = malloc( sizeof( ContextPtr ) );

  RT_CHECK_ERROR( rtTextureSamplerGetContext( texturesampler_ptr->texturesampler, &( context_ptr->context ) ) );
  return Data_Wrap_Struct( class_context, 0, 0, context_ptr );
}

static VALUE textureSamplerSetMipLevelCount( VALUE self, VALUE num_mip_levels )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerSetMipLevelCount( texturesampler_ptr->texturesampler, NUM2INT( num_mip_levels ) ) );
  return Qnil;
}

static VALUE textureSamplerGetMipLevelCount( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  unsigned int num_mip_levels;
  RT_CHECK_ERROR( rtTextureSamplerGetMipLevelCount( texturesampler_ptr->texturesampler, &num_mip_levels ) );
  return INT2NUM( num_mip_levels );
}

static VALUE textureSamplerSetArraySize( VALUE self, VALUE num_textures_in_array )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerSetArraySize( texturesampler_ptr->texturesampler, NUM2INT( num_textures_in_array  ) ) );
  return Qnil;
}

static VALUE textureSamplerGetArraySize( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  unsigned int num_textures_in_array;
  RT_CHECK_ERROR( rtTextureSamplerGetArraySize( texturesampler_ptr->texturesampler, &num_textures_in_array ) );
  return INT2NUM( num_textures_in_array );
}

static VALUE textureSamplerSetWrapMode( VALUE self, VALUE dimension, VALUE wrapmode )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerSetWrapMode( texturesampler_ptr->texturesampler, INT2NUM( dimension ), INT2NUM( wrapmode ) ) );
  return Qnil;
}

static VALUE textureSamplerGetWrapMode( VALUE self, VALUE dimension )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RTwrapmode wrapmode;
  RT_CHECK_ERROR( rtTextureSamplerGetWrapMode( texturesampler_ptr->texturesampler, INT2NUM( dimension ), &wrapmode ) );
  return INT2NUM( wrapmode );
}

static VALUE textureSamplerSetFilteringModes( VALUE self, VALUE minification, VALUE magnification, VALUE mipmapping )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerSetFilteringModes( texturesampler_ptr->texturesampler,
                                                     INT2NUM( minification ),
                                                     INT2NUM( magnification ),
                                                     INT2NUM( mipmapping ) ) );
  return Qnil;
}

static VALUE textureSamplerGetFilteringModes( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RTfiltermode minification;
  RTfiltermode magnification;
  RTfiltermode mipmapping;
  RT_CHECK_ERROR( rtTextureSamplerGetFilteringModes( texturesampler_ptr->texturesampler,
                                                     &minification,
                                                     &magnification,
                                                     &mipmapping ) );
  return rb_ary_new3( 3, INT2NUM( minification ), INT2NUM( magnification ), INT2NUM( mipmapping ) );
}

static VALUE textureSamplerSetMaxAnisotropy( VALUE self, VALUE value )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerSetMaxAnisotropy( texturesampler_ptr->texturesampler, NUM2DBL( value ) ) );
  return Qnil;
}

static VALUE textureSamplerGetMaxAnisotropy( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  float value;
  RT_CHECK_ERROR( rtTextureSamplerGetMaxAnisotropy( texturesampler_ptr->texturesampler, &value ) );
  return rb_float_new( value );
}

static VALUE textureSamplerSetReadMode( VALUE self, VALUE readmode )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerSetReadMode( texturesampler_ptr->texturesampler, INT2NUM( readmode ) ) );
  return Qnil;
}

static VALUE textureSamplerGetReadMode( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RTtexturereadmode readmode;
  RT_CHECK_ERROR( rtTextureSamplerGetReadMode( texturesampler_ptr->texturesampler, &readmode ) );
  return INT2NUM( readmode );
}

static VALUE textureSamplerSetIndexingMode( VALUE self, VALUE indexmode )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RT_CHECK_ERROR( rtTextureSamplerSetIndexingMode( texturesampler_ptr->texturesampler, INT2NUM( indexmode ) ) );
  return Qnil;
}

static VALUE textureSamplerGetIndexingMode( VALUE self )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );

  RTtextureindexmode indexmode;
  RT_CHECK_ERROR( rtTextureSamplerGetIndexingMode( texturesampler_ptr->texturesampler, &indexmode ) );
  return INT2NUM( indexmode );
}

static VALUE textureSamplerSetBuffer( VALUE self, VALUE texture_array_idx, VALUE mip_level, VALUE buffer )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  BufferPtr* buffer_ptr;

  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );
  Data_Get_Struct( buffer, BufferPtr, buffer_ptr );

  RT_CHECK_ERROR( rtTextureSamplerSetBuffer( texturesampler_ptr->texturesampler,
                                      NUM2INT( texture_array_idx ),
                                      NUM2INT( mip_level ),
                                      buffer_ptr->buffer ) );
  return Qnil;
}

static VALUE textureSamplerGetBuffer( VALUE self, VALUE texture_array_idx, VALUE mip_level )
{
  RTresult result;
  TextureSamplerPtr* texturesampler_ptr;
  BufferPtr* buffer_ptr;

  Data_Get_Struct( self, TextureSamplerPtr, texturesampler_ptr );
  buffer_ptr = malloc( sizeof( BufferPtr ) );

  RTbuffer buffer;
  RT_CHECK_ERROR( rtTextureSamplerGetBuffer( texturesampler_ptr->texturesampler,
                                      NUM2INT( texture_array_idx ),
                                      NUM2INT( mip_level ),
                                      &( buffer_ptr->buffer ) ) );

  return Data_Wrap_Struct( class_buffer, 0, 0, buffer_ptr );
}



/******************************************************************************\
 *
 * Register, modules, classes, methods, etc
 *
\******************************************************************************/

void Init_roptix_internal()
{
  /* Module to act as namespace */
  module_optix  = rb_define_module( "Optix" );

  /* Module functions */
  rb_define_module_function( module_optix, "getElementSize", optixGetElementSize, 1 );

  /* Module constants for all OptiX enumerated values */
  rb_define_const( module_optix, "FORMAT_UNKNOWN", INT2NUM( RT_FORMAT_UNKNOWN ) );
  rb_define_const( module_optix, "FORMAT_FLOAT", INT2NUM( RT_FORMAT_FLOAT ) );
  rb_define_const( module_optix, "FORMAT_FLOAT2", INT2NUM( RT_FORMAT_FLOAT2 ) );
  rb_define_const( module_optix, "FORMAT_FLOAT3", INT2NUM( RT_FORMAT_FLOAT3 ) );
  rb_define_const( module_optix, "FORMAT_FLOAT4", INT2NUM( RT_FORMAT_FLOAT4 ) );
  rb_define_const( module_optix, "FORMAT_BYTE", INT2NUM( RT_FORMAT_BYTE ) );
  rb_define_const( module_optix, "FORMAT_BYTE2", INT2NUM( RT_FORMAT_BYTE2 ) );
  rb_define_const( module_optix, "FORMAT_BYTE3", INT2NUM( RT_FORMAT_BYTE3 ) );
  rb_define_const( module_optix, "FORMAT_BYTE4", INT2NUM( RT_FORMAT_BYTE4 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_BYTE", INT2NUM( RT_FORMAT_UNSIGNED_BYTE ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_BYTE2", INT2NUM( RT_FORMAT_UNSIGNED_BYTE2 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_BYTE3", INT2NUM( RT_FORMAT_UNSIGNED_BYTE3 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_BYTE4", INT2NUM( RT_FORMAT_UNSIGNED_BYTE4 ) );
  rb_define_const( module_optix, "FORMAT_SHORT", INT2NUM( RT_FORMAT_SHORT ) );
  rb_define_const( module_optix, "FORMAT_SHORT2", INT2NUM( RT_FORMAT_SHORT2 ) );
  rb_define_const( module_optix, "FORMAT_SHORT3", INT2NUM( RT_FORMAT_SHORT3 ) );
  rb_define_const( module_optix, "FORMAT_SHORT4", INT2NUM( RT_FORMAT_SHORT4 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_SHORT", INT2NUM( RT_FORMAT_UNSIGNED_SHORT ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_SHORT2", INT2NUM( RT_FORMAT_UNSIGNED_SHORT2 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_SHORT3", INT2NUM( RT_FORMAT_UNSIGNED_SHORT3 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_SHORT4", INT2NUM( RT_FORMAT_UNSIGNED_SHORT4 ) );
  rb_define_const( module_optix, "FORMAT_INT", INT2NUM( RT_FORMAT_INT ) );
  rb_define_const( module_optix, "FORMAT_INT2", INT2NUM( RT_FORMAT_INT2 ) ); rb_define_const( module_optix, "FORMAT_INT3", INT2NUM( RT_FORMAT_INT3 ) );
  rb_define_const( module_optix, "FORMAT_INT4", INT2NUM( RT_FORMAT_INT4 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_INT", INT2NUM( RT_FORMAT_UNSIGNED_INT ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_INT2", INT2NUM( RT_FORMAT_UNSIGNED_INT2 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_INT3", INT2NUM( RT_FORMAT_UNSIGNED_INT3 ) );
  rb_define_const( module_optix, "FORMAT_UNSIGNED_INT4", INT2NUM( RT_FORMAT_UNSIGNED_INT4 ) );
  rb_define_const( module_optix, "FORMAT_USER", INT2NUM( RT_FORMAT_USER ) );
  rb_define_const( module_optix, "OBJECTTYPE_UNKNOWN", INT2NUM( RT_OBJECTTYPE_UNKNOWN ) );
  rb_define_const( module_optix, "OBJECTTYPE_GROUP", INT2NUM( RT_OBJECTTYPE_GROUP ) );
  rb_define_const( module_optix, "OBJECTTYPE_GEOMETRY_GROUP", INT2NUM( RT_OBJECTTYPE_GEOMETRY_GROUP ) );
  rb_define_const( module_optix, "OBJECTTYPE_TRANSFORM", INT2NUM( RT_OBJECTTYPE_TRANSFORM ) );
  rb_define_const( module_optix, "OBJECTTYPE_SELECTOR", INT2NUM( RT_OBJECTTYPE_SELECTOR ) );
  rb_define_const( module_optix, "OBJECTTYPE_GEOMETRY_INSTANCE", INT2NUM( RT_OBJECTTYPE_GEOMETRY_INSTANCE ) );
  rb_define_const( module_optix, "OBJECTTYPE_BUFFER", INT2NUM( RT_OBJECTTYPE_BUFFER ) );
  rb_define_const( module_optix, "OBJECTTYPE_TEXTURE_SAMPLER", INT2NUM( RT_OBJECTTYPE_TEXTURE_SAMPLER ) );
  rb_define_const( module_optix, "OBJECTTYPE_OBJECT", INT2NUM( RT_OBJECTTYPE_OBJECT ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT2x2", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT2x2 ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT2x3", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT2x3 ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT2x4", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT2x4 ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT3x2", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT3x2 ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT3x3", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT3x3 ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT3x4", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT3x4 ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT4x2", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT4x2 ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT4x3", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT4x3 ) );
  rb_define_const( module_optix, "OBJECTTYPE_MATRIX_FLOAT4x4", INT2NUM( RT_OBJECTTYPE_MATRIX_FLOAT4x4 ) );
  rb_define_const( module_optix, "OBJECTTYPE_FLOAT", INT2NUM( RT_OBJECTTYPE_FLOAT ) );
  rb_define_const( module_optix, "OBJECTTYPE_FLOAT2", INT2NUM( RT_OBJECTTYPE_FLOAT2 ) );
  rb_define_const( module_optix, "OBJECTTYPE_FLOAT3", INT2NUM( RT_OBJECTTYPE_FLOAT3 ) );
  rb_define_const( module_optix, "OBJECTTYPE_FLOAT4", INT2NUM( RT_OBJECTTYPE_FLOAT4 ) );
  rb_define_const( module_optix, "OBJECTTYPE_INT", INT2NUM( RT_OBJECTTYPE_INT ) );
  rb_define_const( module_optix, "OBJECTTYPE_INT2", INT2NUM( RT_OBJECTTYPE_INT2 ) );
  rb_define_const( module_optix, "OBJECTTYPE_INT3", INT2NUM( RT_OBJECTTYPE_INT3 ) );
  rb_define_const( module_optix, "OBJECTTYPE_INT4", INT2NUM( RT_OBJECTTYPE_INT4 ) );
  rb_define_const( module_optix, "OBJECTTYPE_UNSIGNED_INT", INT2NUM( RT_OBJECTTYPE_UNSIGNED_INT ) );
  rb_define_const( module_optix, "OBJECTTYPE_UNSIGNED_INT2", INT2NUM( RT_OBJECTTYPE_UNSIGNED_INT2 ) );
  rb_define_const( module_optix, "OBJECTTYPE_UNSIGNED_INT3", INT2NUM( RT_OBJECTTYPE_UNSIGNED_INT3 ) );
  rb_define_const( module_optix, "OBJECTTYPE_UNSIGNED_INT4", INT2NUM( RT_OBJECTTYPE_UNSIGNED_INT4 ) );
  rb_define_const( module_optix, "OBJECTTYPE_USER", INT2NUM( RT_OBJECTTYPE_USER ) );
  rb_define_const( module_optix, "WRAP_REPEAT", INT2NUM( RT_WRAP_REPEAT ) );
  rb_define_const( module_optix, "WRAP_CLAMP_TO_EDGE", INT2NUM( RT_WRAP_CLAMP_TO_EDGE ) );
  rb_define_const( module_optix, "FILTER_NEAREST", INT2NUM( RT_FILTER_NEAREST ) );
  rb_define_const( module_optix, "FILTER_LINEAR", INT2NUM( RT_FILTER_LINEAR ) );
  rb_define_const( module_optix, "FILTER_NONE", INT2NUM( RT_FILTER_NONE ) );
  rb_define_const( module_optix, "TEXTURE_READ_ELEMENT_TYPE", INT2NUM( RT_TEXTURE_READ_ELEMENT_TYPE ) );
  rb_define_const( module_optix, "TEXTURE_READ_NORMALIZED_FLOAT", INT2NUM( RT_TEXTURE_READ_NORMALIZED_FLOAT ) );
  rb_define_const( module_optix, "TEXTURE_INDEX_NORMALIZED_COORDINATES",
                                 INT2NUM( RT_TEXTURE_INDEX_NORMALIZED_COORDINATES ) );
  rb_define_const( module_optix, "TEXTURE_INDEX_ARRAY_INDEX", INT2NUM( RT_TEXTURE_INDEX_ARRAY_INDEX ) );
  rb_define_const( module_optix, "BUFFER_INPUT", INT2NUM( RT_BUFFER_INPUT ) );
  rb_define_const( module_optix, "BUFFER_OUTPUT", INT2NUM( RT_BUFFER_OUTPUT ) );
  rb_define_const( module_optix, "BUFFER_INPUT_OUTPUT", INT2NUM( RT_BUFFER_INPUT_OUTPUT ) );

  /* Variable class */
  class_variable = rb_define_class_under( module_optix, "Variable", rb_cObject );
  rb_define_alloc_func( class_variable, variableAlloc );
  rb_define_method( class_variable, "initialize", variableInitialize, 1 );
  rb_define_method( class_variable, "set1f", variableSet1f, 1 );
  rb_define_method( class_variable, "set2f", variableSet2f, 2 );
  rb_define_method( class_variable, "set3f", variableSet3f, 3 );
  rb_define_method( class_variable, "set4f", variableSet4f, 4 );
  rb_define_method( class_variable, "set1i", variableSet1i, 1 );
  rb_define_method( class_variable, "set2i", variableSet2i, 2 );
  rb_define_method( class_variable, "set3i", variableSet3i, 3 );
  rb_define_method( class_variable, "set4i", variableSet4i, 4 );
  rb_define_method( class_variable, "set1ui", variableSet1ui, 1 );
  rb_define_method( class_variable, "set2ui", variableSet2ui, 2 );
  rb_define_method( class_variable, "set3ui", variableSet3ui, 3 );
  rb_define_method( class_variable, "set4ui", variableSet4ui, 4 );
  rb_define_method( class_variable, "setMatrix2x2", variableSetMatrix2x2, 1 );
  rb_define_method( class_variable, "setMatrix2x3", variableSetMatrix2x3, 1 );
  rb_define_method( class_variable, "setMatrix2x4", variableSetMatrix2x4, 1 );
  rb_define_method( class_variable, "setMatrix3x2", variableSetMatrix3x2, 1 );
  rb_define_method( class_variable, "setMatrix3x3", variableSetMatrix3x3, 1 );
  rb_define_method( class_variable, "setMatrix3x4", variableSetMatrix3x4, 1 );
  rb_define_method( class_variable, "setMatrix4x2", variableSetMatrix4x2, 1 );
  rb_define_method( class_variable, "setMatrix4x3", variableSetMatrix4x3, 1 );
  rb_define_method( class_variable, "setMatrix4x4", variableSetMatrix4x4, 1 );
  rb_define_method( class_variable, "setObject", variableSetObject, 1 );
  //rb_define_method( class_variable, "setUserData", variableSetUserData, 1 );
  rb_define_method( class_variable, "get1f", variableGet1f, 0 );
  rb_define_method( class_variable, "get2f", variableGet2f, 0 );
  rb_define_method( class_variable, "get3f", variableGet3f, 0 );
  rb_define_method( class_variable, "get4f", variableGet4f, 0 );
  rb_define_method( class_variable, "get1fv", variableGet1fv, 0 );
  rb_define_method( class_variable, "get2fv", variableGet2fv, 0 );
  rb_define_method( class_variable, "get3fv", variableGet3fv, 0 );
  rb_define_method( class_variable, "get4fv", variableGet4fv, 0 );
  rb_define_method( class_variable, "get1i", variableGet1i, 0 );
  rb_define_method( class_variable, "get2i", variableGet2i, 0 );
  rb_define_method( class_variable, "get3i", variableGet3i, 0 );
  rb_define_method( class_variable, "get4i", variableGet4i, 0 );
  rb_define_method( class_variable, "get1iv", variableGet1iv, 0 );
  rb_define_method( class_variable, "get2iv", variableGet2iv, 0 );
  rb_define_method( class_variable, "get3iv", variableGet3iv, 0 );
  rb_define_method( class_variable, "get4iv", variableGet4iv, 0 );
  rb_define_method( class_variable, "get1ui", variableGet1ui, 0 );
  rb_define_method( class_variable, "get2ui", variableGet2ui, 0 );
  rb_define_method( class_variable, "get3ui", variableGet3ui, 0 );
  rb_define_method( class_variable, "get4ui", variableGet4ui, 0 );
  rb_define_method( class_variable, "get1uiv", variableGet1uiv, 0 );
  rb_define_method( class_variable, "get2uiv", variableGet2uiv, 0 );
  rb_define_method( class_variable, "get3uiv", variableGet3uiv, 0 );
  rb_define_method( class_variable, "get4uiv", variableGet4uiv, 0 );
  rb_define_method( class_variable, "getMatrix2x2fv", variableGetMatrix2x2fv, 1 );
  rb_define_method( class_variable, "getMatrix2x3fv", variableGetMatrix2x3fv, 1 );
  rb_define_method( class_variable, "getMatrix2x4fv", variableGetMatrix2x4fv, 1 );
  rb_define_method( class_variable, "getMatrix3x2fv", variableGetMatrix3x2fv, 1 );
  rb_define_method( class_variable, "getMatrix3x3fv", variableGetMatrix3x3fv, 1 );
  rb_define_method( class_variable, "getMatrix3x4fv", variableGetMatrix3x4fv, 1 );
  rb_define_method( class_variable, "getMatrix4x2fv", variableGetMatrix4x2fv, 1 );
  rb_define_method( class_variable, "getMatrix4x3fv", variableGetMatrix4x3fv, 1 );
  rb_define_method( class_variable, "getMatrix4x4fv", variableGetMatrix4x4fv, 1 );
  //rb_define_method( class_variable, "getObject", variableGetObject, 0 );
  //rb_define_method( class_variable, "getUserData", variableGetUserData, 1 );
  rb_define_method( class_variable, "getName", variableGetName, 0 );
  rb_define_method( class_variable, "getAnnotation", variableGetAnnotation, 0 );
  rb_define_method( class_variable, "getType", variableGetType, 0 );
  rb_define_method( class_variable, "getContext", variableGetContext, 0 );

  /* Context */
  class_context = rb_define_class_under( module_optix, "Context", rb_cObject );
  rb_define_alloc_func( class_context, contextAlloc );
  rb_define_method( class_context, "initialize", contextInitialize, 0 );
  rb_define_singleton_method( class_context, "create", contextCreate, 0 );

  rb_define_method( class_context, "createVariable", contextCreateVariable, 0 );
  rb_define_method( class_context, "createProgramFromPTXFile", contextCreateProgramFromPTXFile, 2 );
  rb_define_method( class_context, "createProgramFromPTXString", contextCreateProgramFromPTXString, 2 );
  rb_define_method( class_context, "createGroup", contextCreateGroup, 0 );
  rb_define_method( class_context, "createSelector", contextCreateSelector, 0 );
  rb_define_method( class_context, "createTransform", contextCreateTransform, 0 );
  rb_define_method( class_context, "createGeometryGroup", contextCreateGeometryGroup, 0 );
  rb_define_method( class_context, "createAcceleration", contextCreateAcceleration, 0 );
  rb_define_method( class_context, "createGeometryInstance", contextCreateGeometryInstance, 0 );
  rb_define_method( class_context, "createGeometry", contextCreateGeometry, 0 );
  rb_define_method( class_context, "createMaterial", contextCreateMaterial, 0 );
  rb_define_method( class_context, "createBuffer", contextCreateBuffer, -1 );
  rb_define_method( class_context, "createTextureSampler", contextCreateTextureSampler, 0 );

  rb_define_method( class_context, "destroy", contextDestroy, 0 );
  rb_define_method( class_context, "validate", contextValidate, 0 );
  rb_define_method( class_context, "setDevices", contextSetDevices, 1 );
  rb_define_method( class_context, "setStackSize", contextSetStackSize, 1 );
  rb_define_method( class_context, "getStackSize", contextGetStackSize, 0 );
  rb_define_method( class_context, "setEntryPointCount", contextSetEntryPointCount, 1 );
  rb_define_method( class_context, "getEntryPointCount", contextGetEntryPointCount, 0 );
  rb_define_method( class_context, "setRayGenerationProgram", contextSetRayGenerationProgram, 2 );
  rb_define_method( class_context, "getRayGenerationProgram", contextGetRayGenerationProgram, 1 );
  rb_define_method( class_context, "setExceptionProgram", contextSetExceptionProgram, 2 );
  rb_define_method( class_context, "getExceptionProgram", contextGetExceptionProgram, 1 );
  rb_define_method( class_context, "setRayTypeCount", contextSetRayTypeCount, 1 );
  rb_define_method( class_context, "getRayTypeCount", contextGetRayTypeCount, 0 );
  rb_define_method( class_context, "setMissProgram", contextSetMissProgram, 2 );
  rb_define_method( class_context, "getMissProgram", contextGetMissProgram, 1 );
  rb_define_method( class_context, "compile", contextCompile, 0 );
  rb_define_method( class_context, "launch", contextLaunch, -1 );
  rb_define_method( class_context, "getRunningState", contextGetRunningState, 0 );
  rb_define_method( class_context, "setPrintEnabled", contextSetPrintEnabled, 1 );
  rb_define_method( class_context, "getPrintEnabled", contextGetPrintEnabled, 0 );
  rb_define_method( class_context, "setPrintBufferSize", contextSetPrintBufferSize, 1 );
  rb_define_method( class_context, "getPrintBufferSize", contextGetPrintBufferSize, 0 );
  rb_define_method( class_context, "setPrintLaunchIndex", contextSetPrintLaunchIndex, 3 );
  rb_define_method( class_context, "getPrintLaunchIndex", contextGetPrintLaunchIndex, 0 );
  rb_define_method( class_context, "declareVariable", contextDeclareVariable, 1 );
  rb_define_method( class_context, "queryVariable", contextQueryVariable, 1 );
  rb_define_method( class_context, "removeVariable", contextRemoveVariable, 1 );
  rb_define_method( class_context, "getVariableCount", contextGetVariableCount, 0 );
  rb_define_method( class_context, "getVariable", contextGetVariable, 1 );

  /* Program */
  class_program = rb_define_class_under( module_optix, "Program", rb_cObject );
  rb_define_alloc_func( class_program, programAlloc );
  rb_define_method( class_program, "initialize", programInitialize, 4 );
  rb_define_method( class_program, "destroy", programDestroy, 0 );
  rb_define_method( class_program, "validate", programValidate, 0 );
  rb_define_method( class_program, "getContext", programGetContext, 0 );
  rb_define_method( class_program, "declareVariable", programDeclareVariable, 1 );
  rb_define_method( class_program, "queryVariable", programQueryVariable, 1 );
  rb_define_method( class_program, "removeVariable", programRemoveVariable, 1 );
  rb_define_method( class_program, "getVariableCount", programGetVariableCount, 0 );
  rb_define_method( class_program, "getVariable", programGetVariable, 1 );

  /* Group */
  class_group = rb_define_class_under( module_optix, "Group", rb_cObject );
  rb_define_alloc_func( class_group, groupAlloc );
  rb_define_method( class_group, "initialize", groupInitialize, 1 );
  rb_define_method( class_group, "destroy", groupDestroy, 0 );
  rb_define_method( class_group, "validate", groupValidate, 0 );
  rb_define_method( class_group, "getContext", groupGetContext, 0 );
  rb_define_method( class_group, "setAcceleration", groupSetAcceleration, 1 );
  rb_define_method( class_group, "getAcceleration", groupGetAcceleration, 0 );
  rb_define_method( class_group, "setChildCount", groupSetChildCount, 1 );
  rb_define_method( class_group, "getChildCount", groupGetChildCount, 0 );
  rb_define_method( class_group, "setChild", groupSetChild, 2 );
  rb_define_method( class_group, "getChild", groupGetChild, 1 );
  //rb_define_method( class_group, "getChildType", groupGetChildType, 1 );

  /* Selector */
  class_selector = rb_define_class_under( module_optix, "Selector", rb_cObject );
  rb_define_alloc_func( class_selector, selectorAlloc );
  rb_define_method( class_selector, "initialize", selectorInitialize, 1 );
  rb_define_method( class_selector, "destroy", selectorDestroy, 0 );
  rb_define_method( class_selector, "validate", selectorValidate, 0 );
  rb_define_method( class_selector, "getContext", selectorGetContext, 0 );
  rb_define_method( class_selector, "setVisitProgram", selectorSetVisitProgram, 1 );
  rb_define_method( class_selector, "getVisitProgram", selectorGetVisitProgram, 0 );
  rb_define_method( class_selector, "setChildCount", selectorSetChildCount, 1 );
  rb_define_method( class_selector, "getChildCount", selectorGetChildCount, 0 );
  rb_define_method( class_selector, "setChild", selectorSetChild, 2 );
  rb_define_method( class_selector, "getChild", selectorGetChild, 1 );
  //rb_define_method( class_selector, "getChildType", selectorGetChildType, 1 );
  rb_define_method( class_selector, "declareVariable", selectorDeclareVariable, 1 );
  rb_define_method( class_selector, "queryVariable", selectorQueryVariable, 1 );
  rb_define_method( class_selector, "removeVariable", selectorRemoveVariable, 1 );
  rb_define_method( class_selector, "getVariableCount", selectorGetVariableCount, 0 );
  rb_define_method( class_selector, "getVariable", selectorGetVariable, 1 );

  /* Transform */
  class_transform = rb_define_class_under( module_optix, "Transform", rb_cObject );
  rb_define_alloc_func( class_transform, transformAlloc );
  rb_define_method( class_transform, "initialize", transformInitialize, 1 );
  rb_define_method( class_transform, "destroy", transformDestroy, 0 );
  rb_define_method( class_transform, "validate", transformValidate, 0 );
  rb_define_method( class_transform, "getContext", transformGetContext, 0 );
  rb_define_method( class_transform, "setMatrix", transformSetMatrix, 1 );
  rb_define_method( class_transform, "getMatrix", transformGetMatrix, 1 );
  rb_define_method( class_transform, "getMatrix", transformGetMatrixInverse, 1 );
  rb_define_method( class_transform, "setChild", transformSetChild, 1 );
  rb_define_method( class_transform, "getChild", transformGetChild, 0 );
  //rb_define_method( class_transform, "getChildType", transformGetChildType, 0 );

  /* GeometryGroup */
  class_geometrygroup = rb_define_class_under( module_optix, "GeometryGroup", rb_cObject );
  rb_define_alloc_func( class_geometrygroup, geometryGroupAlloc );
  rb_define_method( class_geometrygroup, "initialize", geometryGroupInitialize, 1 );
  rb_define_method( class_geometrygroup, "destroy", geometryGroupDestroy, 0 );
  rb_define_method( class_geometrygroup, "validate", geometryGroupValidate, 0 );
  rb_define_method( class_geometrygroup, "getContext", geometryGroupGetContext, 0 );
  rb_define_method( class_geometrygroup, "setAcceleration", geometryGroupSetAcceleration, 1 );
  rb_define_method( class_geometrygroup, "getAcceleration", geometryGroupGetAcceleration, 0 );
  rb_define_method( class_geometrygroup, "setChildCount", geometryGroupSetChildCount, 1 );
  rb_define_method( class_geometrygroup, "getChildCount", geometryGroupGetChildCount, 0 );
  rb_define_method( class_geometrygroup, "setChild", geometryGroupSetChild, 2 );
  rb_define_method( class_geometrygroup, "getChild", geometryGroupGetChild, 1 );

  /* Acceleration */
  class_acceleration = rb_define_class_under( module_optix, "Acceleration", rb_cObject );
  rb_define_alloc_func( class_acceleration, accelerationAlloc );
  rb_define_method( class_acceleration, "initialize", accelerationInitialize, 1 );
  rb_define_method( class_acceleration, "destroy", accelerationDestroy, 0 );
  rb_define_method( class_acceleration, "validate", accelerationValidate, 0 );
  rb_define_method( class_acceleration, "getContext", accelerationGetContext, 0 );
  rb_define_method( class_acceleration, "setBuilder", accelerationSetBuilder, 1 );
  rb_define_method( class_acceleration, "getBuilder", accelerationGetBuilder, 0 );
  rb_define_method( class_acceleration, "setTraverser", accelerationSetTraverser, 1 );
  rb_define_method( class_acceleration, "getTraverser", accelerationGetTraverser, 0 );
  rb_define_method( class_acceleration, "setProperty", accelerationSetProperty, 2 );
  rb_define_method( class_acceleration, "getProperty", accelerationGetProperty, 1 );
  rb_define_method( class_acceleration, "getDataSize", accelerationGetDataSize, 0 );
  //rb_define_method( class_acceleration, "getData", accelerationGetData, 0 );
  //rb_define_method( class_acceleration, "setData", accelerationSetData, 1 );
  rb_define_method( class_acceleration, "markDirty", accelerationMarkDirty, 0 );
  rb_define_method( class_acceleration, "isDirty", accelerationIsDirty, 0 );

  /* GeometryInstance */
  class_geometryinstance = rb_define_class_under( module_optix, "GeometryInstance", rb_cObject );
  rb_define_alloc_func( class_geometryinstance, geometryInstanceAlloc );
  rb_define_method( class_geometryinstance, "initialize", geometryInstanceInitialize, 1 );
  rb_define_method( class_geometryinstance, "destroy", geometryInstanceDestroy, 0 );
  rb_define_method( class_geometryinstance, "validate", geometryInstanceValidate, 0 );
  rb_define_method( class_geometryinstance, "getContext", geometryInstanceGetContext, 0 );
  rb_define_method( class_geometryinstance, "setGeometry", geometryInstanceSetGeometry, 1 );
  rb_define_method( class_geometryinstance, "getGeometry", geometryInstanceGetGeometry, 0 );
  rb_define_method( class_geometryinstance, "setMaterialCount", geometryInstanceSetMaterialCount, 1 );
  rb_define_method( class_geometryinstance, "getMaterialCount", geometryInstanceGetMaterialCount, 0 );
  rb_define_method( class_geometryinstance, "setMaterial", geometryInstanceSetMaterial, 2 );
  rb_define_method( class_geometryinstance, "getMaterial", geometryInstanceGetMaterial, 1 );
  rb_define_method( class_geometryinstance, "declareVariable", geometryInstanceDeclareVariable, 1 );
  rb_define_method( class_geometryinstance, "queryVariable", geometryInstanceQueryVariable, 1 );
  rb_define_method( class_geometryinstance, "removeVariable", geometryInstanceRemoveVariable, 1 );
  rb_define_method( class_geometryinstance, "getVariableCount", geometryInstanceGetVariableCount, 0 );
  rb_define_method( class_geometryinstance, "getVariable", geometryInstanceGetVariable, 1 );

  /* Geometry */
  class_geometry = rb_define_class_under( module_optix, "Geometry", rb_cObject );
  rb_define_alloc_func( class_geometry, geometryAlloc );
  rb_define_method( class_geometry, "initialize", geometryInitialize, 1 );
  rb_define_method( class_geometry, "destroy", geometryDestroy, 0 );
  rb_define_method( class_geometry, "validate", geometryValidate, 0 );
  rb_define_method( class_geometry, "getContext", geometryGetContext, 0 );
  rb_define_method( class_geometry, "setPrimitiveCount", geometrySetPrimitiveCount, 1 );
  rb_define_method( class_geometry, "getPrimitiveCount", geometryGetPrimitiveCount, 0 );
  rb_define_method( class_geometry, "setBoundingBoxProgram", geometrySetBoundingBoxProgram, 1 );
  rb_define_method( class_geometry, "getBoundingBoxProgram", geometryGetBoundingBoxProgram, 0 );
  rb_define_method( class_geometry, "setIntersectionProgram", geometrySetIntersectionProgram, 1 );
  rb_define_method( class_geometry, "getIntersectionProgram", geometryGetIntersectionProgram, 0 );
  rb_define_method( class_geometry, "markDirty", geometryMarkDirty, 0 );
  rb_define_method( class_geometry, "isDirty", geometryIsDirty, 0 );
  rb_define_method( class_geometry, "declareVariable", geometryDeclareVariable, 1 );
  rb_define_method( class_geometry, "queryVariable", geometryQueryVariable, 1 );
  rb_define_method( class_geometry, "removeVariable", geometryRemoveVariable, 1 );
  rb_define_method( class_geometry, "getVariableCount", geometryGetVariableCount, 0 );
  rb_define_method( class_geometry, "getVariable", geometryGetVariable, 1 );

  /* Material */
  class_material = rb_define_class_under( module_optix, "Material", rb_cObject );
  rb_define_alloc_func( class_material, materialAlloc );
  rb_define_method( class_material, "initialize", materialInitialize, 1 );
  rb_define_method( class_material, "destroy", materialDestroy, 0 );
  rb_define_method( class_material, "validate", materialValidate, 0 );
  rb_define_method( class_material, "getContext", materialGetContext, 0 );
  rb_define_method( class_material, "setClosestHitProgram", materialSetClosestHitProgram, 2 );
  rb_define_method( class_material, "getClosestHitProgram", materialGetClosestHitProgram, 1 );
  rb_define_method( class_material, "setAnyHitProgram", materialSetAnyHitProgram, 2 );
  rb_define_method( class_material, "getAnyHitProgram", materialGetAnyHitProgram, 1 );
  rb_define_method( class_material, "declareVariable", materialDeclareVariable, 1 );
  rb_define_method( class_material, "queryVariable", materialQueryVariable, 1 );
  rb_define_method( class_material, "removeVariable", materialRemoveVariable, 1 );
  rb_define_method( class_material, "getVariableCount", materialGetVariableCount, 0 );
  rb_define_method( class_material, "getVariable", materialGetVariable, 1 );

  /* Buffer */
  class_buffer = rb_define_class_under( module_optix, "Buffer", rb_cObject );
  rb_define_alloc_func( class_buffer, bufferAlloc );
  rb_define_method( class_buffer, "initialize", bufferInitialize, 2 );
  rb_define_method( class_buffer, "destroy", bufferDestroy, 0 );
  rb_define_method( class_buffer, "validate", bufferValidate, 0 );
  rb_define_method( class_buffer, "getContext", bufferGetContext, 0 );
  rb_define_method( class_buffer, "setFormat", bufferSetFormat, 1 );
  rb_define_method( class_buffer, "getFormat", bufferGetFormat, 0 );
  rb_define_method( class_buffer, "setElementSize", bufferSetElementSize, 1 );
  rb_define_method( class_buffer, "getElementSize", bufferGetElementSize, 0 );
  rb_define_method( class_buffer, "setSize1D", bufferSetSize1D, 1 );
  rb_define_method( class_buffer, "getSize1D", bufferGetSize1D, 0 );
  rb_define_method( class_buffer, "setSize2D", bufferSetSize2D, 2 );
  rb_define_method( class_buffer, "getSize2D", bufferGetSize2D, 0 );
  rb_define_method( class_buffer, "setSize3D", bufferSetSize3D, 3 );
  rb_define_method( class_buffer, "getSize3D", bufferGetSize3D, 0 );
  rb_define_method( class_buffer, "getDimensionality", bufferGetDimensionality, 0 );
  rb_define_method( class_buffer, "map", bufferMap, 0 );
  rb_define_method( class_buffer, "unmap", bufferUnmap, 0 );
  rb_define_method( class_buffer, "writeToPPM", bufferWriteToPPM, 1 );

  /* MappedBuffer */
  class_mappedbuffer = rb_define_class_under( module_optix, "MappedBuffer", rb_cObject );
  rb_define_alloc_func( class_mappedbuffer, mappedBufferAlloc );
  rb_define_method( class_mappedbuffer, "initialize", mappedBufferInitialize, 1 );
  rb_define_method( class_mappedbuffer, "get", mappedBufferGet, 0 );
  rb_define_method( class_mappedbuffer, "set", mappedBufferSet, 1 );

  /* Texture Sampler */
  class_texturesampler = rb_define_class_under( module_optix, "TextureSampler", rb_cObject );
  rb_define_alloc_func( class_texturesampler, textureSamplerAlloc );
  rb_define_method( class_texturesampler, "initialize", textureSamplerInitialize, 0 );
  rb_define_method( class_texturesampler, "destroy", textureSamplerDestroy, 0 );
  rb_define_method( class_texturesampler, "validate", textureSamplerValidate, 0 );
  rb_define_method( class_texturesampler, "getContext", textureSamplerGetContext, 0 );
  rb_define_method( class_texturesampler, "setMipLevelCount", textureSamplerSetMipLevelCount, 1 );
  rb_define_method( class_texturesampler, "getMipLevelCount", textureSamplerGetMipLevelCount, 0 );
  rb_define_method( class_texturesampler, "setArraySize", textureSamplerSetArraySize, 1 );
  rb_define_method( class_texturesampler, "getArraySize", textureSamplerGetArraySize, 0 );
  rb_define_method( class_texturesampler, "setWrapMode", textureSamplerSetWrapMode, 2 );
  rb_define_method( class_texturesampler, "getWrapMode", textureSamplerGetWrapMode, 1 );
  rb_define_method( class_texturesampler, "setFilteringModes", textureSamplerSetFilteringModes, 3 );
  rb_define_method( class_texturesampler, "getFilteringModes", textureSamplerGetFilteringModes, 0 );
  rb_define_method( class_texturesampler, "setMaxAnisotropy", textureSamplerSetMaxAnisotropy, 1 );
  rb_define_method( class_texturesampler, "getMaxAnisotropy", textureSamplerGetMaxAnisotropy, 0 );
  rb_define_method( class_texturesampler, "setReadMode", textureSamplerSetReadMode, 1 );
  rb_define_method( class_texturesampler, "getReadMode", textureSamplerGetReadMode, 0 );
  rb_define_method( class_texturesampler, "setIndexingMode", textureSamplerSetIndexingMode, 1 );
  rb_define_method( class_texturesampler, "getIndexingMode", textureSamplerGetIndexingMode, 0 );
  rb_define_method( class_texturesampler, "setBuffer", textureSamplerSetBuffer, 3 );
  rb_define_method( class_texturesampler, "getBuffer", textureSamplerGetBuffer, 2 );
}
