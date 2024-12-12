path_cur=$(dirname $0)

cd $path_cur

# Set environment PATH (Please confirm that the install_path is correct).
export TE_PARALLEL_COMPILER=1
export install_path=/usr/local/Ascend/ascend-toolkit/latest
export PATH=/usr/local/python3.9.2/bin:${install_path}/atc/ccec_compiler/bin:${install_path}/atc/bin:$PATH
export PYTHONPATH=${install_path}/atc/python/site-packages:${install_path}/atc/python/site-packages/auto_tune.egg/auto_tune:${install_path}/atc/python/site-packages/schedule_search.egg
export LD_LIBRARY_PATH=${install_path}/atc/lib64:$LD_LIBRARY_PATH
export ASCEND_OPP_PATH=${install_path}/opp

soc="Ascend"
chip_version=$(npu-smi info | awk '{print $3}' | grep -m 1 310)

# Execute, transform YOLOv3 model.
cd model
atc --model=./yolov3_tf.pb --framework=3 --output=./yolov3_tf_bs1_fp16 --soc_version="$soc$chip_version" --insert_op_conf=./aipp_yolov3_416_416.aippconfig --input_shape="input:1,416,416,3" --out_nodes="yolov3/yolov3_head/Conv_6/BiasAdd:0;yolov3/yolov3_head/Conv_14/BiasAdd:0;yolov3/yolov3_head/Conv_22/BiasAdd:0"
cd ..

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
exit 0