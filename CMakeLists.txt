cmake_minimum_required(VERSION 3.5)

project(polyglot LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED)

add_executable(polyglot-cpp
    core/CppWrapperWriter.cpp
    core/CppWrapperWriter.h
    core/CppTypeProxyWriter.cpp
    core/CppTypeProxyWriter.h
    core/DWrapperWriter.cpp
    core/DWrapperWriter.h
    core/PolyglotAST.cpp
    core/PolyglotAST.h
    core/RustWrapperWriter.cpp
    core/RustWrapperWriter.h
    core/TypeProxyWriter.h
    core/Utils.cpp
    core/Utils.h
    core/ZigWrapperWriter.cpp
    core/ZigWrapperWriter.h
    core/WrapperWriter.cpp
    core/WrapperWriter.h

    polyglot-cpp/main.cpp
    polyglot-cpp/CppParser.cpp
    polyglot-cpp/CppParser.h
    polyglot-cpp/CppUtils.cpp
    polyglot-cpp/CppUtils.h
)
target_include_directories(polyglot-cpp PRIVATE core polyglot-cpp)
target_link_libraries(polyglot-cpp PRIVATE
    clang-cpp
    LLVM
)

include(GNUInstallDirs)
install(TARGETS polyglot-cpp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
