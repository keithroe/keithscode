
# To specify non-standard optix install location set env variable OPTIX_LOCATION

require 'rubygems'
require 'mkrf/rakehelper'

task :default do
  setup_extension( 'roptix', 'roptix_internal' ) 
  rake( 'test' )
end

