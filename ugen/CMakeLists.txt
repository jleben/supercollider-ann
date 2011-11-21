# setup environment

include_directories(
  ${SC_PATH}/include/plugin_interface
  ${SC_PATH}/include/common
)

include_directories(
  ${SC_PATH}/external_libraries/libsndfile/
  ${SC_PATH}/external_libraries/nova-tt
  ${SC_PATH}/external_libraries/nova-simd
  ${SC_PATH}/external_libraries/boost/
  ${SC_PATH}/external_libraries/boost_lockfree/
  #${SC_PATH}/external_libraries/boost/boost
  #${SC_PATH}/external_libraries/boost_endian/boost
  #${SC_PATH}/external_libraries/boost_move/boost
)
add_definitions("-DNOVA_SIMD")

include_directories(${FANN_PATH}/src/include)

link_directories(${FANN_PATH}/cmake-build/)

# define sources

set( src
    fann_rt.cpp
    plugin.cpp
    pool.cpp
    ann.cpp
)

# define targets

set(CMAKE_SHARED_MODULE_PREFIX "")

if(APPLE OR WIN32)
    set(CMAKE_SHARED_MODULE_SUFFIX ".scx")
endif()

add_library(ann_ugen MODULE ${src} )
target_link_libraries( ann_ugen -lfloatfann -lm )
#set_property( TARGET ann_ugen OUTPUT_NAME ann )

add_library(ann_ugen_supernova MODULE ${src})
set_property(TARGET ann_ugen_supernova APPEND PROPERTY COMPILE_DEFINITIONS  "SUPERNOVA")
target_link_libraries( ann_ugen_supernova -lfloatfann -lm )
#set_property( TARGET ann_ugen_supernova OUTPUT_NAME ann_supernova )

# install targets

install( TARGETS ann_ugen DESTINATION "lib/SuperCollider/plugins" )
install( TARGETS ann_ugen_supernova DESTINATION "lib/supernova/plugins" )
install( FILES ann_ugen.sc DESTINATION "${SC_EXT_INSTALL_PATH}/Ann" )

install( CODE "execute_process(COMMAND chown -R jakob ${SC_EXT_INSTALL_PATH}/Ann)" )
install( CODE "execute_process(COMMAND chgrp -R jakob ${SC_EXT_INSTALL_PATH}/Ann)" )
#install( SCRIPT ../permissions.cmake )
