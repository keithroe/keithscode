
require 'fileutils'

task :default => [:extension]

task :extension => [ 'libc/extconf.rb', 'libc/optix_wrap.c' ] do
    FileUtils.cd 'libc/'

    # You can set optix_dir on rake commandline:  $ rake optix_dir=/usr/local/optix
    if( ENV.include? optix_dir )
      sh "ruby extconf.rb --with-optix-dir=#{ENV[optix_dir]}"
    else
      sh "ruby extconf.rb --with-optix-dir=foo"
    end
    sh 'make'
    FileUtils.mv 'roptix.so', '../lib/'
end


