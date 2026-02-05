cmake_minimum_required(VERSION 3.27)
project(PhasmoTimer LANGUAGES CXX)

# ------------------------------------------------
# Build options
# ------------------------------------------------
option(ENABLE_AVX2 "Enable AVX2 optimizations" ON)

# ------------------------------------------------
# Build type & C++ standard
# ------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------
# Sources
# ------------------------------------------------
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

# ==============================================
# Enable FetchContent module
# ==============================================
include(FetchContent)

# ==============================================
# Fetch zlib library
# ==============================================
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
)
FetchContent_MakeAvailable(zlib)

# ==============================================
# Fetch curl library
# ==============================================
FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_18_0
)

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(CURL_STATICLIB ON CACHE BOOL "" FORCE)
set(CURL_USE_ZLIB ON CACHE BOOL "" FORCE)
set(ZLIB_LIBRARY ${zlib_BINARY_DIR}/zlibstatic.lib CACHE FILEPATH "" FORCE)
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE PATH "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(HTTP_ONLY ON CACHE BOOL "" FORCE)
set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
set(CURL_BROTLI OFF CACHE BOOL "" FORCE)
set(CURL_ZSTD OFF CACHE BOOL "" FORCE)
set(USE_LIBIDN2 OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBSSH2 OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "" FORCE)
set(ENABLE_CURL_MANUAL OFF CACHE BOOL "" FORCE)
set(USE_NGHTTP2 OFF CACHE BOOL "" FORCE)
set(USE_NGTCP2 OFF CACHE BOOL "" FORCE)
set(USE_QUICHE OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(curl)

# ------------------------------------------------
# Libraries
# ------------------------------------------------
target_link_libraries(PhasmoTimer PRIVATE
    d3d11 d3dcompiler dxgi dcomp
    Ws2_32 Crypt32 Iphlpapi secur32
    CURL::libcurl
)

# ------------------------------------------------
# Preprocessor definitions
# ------------------------------------------------
target_compile_definitions(PhasmoTimer PRIVATE
    UNICODE _UNICODE
    NOMINMAX
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Debug>:_DEBUG>
)

# ------------------------------------------------
# AVX2 support
# ------------------------------------------------
if(ENABLE_AVX2)
    message(STATUS "AVX2 optimizations: ENABLED")
    target_compile_definitions(PhasmoTimer PRIVATE ENABLE_AVX2)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(PhasmoTimer PRIVATE /clang:-mavx2 /clang:-mfma)
    elseif(MSVC)
        target_compile_options(PhasmoTimer PRIVATE /arch:AVX2)
    endif()
else()
    message(STATUS "AVX2 optimizations: DISABLED")
endif()

# ------------------------------------------------
# LLVM / Clang
# ------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(/Gw)
    target_compile_options(PhasmoTimer PRIVATE
        /O2
        /Gw
        /clang:-O3
        /clang:-flto=thin
        /clang:-fdata-sections
        /clang:-ffunction-sections
        /clang:-fmerge-all-constants
        /clang:-fomit-frame-pointer
        /clang:-fno-exceptions
        /clang:-fno-rtti
        /clang:-fno-math-errno
        /clang:-fno-trapping-math
        /clang:-fvisibility=hidden
        /clang:-fvisibility-inlines-hidden
    )
    target_link_options(PhasmoTimer PRIVATE
        /INCREMENTAL:NO
        /OPT:REF
        /OPT:ICF=10
    )
endif()

# ------------------------------------------------
# Output
# ------------------------------------------------
set_target_properties(PhasmoTimer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "PhasmoTimer"
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/PhasmoTimer.manifest")
    target_sources(PhasmoTimer PRIVATE PhasmoTimer.manifest)
endif()

target_link_options(PhasmoTimer PRIVATE /MANIFESTUAC:level='asInvoker')

# ------------------------------------------------
# Build info
# ------------------------------------------------
message(STATUS "==========================================")
message(STATUS "PhasmoTimer LLVM Build")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Using ThinLTO + lld-link")
message(STATUS "==========================================")