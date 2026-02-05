cmake_minimum_required(VERSION 3.27)
project(PhasmoTimer LANGUAGES CXX)

# -----------------------------
# Default build type & C++ standard
# -----------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
    message(STATUS "Build type not specified, using Release")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# Sources
# -----------------------------
set(IMGUI_SRC
    include/imgui/imgui.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_tables.cpp
    include/imgui/imgui_widgets.cpp
    include/imgui/imgui_demo.cpp
    include/imgui/imgui_impl_dx11.cpp
    include/imgui/imgui_impl_win32.cpp
)

set(APP_SRC
    src/config.cpp src/fonts.cpp src/foregroundtracker.cpp src/gui.cpp
    src/hud_bar.cpp src/imgui_wrappers.cpp src/input.cpp src/main.cpp
    src/render.cpp src/stamina.cpp src/timer.cpp src/tools.cpp src/updater.cpp
)

set(RESOURCES PhasmoTimer.rc)

add_executable(PhasmoTimer WIN32 ${IMGUI_SRC} ${APP_SRC} ${RESOURCES})
target_include_directories(PhasmoTimer PRIVATE include src)

# -----------------------------
# Libraries
# -----------------------------
target_link_libraries(PhasmoTimer PRIVATE
    d3d11 d3dcompiler dxgi dcomp
    Ws2_32 Crypt32 Iphlpapi secur32
)
find_package(CURL CONFIG REQUIRED)
target_link_libraries(PhasmoTimer PRIVATE CURL::libcurl)

# -----------------------------
# Preprocessor definitions
# -----------------------------
target_compile_definitions(PhasmoTimer PRIVATE
    UNICODE _UNICODE
    $<$<CONFIG:Release>:NDEBUG NOMINMAX>
    $<$<CONFIG:Debug>:_DEBUG>
)

# -----------------------------
# Compiler & linker flags
# -----------------------------
if(MSVC)
    target_compile_options(PhasmoTimer PRIVATE /utf-8 /W3)
    target_compile_options(PhasmoTimer PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(PhasmoTimer PRIVATE
            /clang:-Wno-unused-command-line-argument
            $<$<CONFIG:Release>:/clang:-fno-math-errno
                                  /clang:-fvectorize
                                  /clang:-fslp-vectorize
                                  /clang:-finline-functions
                                  /clang:-fmerge-all-constants
                                  /clang:-flto=full
                                  /clang:-fno-exceptions
                                  /clang:-fno-rtti>
        )
        target_link_options(PhasmoTimer PRIVATE
            $<$<CONFIG:Release>:/OPT:REF /OPT:ICF=5 /OPT:LBR
                                  /MERGE:.rdata=.text /INCREMENTAL:NO>
        )
    else()
        target_compile_options(PhasmoTimer PRIVATE
            $<$<CONFIG:Release>:/Gy /Oi /Ob3 /Ot /Oy /GF /GL>
        )
        target_link_options(PhasmoTimer PRIVATE
            $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF /NODEFAULTLIB:LIBCMT>
        )
    endif()
endif()

# Enable LTO for Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# -----------------------------
# Output
# -----------------------------
set_target_properties(PhasmoTimer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "PhasmoTimer"
    LINK_FLAGS_RELEASE ""
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/PhasmoTimer.manifest")
    target_sources(PhasmoTimer PRIVATE PhasmoTimer.manifest)
endif()

if(MSVC)
    target_link_options(PhasmoTimer PRIVATE /MANIFESTUAC:level='asInvoker')
endif()

# -----------------------------
# Build info
# -----------------------------
message(STATUS "==========================================")
message(STATUS "PhasmoTimer Build Configuration")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Output: ${CMAKE_BINARY_DIR}/bin/PhasmoTimer.exe")
message(STATUS "==========================================")