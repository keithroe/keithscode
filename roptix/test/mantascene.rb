
class MantaScene
  @@WIDTH  = 512
  @@HEIGHT = 512

  def initialize()
    @context = nil
    @output_buffer = nil
  end

  def initScene()
    @context = Context::create()
    @context.setRayTypeCount( 2 )
    @context.setEntryPointCount( 1 )
    @context.setStackSize( 1568 )

    variable = @context.declareVariable( "max_depth" )
    variable.set1i( 4 )
    variable = @context.declareVariable( "radiance_ray_type" )
    variable.set1ui( 0 )
    variable = @context.declareVariable( "shadow_ray_type" )
    variable.set1ui( 1 )
    variable = @context.declareVariable( "scene_epsilon" )
    variable.set1f( 1e-4 )

    variable = @context.declareVariable( "output_buffer" )
    @output_buffer = @context.createBuffer( BUFFER_OUTPUT, FORMAT_UNSIGNED_BYTE4, @@WIDTH, @@HEIGHT )
    variable.setObject( @output_buffer )

    # ray gen prog 
    ray_gen_program = @context.createProgramFromPTXFile( "#{$dir}/ptx/pinhole_camera.ptx", "pinhole_camera" )
    @context.setRayGenerationProgram( 0, ray_gen_program )
    
    # exception program
    exception_program = @context.createProgramFromPTXFile( "#{$dir}/ptx/pinhole_camera.ptx", "exception" )
    @context.setExceptionProgram( 0, exception_program )
    variable = @context.declareVariable( "bad_color" )
    variable.set3f( 1.0, 1.0, 0.0 )

    # miss prog
    miss_program = @context.createProgramFromPTXFile( "#{$dir}/ptx/constantbg.ptx", "miss" )
    @context.setMissProgram( 0, miss_program)
    variable = @context.declareVariable( "bg_color" )
    variable.set3f( 54.0/255.0, 83.0/255.0, 102.5/255.0 ) 

    @context["ambient_light_color"]->setFloat(0,0,0);
   
    # TODO: create FormatUser class that packs itself and knows its size
    lights = [
      [ [ 0.0, 8.0, -5.0 ], [ .6, .1, .1 ], 1 ],
      [ [ 5.0, 8.0,  0.0 ], [ .1, .6, .1 ], 1 ],
      [ [ 5.0, 2.0, -5.0 ], [ .2, .2, .2 ], 1 ]
    ]
    
    variable = @context.createBuffer( RT_BUFFER_INPUT )
    light_buffer->setFormat( RT_FORMAT_USER )
    light_buffer->setElementSize(  )

    lights.flatten.pack( "ffffffi"*lights.size )

 








  end
end
