#!/bin/bash
rm -fr build
mkdir -p build
cd build

cmake ..
make -j || {
    ret=$?
    echo "Failed to build."
    exit ${ret}
}

func() {
    echo "Usage:"
    echo "bash run.sh -m model_path -c model_config_path -l model_label_path -i image_path -t post_Type [-y]"
    echo "Description:"
    echo "-m model path"
    echo "-c model config file path"
    echo "-l label path for model"
    echo "-i image path to infer"
    echo "-t post type of model"
    echo "-y [Optional] use yuv model, default is not yuv"
    exit -1
}

is_yuv=0
argc=6
args=0
while getopts "i:m:c:l:y:t:h" arg
do
    if [ "$args" -gt "$argc" ]; then
      echo "Error: Wrong usage, too many arguments."
      func
      exit 1
    fi
    case "$arg" in
        i)
        img_path="$OPTARG"
        ;;
        m)
        model_path="$OPTARG"
        ;;
        c)
        model_config_path="$OPTARG"
        ;;
        l)
        model_label_path="$OPTARG"
        ;;
        y)
        is_yuv=1
        ;;
        t)
        post_type="$OPTARG"
        ;;
        h)
        func
        exit 1
        ;;
        ?)
        echo "Error: Wrong usage, unknown argument."
        func
        exit 1
        ;;
    esac
    args=$(($args+1))
done

if [ ! -n "$model_path" ]; then
    echo "Error: Required argument \"-m model_path\" is missing." 
    func
    exit 1
fi
if [ ! -n "$model_config_path" ]; then
    echo "Error: Required argument \"-c model_config_path\" is missing." 
    func
    exit 1
fi
if [ ! -n "$model_label_path" ]; then
    echo "Error: Required argument \"-l model_label_path\" is missing." 
    func
    exit 1
fi
if [ ! -n "$img_path" ]; then
    echo "Error: Required argument \"-i img_path\" is missing." 
    func
    exit 1
fi
echo $post_type
cd ..
if [ "$is_yuv" -gt 0 ]; then
    ./yoloInfer -m "$model_path" -c "$model_config_path" -l "$model_label_path" -i "$img_path" -t "$post_type" -y
else
    ./yoloInfer -m "$model_path" -c "$model_config_path" -l "$model_label_path" -i "$img_path" -t "$post_type"
fi

exit 0


# bash run.sh  -m ./model/yolov5m_bs1.om  -c model/yolov.cfg  -l ./model/coco.names  -i zidane.jpg