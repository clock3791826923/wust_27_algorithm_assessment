#include <iostream>
#include <type_traits>
#include <algorithm> //std::max
//题目提供的三种原始数据结构
struct CameraFrame { int w, h; const unsigned char* buf; };
struct RosImage    { int w, h, step; const unsigned char* data; };
struct VideoFrame  { int w, h; const unsigned char* yuv; };
// 1. 虚函数版本 (动态多态)
namespace virt 
{
    class ImageSource 
    {
    public:
        virtual ~ImageSource()=default;   
        virtual int width() const=0;
        virtual int height() const=0;
        virtual unsigned char at(int x, int y) const=0;
    };
    // 相机帧实现
    class CameraSource : public ImageSource 
    {
    public:
        explicit CameraSource(CameraFrame f):f(f){}//explicit 避免隐式转换
        int width() const override 
        {
            return f.w; 
        }
        int height() const override 
        { 
            return f.h; 
        }
        unsigned char at(int x, int y) const override 
        { 
            return f.buf[y * f.w + x]; 
        }
    private:
        CameraFrame f;
    };
    // ROS帧实现
    class RosSource : public ImageSource 
    {
    public:
        explicit RosSource(RosImage f) : f(f) {}
        int width() const override 
        { 
            return f.w; 
        }
        int height() const override 
        { 
            return f.h; 
        }
        unsigned char at(int x, int y) const override 
        { 
            return f.data[y * f.step + x]; 
        }
    private:
        RosImage f;
    };
    // 视频帧实现
    class VideoSource : public ImageSource 
    {
    public:
        explicit VideoSource(VideoFrame f) : f(f) {}
        int width() const override 
        { 
            return f.w; 
        }
        int height() const override 
        { 
            return f.h; 
        }
        unsigned char at(int x, int y) const override 
        { 
            return std::max(f.yuv[y * f.w + x] - 16, 0); 
        }
    private:
        VideoFrame f;
    };
    // 虚函数预处理（通过基类引用调用）
    uint64_t preprocess(const ImageSource& src) 
    {
        uint64_t sum=0;
        for (int y=0; y<src.height(); y++) 
        {
            for (int x=0; x<src.width(); x++) 
            {
                sum+=src.at(x, y); //运行时查表，开销大
            }
        }
        return sum;
    }
}
// 2. 模板版本 (静态多态)
namespace tmpl 
{
    template <typename T>
    uint64_t preprocess(const T& f)
    {
        uint64_t sum=0;
        for (int y=0; y<f.h; y++) 
        {
            for (int x=0; x<f.w; x++) 
            {
                if constexpr (std::is_same_v<T, CameraFrame>) //判断类型
                {
                    sum+=f.buf[y * f.w + x];
                } 
                else if constexpr (std::is_same_v<T, RosImage>) 
                {
                    sum+=f.data[y * f.step + x];
                } 
                else if constexpr (std::is_same_v<T, VideoFrame>) 
                {
                    int val=f.yuv[y * f.w + x] - 16;
                    sum+=val>0 ? val:0;
                }
            }
        }
        return sum;
    }
}
int main() 
{
    // 造点测试数据
    int w=2,h=2;
    unsigned char data[4]={10,20,30,40};
    CameraFrame cam={w,h,data};
    RosImage ros={w,h,w+2,data};
    VideoFrame vid={w,h,data};
    // 测试虚函数版
    virt::CameraSource cs(cam);
    virt::RosSource rs(ros);
    virt::VideoSource vs(vid);
    std::cout<<"virt:"<<std::endl;
    std::cout<<"Cam: "<<virt::preprocess(cs)<<std::endl;
    std::cout<<"Ros: "<<virt::preprocess(rs)<<std::endl;
    std::cout<<"Vid: "<<virt::preprocess(vs)<<std::endl;
    // 测试模板版
    std::cout<< "tmpl:"<<std::endl;
    std::cout<<"Cam: "<<tmpl::preprocess(cam)<<std::endl;
    std::cout<<"Ros: "<<tmpl::preprocess(ros)<<std::endl;
    std::cout<<"Vid: "<<tmpl::preprocess(vid)<<std::endl;
    return 0;
}
