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
}

echo "Building polyglot-cpp..."
echo

mkdir -p build
cmake -Bbuild .
cmake --build build -- -j $(nproc)

echo
echo "Building polybuild..."
echo

dub build

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