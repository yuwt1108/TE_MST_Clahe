cmake_minimum_required(VERSION 3.16)
project(te_mst_demo LANGUAGES CXX)

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)
add_executable(te_mst_demo main.cpp te_mst.cpp te_mst.h)
target_compile_features(te_mst_demo PRIVATE cxx_std_17)
target_compile_options(te_mst_demo PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
target_include_directories(te_mst_demo PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(te_mst_demo PRIVATE ${OpenCV_LIBS})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT te_mst_demo)
if(TARGET opencv_world)
    set_property(TARGET te_mst_demo PROPERTY VS_DEBUGGER_ENVIRONMENT
        "PATH=$<TARGET_FILE_DIR:opencv_world>;%PATH%")
endif()
