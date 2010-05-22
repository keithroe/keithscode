require 'mkmf'

puts dir_config( 'optix', 'include', 'lib64' ).inspect # TODO: check for 64b vs 32b system

exit(1) if !have_header( "optix.h" )
exit(1) if !have_library( "optix", "rtContextCreate" )


create_makefile( "roptix" )
