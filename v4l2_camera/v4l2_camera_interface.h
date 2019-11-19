int v4l2_camera_open(char *device_name);
int v4l2_camera_capture_start(int fd);
int v4l2_camera_data_ready_check(int fd);
char *v4l2_camera_get_a_framebuf(int fd, int *buflen, int *buf_handle);
int v4l2_camera_release_framebuf(int fd, int buf_handle);
int v4l2_camera_close(int fd);