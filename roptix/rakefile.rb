
require 'fileutils'

task :default => [:extension]

task :extension => [ 'libc/extconf.rb', 'libc/optix_wrap.c' ] do
    FileUtils.cd 'libc/'
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

    # You can set optix_dir on rake commandline:  $ rake optix_dir=/usr/local/optix
    if( ENV.include? "optix_dir" )
      sh "#{archflags} ruby extconf.rb --with-optix-dir=#{ENV["optix_dir"]}"
    else
      sh "#{archflags} ruby extconf.rb --with-optix-dir=#{default_optix_dir}"
    end
    sh 'make'
    FileUtils.mv "roptix#{lib_ext}", '../lib/'
end


