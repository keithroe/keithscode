
require 'fileutils'

task :default => [:extension]

task :extension => [ 'libc/extconf.rb', 'libc/optix_wrap.c' ] do
    FileUtils.cd 'libc/'
    puts "<<#{RUBY_PLATFORM}>>"
    if RUBY_PLATFORM =~ /win32|mingw|cygwin/
      puts "You are running windows?  Sucks for you ..."
      exit 1
    elsif RUBY_PLATFORM =~ /darwin/
      archflags         = 'ARCHFLAGS="-arch i386"'
      default_optix_dir = '/Developer/OptiX'
      lib_ext           = '.bundle'
    else  RUBY_PLATFORM =~ /linux/
      archflags         = '' 
      default_optix_dir = '/usr/local/optix'
      lib_ext           = '.so'
    end

    # $ rake optix_lib=/usr/local/optix/include
    # $ rake optix_inc=/usr/local/optix/lib64
    libdir = ( ENV.include? "optix_lib" ) ?  "--with-optix-lib=#{ENV["optix_lib"]}" :
                                             "--with-optix-lib=#{default_optix_dir}/lib64" 
    incdir = ( ENV.include? "optix_inc" ) ?  "--with-optix-include=#{ENV["optix_inc"]}" :
                                             "--with-optix-include=#{default_optix_dir}/include" 
    sh "#{archflags} ruby extconf.rb #{libdir} #{incdir}" 
    sh 'make'
    FileUtils.mv "roptix#{lib_ext}", '../lib/'
end


