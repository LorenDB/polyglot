#!/bin/bash

install_binaries()
{
    polyglot_install_dir="$HOME/.local/bin"
    read -r -p "Where do you want to install polyglot? [$polyglot_install_dir] " response
    case "$response" in
        "")
            ;;
        *)
            polyglot_install_dir=$response
            ;;
    esac

    cp build/polybuild "${polyglot_install_dir}/polybuild"
    cp build/polyglot-cpp "${polyglot_install_dir}/polyglot-cpp"
    cp build/polyglot-d "${polyglot_install_dir}/polyglot-d"
}

original_root=$(pwd)
fail_build_script()
{
    echo $1
    cd $original_root
    exit
}

echo "Building polyglot-cpp..."
echo

mkdir -p build
cmake -Bbuild . || fail_build_script "Failed to run CMake"
cmake --build build -- -j $(nproc) || fail_build_script "Failed to build polyglot-cpp"

echo
echo "Building polyglot-d..."
echo

cd polyglot-d
dub build || fail_build_script "Failed to build polyglot-d"
cd ..

echo
echo "Building polybuild..."
echo

cd polybuild
dub build || fail_build_script "Failed to build polybuild"
cd ..

echo
read -r -p "Would you like to install polyglot now? [y/N] " response
case "$response" in
    [yY][eE][sS]|[yY])
        install_binaries
        ;;
    *)
        echo "Before using polyglot, you must make sure that at least the language scanners"
        echo "(e.g. polyglot-cpp) are in your path. Otherwise polybuild will not be able to call them."
        ;;
esac