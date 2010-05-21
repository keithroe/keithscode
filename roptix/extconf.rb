require 'mkmf'

optix_library_dir = "/home/kmorley/Code/rtsdk/rtmain/debug_build/lib/"

find_header( "optix.h", "/home/kmorley/Code/rtsdk/rtmain/include/" )
find_library( "optix", "rtContextCreate", optix_library_dir )
dir_config( "optix" )

create_makefile("optix", "./libc/" )
