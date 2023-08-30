cmake_minimum_required(VERSION 3.5)

project(polyglot LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED)

add_executable(polyglot-cpp
    src/main.cpp

    src/core/PolyglotAST.cpp
    src/core/PolyglotAST.h
    src/core/WrapperWriter.cpp
    src/core/WrapperWriter.h

    src/cppscanner/DGen.cpp
    src/cppscanner/DGen.h
    src/cppscanner/CppBindingGenerator.cpp
    src/cppscanner/CppBindingGenerator.h
    src/cppscanner/RustGen.cpp
    src/cppscanner/RustGen.h
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
