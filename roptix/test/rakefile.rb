
require 'pathname'
require 'rubygems'
require 'mkrf'

task :default => [:cuda]

task :cuda => Dir.glob( File.join( File.dirname( __FILE__ ), 'cuda', '**.cu' ) ) do

  avail = Mkrf::Availability.new()
  avail.include_library( 'optix', 'rtContextCreate',
                         "#{ENV['OPTIX_LOCATION']}/lib64",
                         "#{ENV['OPTIX_LOCATION']}/lib",
                         '/usr/local/OptiX/lib',
                         '/usr/local/OptiX/lib64',
                         '/Developer/OptiX/lib',
                         '/Developer/OptiX/lib64' )

  avail.include_header( 'optix.h',
                        "#{ENV['OPTIX_LOCATION']}/include",
                        '/usr/local/OptiX/include',
                        '/Developer/OptiX/include' )

  ptx_dir  = Pathname.new( File.dirname( __FILE__ ) ) + 'ptx'
  cuda_dir = Pathname.new( File.dirname( __FILE__ ) ) + 'cuda'

  Dir.glob( cuda_dir + '**.cu' ).each do |cuda_file|
    ptx_file = File.basename( cuda_file, '.cu' ) + ".ptx"

    sh "nvcc -m32 --ptx #{cuda_file} -o #{ptx_dir + ptx_file} " +
       "#{avail.includes_compile_string} " +
       "#{avail.library_compile_string}"
  end
end

