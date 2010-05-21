
require 'fileutils'

task :default => [:extension]

task :extension => [ 'libc/extconf.rb', 'libc/optix_wrap.c' ] do
    FileUtils.cd 'libc/'
    sh 'ruby extconf.rb'
    sh 'make'
    FileUtils.mv 'roptix.so', '../lib/'
end


