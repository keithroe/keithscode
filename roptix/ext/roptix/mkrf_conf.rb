
require 'rubygems'
require 'mkrf'

Mkrf::Generator.new( 'roptix_internal' ) do |g|
  g.include_library( 'optix', 'rtContextCreate',
                     "#{ENV['OPTIX_LOCATION']}/lib64",
                     "#{ENV['OPTIX_LOCATION']}/lib",
                     '/usr/local/OptiX/lib',
                     '/usr/local/OptiX/lib64',
                     '/Developer/OptiX/lib',
                     '/Developer/OptiX/lib64' )

  g.include_header( 'optix.h',
                     "#{ENV['OPTIX_LOCATION']}/include",
                     '/usr/local/OptiX/include',
                     '/Developer/OptiX/include' )

  if( !g.has_function?( 'rtContextCreate' ) )
    g.abort!( "Failed to load optix header or library" )
  end
end
