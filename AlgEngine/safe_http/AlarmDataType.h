#ifndef __ALARMDATATYPE_H__
#define __ALARMDATATYPE_H__

#include <vector>
#include <string>

struct ObjectRect
{
    // 报警目标在全景图中的相对位置:
    int objLeft;
    int objTop;
    int objRight;
    int objBottom;
    int absoluteObjLeft;
    int absoluteObjTop;
    int absoluteObjRight;
    int absoluteObjBottom;
};
struct ImageInfo
{
    // 图片类型,参见 附录15   0：原始图
    int imgType;
    // 报警抓图分辨率宽,单位:像素
    int imgWidth;
    // 报警抓图分辨率高,单位:像素
    int imgHeight;
    // 图片抓拍时间, utc时间,单位毫秒;有可能抓拍多张图后再发报警消息,类似取证
    double capTime;
    // 目标框,可能一张报警图片存在多个目标
    std::vector<ObjectRect> objectRects;
    // 图片大小
    int imgSize;
    // 图片地址  
    /**
     * 可以有四种形式，任意一种都行
     * 1. 经过base64编码的数据文件绝对路径
     * 2. 绝对路径jpg格式的图片
     * 3. base64编码的数据指针（内存）及大小
     * 4. jpg格式的图片数据指针（内存）及大小
    */
    std::string imgUrl;
};

struct AlarmInfo
{
    // 报警类型,见 附录16
    std::string alarmType;
    // 报警动作,start-开始,stop结束,pluse-脉冲,固定填写 pluse 即可
    std::string alarmAction;
    // 报警名称,不能重复。
    std::string alarmName;
    // 报警图片列表,当报警消息中不存在图片则为空
    std::vector<ImageInfo> imgList;
    std::string classType;
    // 算子透传字段,可根据需求自定义
    std::string extData;
    // 报警信息,不同的alarmType对应的info信息不同
    // std::string info;?
};

#endif /* __ALARMDATATYPE_H__ */