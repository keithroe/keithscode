require 'mkmf'
$VERBOSE = true
puts dir_config( 'optix', 'include', 'lib64' ).inspect # TODO: check for 64b vs 32b system

puts "LIBPATH.join: <<#{$LIBPATH.join( ':' )}>>"

#find_header( "optix.h", "/home/kmorley/Code/rtsdk/rtmain/include/" )
#find_library( "optix", "rtContextCreate", optix_library_dir )
puts have_header( "optix.h " )
puts have_library( "optix", "rtContextCreate" )
puts find_library( "optix", "rtContextCreate", $LIBPATH.join(':') )
puts find_library( "optix", "rtContextCreate", "/usr/local/optix/lib64/" )


create_makefile( "roptix" )
