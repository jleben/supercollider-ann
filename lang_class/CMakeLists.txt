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

add_library( ann_lang MODULE ann.cpp )

target_link_libraries( ann_lang -lfloatfann )

install( TARGETS ann_lang DESTINATION "lib/SuperCollider/lang_plugins" )
install( FILES ann_lang.sc DESTINATION "~/share/SuperCollider/Extensions/Ann" )
