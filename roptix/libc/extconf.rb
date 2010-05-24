require 'mkmf'

dir_config( 'optix', '/usr/local/optix/include', '/usr/local/optix/lib64' )

exit(1) if !have_header( "optix.h" )
exit(1) if !have_library( "optix", "rtContextCreate" )

create_makefile( "roptix" )
