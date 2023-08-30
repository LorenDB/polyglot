cmake_minimum_required(VERSION 3.5)

project(polyglot LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED)

add_executable(polyglot-cpp
    src/core/DWrapperWriter.cpp
    src/core/DWrapperWriter.h
    src/core/PolyglotAST.cpp
    src/core/PolyglotAST.h
    src/core/RustWrapperWriter.cpp
    src/core/RustWrapperWriter.h
    src/core/WrapperWriter.cpp
    src/core/WrapperWriter.h

    src/cppscanner/main.cpp
    src/cppscanner/CppParser.cpp
    src/cppscanner/CppParser.h
    src/cppscanner/Utils.cpp
    src/cppscanner/Utils.h
)

target_link_libraries(polyglot-cpp PRIVATE
    clang-cpp
    LLVM
)

include(GNUInstallDirs)
install(TARGETS polyglot-cpp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
