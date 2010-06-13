
# To specify non-standard optix install location set env variable OPTIX_LOCATION

require 'rubygems'
require 'mkrf/rakehelper'

setup_extension( 'roptix', 'roptix_internal' ) 

task :default => [ :roptix_internal, :test ]

task :test do
  rake( 'test' )
end
