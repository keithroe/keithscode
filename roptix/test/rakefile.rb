
require 'pathname'

task :default => [:cuda]

task :cuda => Dir.glob( File.join( File.dirname( __FILE__ ), 'cuda', '**.cu' ) ) do
  puts "hello"
  test_dir = Pathname.new( File.dirname( __FILE__ ) )
  ptx_dir  = Pathname.new( File.dirname( __FILE__ ) ) + 'ptx'
  Dir.glob( File.join( File.dirname( __FILE__ ), 'cuda', '**.cu' ) ).each do |cuda_file|
    ptx_file = File.basename( cuda_file, '.cu' ) + ".ptx"
    sh "nvcc -m32 --ptx #{cuda_file} -o #{ptx_dir + ptx_file} " +
       "-I#{test_dir+'cuda'} -I#{ENV['optix_inc']} -L#{ENV['optix_lib']}" 
  end
end

