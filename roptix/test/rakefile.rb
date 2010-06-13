
require 'pathname'
require 'rubygems'
require 'mkrf'


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


# map of 'result ptx_file' => 'source cuda_file'
ptx_files = Hash.new
Dir.glob( cuda_dir + '**.cu' ).each do |cuda_file|
  ptx_files[ ptx_dir + ( File.basename( cuda_file, '.cu' ) + ".ptx") ] = cuda_file
end

# create file rule for each ptx file, dependent upon corresponding cuda src file
cuda_headers = Dir.glob( cuda_dir + '**.h' )
ptx_files.each do |ptx_file, cuda_file| 
  file ptx_file => cuda_headers + [ cuda_file ] do
    sh "nvcc -m32 --ptx #{cuda_file} -o #{ptx_file} " +
       "#{avail.includes_compile_string} " +
       "#{avail.library_compile_string}"
  end
end


task :default => [ :cuda ]

task :cuda => ptx_files.keys

