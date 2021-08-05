You need Visual Studio 2019 and CMake

You need to use "x64 Native Tools Command Prompt for VS 2019" (aka vcvars64) for building libraries.
# You need to build these libraries manualy:
## SD2 version 2.0.8

`git clone --depth=1 --branch=release-2.0.8 https://github.com/libsdl-org/SDL`   

There is a bug in this version you have to fix before continue, in file CMakeLists.txt you have to change this line  
`list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid)`  
to this  
`list(APPEND EXTRA_LIBS vcruntime setupapi user32 gdi32 winmm imm32 ole32 oleaut32 version uuid)`  
then continue

`mkdir sdl2-build`   
`cd sdl2-build`   
`cmake -G "Ninja" ../SDL -DCMAKE_BUILD_TYPE=Release -DSDL_STATIC =OFF -DCMAKE_INSTALL_PREFIX="<SDL2 installation path>"`  
`cmake --build .`  
`cmake --install .`  

`<SDL2 installation path>` for example E:/sdl2-install

## json-c version 0.15

`git clone --depth=1 --branch=json-c-0.15-20200726 https://github.com/json-c/json-c`   
`mkdir json-c-build`   
`cd json-c-build`   
`cmake -G "Ninja" ../json-c -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX="<json-c installation path>"`  
`cmake --build .`  
`cmake --install .`  

`<json-c installation path>` for example E:/json-c-install

# These are libraries that you can use prebuilt binaries: 
## SDL_image 2.0.5
https://www.libsdl.org/projects/SDL_image/ direct (https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-VC.zip)  
extract content to a install folder than we can use later `<SDL_image installation path>`  
## SDL_mixer 2.0.4
https://www.libsdl.org/projects/SDL_mixer/ direct (https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.0.4-VC.zip)  
extract content to a install folder than we can use later `<SDL_image installation path>`  
## Lua 5.2
https://sourceforge.net/projects/luabinaries/files/5.2.4/Windows%20Libraries/Dynamic/ file name (lua-5.2.4_Win64_dll16_lib.zip)  
extract content to a install folder than we can use later `<lua installation path>`  

# Build uzu2
Last step is to provide variables for your cmake project (using Visual Studio of of course)  
Edit your `CMakeSettings.json` file (in uzu2 project)  
Example:  
`
{  
  "name": "SDL2_DIR",  
  "value": "<SDL2 installation path>",  
  "type": "PATH"  
}  
`
| VARIABLE               | VALUE                                            |
|------------------------|--------------------------------------------------|
| SDL2_DIR               | <SDL2 installation path>                         |
| SDL2_IMAGE_INCLUDE_DIR | <SDL_image installation path>/include            |
| SDL2_IMAGE_LIBRARY     | <SDL_image installation path>/x64/SDL2_image.lib |
| SDL2_MIXER_INCLUDE_DIR | <SDL_mixer installation path>/include            |
| SDL2_MIXER_LIBRARY     | <SDL_mixer installation path>/x64/SDL2_mixer.lib |
| json_c_DIR             | <json-c installation path>                       |
| Glad_DIR               | <uzu project path>/external/glad                 |
| LUA_DIR                | <lua installation path>                          |
  
Do this for both `Debug` and `Release` configurations   
  
Launch Visual Studio and `Open CMake Folder`, select target `uzu_app`