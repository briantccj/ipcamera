https://whycan.cn/t_1760.html

3.运行mediaserver
~/live/mediaServer$ ./live555MediaServer

4. 建立fifo
~/live/mediaServer$ mkfifo test.m4e
~/live/mediaServer$ chmod u+rw test.m4e

5.设计软件，采集video数据，编码成xvid，写入fifo
/home/avr32/live/mediaServer/test.m4e
编译命令:
gcc -o testx ubuntu_encode_usb_camera.c -lxvidcore
运行testx
#testx

7. vlc打开网址:
vlc rtsp://127.0.0.1:8554/test.m4e