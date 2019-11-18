//https://www.cnblogs.com/surpassal/archive/2012/12/19/zed_webcam_lab1.html
//https://blog.csdn.net/g_salamander/article/details/8107692
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
// #include <opencv2/opencv.hpp>
#include <unistd.h>
#include <cstring>

#define FILE_VIDEO     "/dev/video0"

#define  IMAGEWIDTH    640
#define  IMAGEHEIGHT   480

#define BUFFER_COUNT    (4)
struct buffer
{
    void * start;
    unsigned int length;
} * buffers;

int v4l2_camera_opendev(char *device_name)
{
    int fd = -1;

    if((fd = open(device_name, O_RDWR)) == -1) 
    {
        printf("Error opening V4L interface\n");
    }
    return fd;
}
int v4l2_camera_init(int fd)
{
    int i;
    int ret = 0;
    struct   v4l2_capability   cap;
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_format fmt, fmtack;
    struct v4l2_streamparm setfps;  
    
    memset(&cap, 0, sizeof(cap));
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    memset(&fmt, 0, sizeof(fmt));
    memset(&fmtack, 0, sizeof(fmtack));
    memset(&setfps, 0, sizeof(setfps));
    
    //query cap 查询摄像头的信息
    if(ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) 
    {
        printf("Error opening device %s: unable to query device.\n",FILE_VIDEO);
        return (-1);
    }
    else
    {
         printf("driver:\t\t%s\n",cap.driver);
         printf("card:\t\t%s\n",cap.card);
         printf("bus_info:\t%s\n",cap.bus_info);
         printf("version:\t%d\n",cap.version);
         printf("capabilities:\t%x\n",cap.capabilities);
         
         if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) 
         {
            printf("Device %s: supports capture.\n",FILE_VIDEO);
        }

        if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) 
        {
            printf("Device %s: supports streaming.\n",FILE_VIDEO);
        }
    } 
    
    //emu all support fmt 列举摄像头所支持的像素格式
    fmtdesc.index=0;
    fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Support format:\n");
    while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)
    {
        printf("\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
        fmtdesc.index++;
    }
    
    //set fmt 设置摄像头所支持的像素格式
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.height = IMAGEHEIGHT;
    fmt.fmt.pix.width = IMAGEWIDTH;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    
    if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        printf("Unable to set format\n");
        return -1;
    }     
    //获取像素格式设置
    if(ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
    {
        printf("Unable to get format\n");
        return -1;
    } 
    {
         printf("fmt.type:\t\t%d\n",fmt.type);
         printf("pix.pixelformat:\t%c%c%c%c\n",fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,(fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
         printf("pix.height:\t\t%d\n",fmt.fmt.pix.height);
         printf("pix.width:\t\t%d\n",fmt.fmt.pix.width);
         printf("pix.field:\t\t%d\n",fmt.fmt.pix.field);
    }
    //set fps
    setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    setfps.parm.capture.timeperframe.numerator = 10;
    setfps.parm.capture.timeperframe.denominator = 10;

    return ret;
}
int v4l2_camera_mmap(int fd)
{
    struct v4l2_buffer buf;
    
    unsigned int n_buffers;
    
    struct v4l2_requestbuffers req;
    //request for 4 buffers 为摄像头申请缓冲区
    memset(&req, 0, sizeof(req));
    req.count=BUFFER_COUNT;
    req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory=V4L2_MEMORY_MMAP;
    if(ioctl(fd,VIDIOC_REQBUFS,&req)==-1)
    {
        printf("request for buffers error\n");
    }
    //mmap for buffers 获取每个缓冲区的信息，并mmap到用户空间
    buffers = (buffer*)malloc(req.count*sizeof (*buffers));
    if (!buffers) 
    {
        printf ("Out of memory\n");
        return(-1);
    }
    
    for (n_buffers = 0; n_buffers < req.count; n_buffers++) 
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
        //query buffers 请求驱动申请内核空间
        if (ioctl (fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            printf("query buffer error\n");
            return(-1);
        }
        buffers[n_buffers].length = buf.length;
        //map 将内核的内存空间映射到应用层
        buffers[n_buffers].start = mmap(NULL,buf.length,PROT_READ |PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[n_buffers].start == MAP_FAILED)
        {
            printf("buffer map error\n");
            return(-1);
        }
    }
     //queue 
    for (n_buffers = 0; n_buffers < req.count; n_buffers++)
    {
        memset(&buf, 0, sizeof(buf));
        buf.index = n_buffers;
        buf.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory =V4L2_MEMORY_MMAP;
        ioctl(fd, VIDIOC_QBUF, &buf);
    }
    printf("mmap ok\r\n");
    return 0; 
}
int v4l2_camera_open(char *device_name)
{
    int fd;

    fd = v4l2_camera_opendev(device_name);
    v4l2_camera_init(fd);
    v4l2_camera_mmap(fd);
        
    return fd;
}
int v4l2_camera_capture_start(int fd)
{
    enum v4l2_buf_type type;
   
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl (fd, VIDIOC_STREAMON, &type);

    return(0);
}
int v4l2_camera_data_ready_check(int fd)
{
    fd_set fds;			
    struct timeval tv;
    int ret;			
    FD_ZERO(&fds);			
    FD_SET(fd,&fds);			/*Timeout*/			
    tv.tv_sec = 2;			
    tv.tv_usec = 0;			
    ret = select(fd + 1,&fds,NULL,NULL,&tv);

    if(-1 == ret)
    {    
        if(EINTR == errno)
        return -1;
    
        perror("Fail to select");   
        exit(EXIT_FAILURE);
    }
    
    if(0 == ret)
    { 
        fprintf(stderr,"select Timeout\n");
        //exit(-1);
    }
    return 0;
}
char *v4l2_camera_get_a_framebuf(int fd, int *buflen, int *buf_handle)
{
    struct v4l2_buffer buf;
    memset((void*)&buf, 0, sizeof(buf));
    buf.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory =V4L2_MEMORY_MMAP;

    ioctl(fd, VIDIOC_DQBUF, &buf);

    printf("buf.sequence  %d\r\n", buf.sequence);

    *buflen = buf.bytesused;
    *buf_handle = buf.index;

    return (char*)buffers[buf.index].start;
}
int v4l2_camera_release_framebuf(int fd, int buf_handle)
{
    struct v4l2_buffer buf;
    memset((void*)&buf, 0, sizeof(buf));
    buf.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory =V4L2_MEMORY_MMAP;
    buf.index = buf_handle;

    ioctl(fd, VIDIOC_QBUF, &buf);

    return 0;
}
int v4l2_camera_close(int fd)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ioctl(fd, VIDIOC_STREAMOFF, &type);     
    if(fd != -1) 
    {
        close(fd);
        printf("closed\r\n");
    }
    for (int i = 0; i < BUFFER_COUNT; i++) 
    {
        munmap(buffers[i].start, buffers[i].length);
    }
    free(buffers);

    return 0;
}

int stop;
void Stop(int signo) 
{
    v4l2_camera_close(stop);
    _exit(0);
}

int main(void)
{
    int fd;
    int buflen;
    int buf_handle;
    char *frambuf;

    signal(SIGINT, Stop);

    fd = v4l2_camera_open(FILE_VIDEO);
    stop = fd;
    v4l2_camera_capture_start(fd);

    while(1)
    {
        if(v4l2_camera_data_ready_check(fd) == 0)
        {
            frambuf = v4l2_camera_get_a_framebuf(fd, &buflen, &buf_handle);

            // frambuf

            v4l2_camera_release_framebuf(fd, buf_handle);
        }
    }
    v4l2_camera_close(fd);

    return(0);
}