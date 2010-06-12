#!/usr/bin/ruby -w

$dir = File.dirname( __FILE__ )
require  $dir + '/test_helper'

include Optix   # using namespace

class Sample5

  @@WIDTH  = 1024
  @@HEIGHT = 768 

  def initialize
    @context  = nil
    @sphere   = nil
    @material = nil
    @output_buffer = nil
  end

  def createContext
    context = Context::create()
    context.setRayTypeCount( 1 )
    context.setEntryPointCount( 1 )

    # should be context["radiance_ray_type"].set( 0 )
    variable = context.declareVariable( "radiance_ray_type" )
    variable.set1ui( 0 )
    variable = context.declareVariable( "scene_epsilon" )
    variable.set1f( 0.0001 )
  
    output_buffer = context.declareVariable( "output_buffer" )
    @output_buffer = context.createBuffer( BUFFER_OUTPUT, FORMAT_UNSIGNED_BYTE4, @@WIDTH, @@HEIGHT )
    output_buffer.setObject( @output_buffer ) # TODO:test this

    # ray gen prog 
    # TODO: make context.createProgramFromPTXFile( string1, string2)
    ray_gen_program = context.createProgramFromPTXFile( "#{$dir}/ptx/pinhole_camera.ptx", "pinhole_camera" )
    context.setRayGenerationProgram( 0, ray_gen_program )

    variable = context.declareVariable( "eye" )
    variable.set3f( 0, 0, 5 )
    variable = context.declareVariable( "U" )
    variable.set3f( 2.88675, 0, 0 )
    variable = context.declareVariable( "V" )
    variable.set3f( 0, 2.16506, 0 )
    variable = context.declareVariable( "W" )
    variable.set3f( 0, 0, -5 )

    # exception program
    exception_program = context.createProgramFromPTXFile( "#{$dir}/ptx/pinhole_camera.ptx", "exception" )
    context.setExceptionProgram( 0, exception_program )
    variable = context.declareVariable( "bad_color" )
    variable.set3f( 1.0, 1.0, 0.0 )
                                                          
    # miss prog
    miss_program = context.createProgramFromPTXFile( "#{$dir}/ptx/constantbg.ptx", "miss" )
    context.setMissProgram( 0, miss_program)
    variable = context.declareVariable( "bg_color" )
    variable.set3f( 0.3, 0.1, 0.2 )

    return context
  end

  def createGeometry
    sphere = @context.createGeometry()
    sphere.setPrimitiveCount( 1 )
    ptx_path = "#{$dir}/ptx/sphere.ptx"
    sphere.setBoundingBoxProgram( @context.createProgramFromPTXFile( ptx_path, "bounds" ) )
    sphere.setIntersectionProgram( @context.createProgramFromPTXFile( ptx_path, "intersect" ) )
    variable = sphere.declareVariable( "sphere" )
    variable.set4f( 0, 0, 0, 1.5 )
    return sphere;
  end

  def createMaterial

    chp = @context.createProgramFromPTXFile( "#{$dir}/ptx/normal_shader.ptx", "closest_hit_radiance" )
    matl = @context.createMaterial()
    matl.setClosestHitProgram( 0, chp )
    return matl
  end
  
  def createInstance
    # Create geometry instance
    gi = @context.createGeometryInstance()
    gi.setMaterialCount( 1 )
    gi.setGeometry( @sphere )
    gi.setMaterial( 0, @material )

    # Create geometry group
    geometrygroup = @context.createGeometryGroup()
    geometrygroup.setChildCount( 1 )
    puts "geometrygroup child count #{geometrygroup.getChildCount}"
    geometrygroup.setChild( 0, gi )

    accel =  @context.createAcceleration() # TODO: Context.createAcceleration( trav, build )
    accel.setTraverser( "NoAccel" )
    accel.setBuilder( "NoAccel" )
    geometrygroup.setAcceleration( accel )

    variable = @context.declareVariable( "top_object" )
    variable.setObject( geometrygroup )
  end

  def run()
    @context  = createContext()
    @sphere   = createGeometry()
    @material = createMaterial()
    createInstance()

    @context.validate()
    @context.compile()
    @context.launch( 0, @@WIDTH, @@HEIGHT)

    # display
    @output_buffer.writeToPPM "foo.ppm" 

    
    @context.destroy()
    puts "here 8"
  end

end



Sample5.new.run
