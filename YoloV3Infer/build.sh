rm -rf build
mkdir -p build
cd build

# Compile
cmake ..
make -j || {
    ret=$?
    echo "Failed to build"
    exit ${ret}
}

# Infer
cd ..
./mxbaseV2_sample test.jpg # test.jpg could be changed!