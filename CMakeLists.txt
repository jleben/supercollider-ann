cmake_minimum_required (VERSION 2.8)
project (Ann)

set( SC_PATH "/home/jakob/programming/supercollider"
  CACHE PATH "The SuperCollider source directory."
)
set( FANN_PATH "/home/jakob/programming/fann-2.1.0"
  CACHE PATH "The Fann source directory."
)
set( SC_EXT_INSTALL_PATH "~/.local/share/SuperCollider/Extensions/"
  CACHE PATH "The SuperCollider 'Extensions' directory where subdirectory with Ann extensions will be installed."
)

add_subdirectory( lang_class )
add_subdirectory( ugen )
