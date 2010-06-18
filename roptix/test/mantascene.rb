
$dir = File.dirname( __FILE__ )
require  $dir + '/test_helper'
require  'matrix' 

include Optix

class MantaScene < OptixViewer

  def initialize
    super
    @context = nil
    @output_buffer = nil
  end

  def getOutputBuffer
    @output_buffer
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
    @output_buffer = @context.createBuffer( BUFFER_OUTPUT, FORMAT_UNSIGNED_BYTE4, @width, @height )
    variable.setObject( @output_buffer )

    # ray gen prog 
    ray_gen_program = @context.createProgramFromPTXFile( "#{$dir}/ptx/pinhole_camera.ptx", "pinhole_camera" )
    @context.setRayGenerationProgram( 0, ray_gen_program )
    
    eye = [3, 2, -3]
    u,v,w = cameraParams( eye, [ 0, 0.3, 0 ], [0,1,0], 60.0, @width.to_f / @height.to_f )
    variable = @context.declareVariable( "eye" )
    variable.set3f( *eye )
    variable = @context.declareVariable( "U" )
    variable.set3f( *u )
    variable = @context.declareVariable( "V" )
    variable.set3f( *v )
    variable = @context.declareVariable( "W" )
    variable.set3f( *w )

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

    variable = @context.declareVariable( "ambient_light_color" )
    variable.set3f(0,0,0)
   
    # TODO: create FormatUser base class that packs itself and knows its size.  perhaps
    #       it can work by packing any FormatUser or CInt, CFloat, etc. member vars in
    #       an ordered fashion
    lights = [
      [ [ 0.0, 8.0, -5.0 ], [ 0.6, 0.1, 0.1 ], 1.0 ],
      [ [ 5.0, 8.0,  0.0 ], [ 0.1, 0.6, 0.1 ], 1.0 ],
      [ [ 5.0, 2.0, -5.0 ], [ 0.2, 0.2, 0.2 ], 1.0 ]
    ]
    
    light_buffer = @context.createBuffer( BUFFER_INPUT )
    light_buffer.setFormat( FORMAT_USER )
    light_buffer.setElementSize( 7*4 )
    light_buffer.setSize1D( 3 )
    t = light_buffer.map
    t.set( lights.flatten.pack( "ffffffi"*lights.size ) )
    light_buffer.unmap
    @context.declareVariable( "lights" ).setObject( light_buffer )

    createGeometry
 
  end

  def createGeometry
    ptx = "#{$dir}/ptx"
  
    # Sphere
    sphere = @context.createGeometry
    sphere.setPrimitiveCount( 1 )
    sphere.setBoundingBoxProgram( @context.createProgramFromPTXFile( "#{ptx}/sphere.ptx", "bounds" ) )
    sphere.setIntersectionProgram( @context.createProgramFromPTXFile( "#{ptx}/sphere.ptx", "robust_intersect" ) )
    var = sphere.declareVariable( "sphere" )
    var.set4f( 0.0, 1.2, 0.0, 1.0 )

    # Floor
    parallelogram = @context.createGeometry()
    parallelogram.setPrimitiveCount( 1 )
    parallelogram.setBoundingBoxProgram( @context.createProgramFromPTXFile( "#{ptx}/parallelogram.ptx", "bounds" ) )
    parallelogram.setIntersectionProgram( @context.createProgramFromPTXFile( "#{ptx}/parallelogram.ptx","intersect" ) )
    anchor = [ -20.0, 0.01, 20.0 ]
    v1     = [ 40.0, 0.0, 0.0 ]
    v2     = [ 0.0, 0.0, -40.0 ]
    normal = cross( v1, v2 )
    normal = normalize( normal )
    d      = dot( normal, anchor )
    l2_inv = 1.0 / dot( v1,v1 )
    v1     = v1.map { |x| x * l2_inv }
    l2_inv = 1.0 / dot( v2,v2 )
    v2     = v2.map { |x| x * l2_inv }
    plane  = normal + [d]
    var = parallelogram.declareVariable( "plane" )
    var.set4f( *plane )
    var = parallelogram.declareVariable( "v1" )
    var.set3f( *v1 )
    var = parallelogram.declareVariable( "v2" )
    var.set3f( *v2 )
    var = parallelogram.declareVariable( "anchor" )
    var.set3f( *anchor )

    # Phong programs
    phong_ch = @context.createProgramFromPTXFile( "#{ptx}/phong.ptx", "closest_hit_radiance" )
    phong_ah = @context.createProgramFromPTXFile( "#{ptx}/phong.ptx", "any_hit_shadow" )

    # Sphere material
    sphere_matl = @context.createMaterial()
    sphere_matl.setClosestHitProgram( 0, phong_ch )
    sphere_matl.setAnyHitProgram( 1, phong_ah )

    var = sphere_matl.declareVariable( "Ka" )
    var.set3f( 0.0,0.0,0.0)
    var = sphere_matl.declareVariable( "Kd" )
    var.set3f( 0.6, 0.0, 0.0)
    var = sphere_matl.declareVariable( "Ks" )
    var.set3f( 0.6, 0.6, 0.6)
    var = sphere_matl.declareVariable( "phong_exp" )
    var.set1f(32.0)
    var = sphere_matl.declareVariable( "reflectivity" )
    var.set3f(0.4, 0.4, 0.4)

    # Floor material
    check_ch   = @context.createProgramFromPTXFile( "#{ptx}/checker.ptx", "closest_hit_radiance" )
    check_ah   = @context.createProgramFromPTXFile( "#{ptx}/checker.ptx", "any_hit_shadow" )
    floor_matl = @context.createMaterial()
    floor_matl.setClosestHitProgram( 0, check_ch )
    floor_matl.setAnyHitProgram( 1, check_ah )

    var = floor_matl.declareVariable( "Kd1" )
    var.set3f( 0.0, 0.0, 0.0)
    var = floor_matl.declareVariable( "Ka1" )
    var.set3f( 0.0, 0.0, 0.0)
    var = floor_matl.declareVariable( "Ks1" )
    var.set3f( 0.5, 0.5, 0.5)
    var = floor_matl.declareVariable( "reflectivity1" )
    var.set3f( 0.5, 0.5, 0.5)
    var = floor_matl.declareVariable( "phong_exp1" )
    var.set1f( 32.0 )

    var = floor_matl.declareVariable( "Kd2" )
    var.set3f( 0.6, 0.6, 0.6)
    var = floor_matl.declareVariable( "Ka2" )
    var.set3f( 0.0, 0.0, 0.0)
    var = floor_matl.declareVariable( "Ks2" )
    var.set3f( 0.5, 0.5, 0.5)
    var = floor_matl.declareVariable( "reflectivity2" )
    var.set3f( 0.2, 0.2, 0.2)
    var = floor_matl.declareVariable( "phong_exp2" )
    var.set1f( 32.0 )

    var = floor_matl.declareVariable( "inv_checker_size" )
    var.set3f( 40.0, 40.0, 1.0 )


    # Place geometry into hierarchy
    sphere_gi = @context.createGeometryInstance
    sphere_gi.setGeometry( sphere )
    sphere_gi.setMaterialCount( 1 )
    sphere_gi.setMaterial( 0, sphere_matl )
    
    floor_gi = @context.createGeometryInstance
    floor_gi.setGeometry( parallelogram )
    floor_gi.setMaterialCount( 1 )
    floor_gi.setMaterial( 0, floor_matl )
    
    accel = @context.createAcceleration
    accel.setBuilder( "NoAccel" )
    accel.setTraverser( "NoAccel" )
    geometrygroup = @context.createGeometryGroup()
    geometrygroup.setChildCount( 2 )
    geometrygroup.setChild( 0, sphere_gi )
    geometrygroup.setChild( 1, floor_gi )

    geometrygroup.setAcceleration( accel )

    var = @context.declareVariable( "top_object" )
    var.setObject( geometrygroup )
    var = @context.declareVariable( "top_shadower" )
    var.setObject( geometrygroup )

  end

  def launch 
    @context.launch( 0, @width, @height )
  end
end

MantaScene.new.run
