set( SC_HEADERS_DIR "${SC_PATH}/include" )

# environment

include_directories(
  "${SC_HEADERS_DIR}/common"
  "${SC_HEADERS_DIR}/lang"
  "${SC_HEADERS_DIR}/lang_plugin_interface"
  "${FANN_PATH}/src/include"
)

link_directories(${FANN_PATH}/cmake-build/)

set(CMAKE_SHARED_MODULE_PREFIX "")

# target

set( sc_files
    ann_lang.sc
    ann_mapper.sc
    node.sc
)

add_library( ann_lang MODULE ann.cpp )

target_link_libraries( ann_lang -lfloatfann )

install( TARGETS ann_lang DESTINATION "lib/SuperCollider/lang_plugins" )
install( FILES ${sc_files} DESTINATION "${SC_EXT_INSTALL_PATH}/Ann" )

install( CODE "execute_process(COMMAND chown -R jakob ${SC_EXT_INSTALL_PATH}/Ann)" )
install( CODE "execute_process(COMMAND chgrp -R jakob ${SC_EXT_INSTALL_PATH}/Ann)" )
#install( SCRIPT ../permissions.cmake )
