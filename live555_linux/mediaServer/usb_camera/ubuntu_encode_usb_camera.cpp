#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "xvid.h"
#include <cstring>
#include<sys/types.h>
#include<sys/stat.h>
#include "v4l2_camera_interface.h"

#define FRAMERATE_INCR 1001
#define SMALL_EPS (1e-10)
#define VIDEO "/dev/video0"
#define INDEX 0
#define IOCTL(fd, req, addr ) \
((-1==ioctl(fd,req,addr))?(perror(#req),close(fd),exit(EXIT_FAILURE)):0)
void getnextframe(int fd, uint8_t *mp4_buffer);
static float ARG_FRAMERATE = 25.00f;
static int ARG_QUALITY = 4;
static int ARG_MAXKEYINTERVAL = 250;
static char * ARG_OUTPUTFILE= "/tmp/test3.m4e";
//static char * ARG_OUTPUTFILE= "test.m4e";


static FILE * fp = NULL;


static int done = 0;
static const int motion_presets[] = {
/* quality 0 */
0,

/* quality 1 */
XVID_ME_ADVANCEDDIAMOND16,

/* quality 2 */
XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16,

/* quality 3 */
XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 |
XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8,

/* quality 4 */
XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 |
XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 |
XVID_ME_CHROMA_PVOP | XVID_ME_CHROMA_BVOP,

/* quality 5 */
XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 |
XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 |
XVID_ME_CHROMA_PVOP | XVID_ME_CHROMA_BVOP,

/* quality 6 */
XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 | XVID_ME_EXTSEARCH16 |
XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 | XVID_ME_EXTSEARCH8 |
XVID_ME_CHROMA_PVOP | XVID_ME_CHROMA_BVOP,

};
static const int vop_presets[] = {
/* quality 0 */
0,

/* quality 1 */
0,

/* quality 2 */
XVID_VOP_HALFPEL,

/* quality 3 */
XVID_VOP_HALFPEL | XVID_VOP_INTER4V,

/* quality 4 */
XVID_VOP_HALFPEL | XVID_VOP_INTER4V,

/* quality 5 */
XVID_VOP_HALFPEL | XVID_VOP_INTER4V |
XVID_VOP_TRELLISQUANT,

/* quality 6 */
XVID_VOP_HALFPEL | XVID_VOP_INTER4V |
XVID_VOP_TRELLISQUANT | XVID_VOP_HQACPRED,

};

static int XDIM=640,YDIM=480;
// static use_assembler = 0;
static void *enc_handle = NULL;

static int enc_init(int use_assembler);
static int enc_main(unsigned char *image,
unsigned char *bitstream,
int *key,
int *stats_type,
int *stats_quant,
int *stats_length,
int stats[3]);
static int enc_stop();
//signal act method defination
static void clean(int signum)
{
    done=1;
}
FILE *create_fifo(char *name);
int 
main(int argc,char *argv[])
{
    int fd;
    fp = create_fifo("/tmp/test3.m4e");
    fd = v4l2_camera_open(VIDEO);
    v4l2_camera_capture_start(fd);
    //init xvid
    
    int result;
    result = enc_init(1);
    if(result!=0)
    {
fprintf(stderr, "Encore INIT problem, return value %d\n", result);        
    }
    
    //get mem of mpg4 frame
    
    uint8_t *mp4_buffer = NULL;    
mp4_buffer = (unsigned char *) malloc(XDIM*YDIM);
if (mp4_buffer==NULL)
    {
        fprintf(stderr,"malloc error");

    }
    int outbytes,m4v_size;
    char *frambuf;
    int buflen;
    int buf_handle;


int key;
int stats_type;
int stats_quant;
int stats_length;
int sse[3];

    while(!done)
    {
        if(v4l2_camera_data_ready_check(fd) == 0)
        {
            frambuf = v4l2_camera_get_a_framebuf(fd, &buflen, &buf_handle);
        
            m4v_size = enc_main((uint8_t *)frambuf, mp4_buffer+16, &key, &stats_type,&stats_quant, &stats_length, sse);

            // outbytes = fwrite(mp4_buffer, 1, m4v_size, fp);
            fwrite(mp4_buffer, 1, m4v_size, fp);
            // if(outbytes != m4v_size)
            // {
            //     fprintf(stderr,"Error writing the m4u file\n");
            //     exit(7);
            // }
            v4l2_camera_release_framebuf(fd, buf_handle);
        }
    }
return 0;
}
int
enc_init(int use_assembler)
{
int xerr;
//xvid_plugin_cbr_t cbr;
    xvid_plugin_single_t single;
xvid_plugin_2pass1_t rc2pass1;
xvid_plugin_2pass2_t rc2pass2;
//xvid_plugin_fixed_t rcfixed;
xvid_enc_plugin_t plugins[7];
xvid_gbl_init_t xvid_gbl_init;
xvid_enc_create_t xvid_enc_create;

/*------------------------------------------------------------------------
* XviD core initialization
*----------------------------------------------------------------------*/

/* Set version -- version checking will done by xvidcore */
memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init));
xvid_gbl_init.version = XVID_VERSION;
    xvid_gbl_init.debug = 0;


/* Do we have to enable ASM optimizations ? */
if (use_assembler) {
xvid_gbl_init.cpu_flags = 0;
}

/* Initialize XviD core -- Should be done once per __process__ */
xvid_global(NULL, XVID_GBL_INIT, &xvid_gbl_init, NULL);

/*------------------------------------------------------------------------
* XviD encoder initialization
*----------------------------------------------------------------------*/

/* Version again */
memset(&xvid_enc_create, 0, sizeof(xvid_enc_create));
xvid_enc_create.version = XVID_VERSION;

/* Width and Height of input frames */
xvid_enc_create.width = XDIM;
xvid_enc_create.height = YDIM;
xvid_enc_create.profile = XVID_PROFILE_S_L3;

/* init plugins  */
    xvid_enc_create.zones = NULL;
    xvid_enc_create.num_zones = 0;

xvid_enc_create.plugins = NULL;
xvid_enc_create.num_plugins = 0;

/* No fancy thread tests */
xvid_enc_create.num_threads = 0;

/* Frame rate - Do some quick float fps = fincr/fbase hack */
if ((ARG_FRAMERATE - (int) ARG_FRAMERATE) < SMALL_EPS) {
xvid_enc_create.fincr = 1;
xvid_enc_create.fbase = (int) ARG_FRAMERATE;
} else {
xvid_enc_create.fincr = FRAMERATE_INCR;
xvid_enc_create.fbase = (int) (FRAMERATE_INCR * ARG_FRAMERATE);
}

/* Maximum key frame interval */
    if (ARG_MAXKEYINTERVAL > 0) {
        xvid_enc_create.max_key_interval = ARG_MAXKEYINTERVAL;
    }else {
    xvid_enc_create.max_key_interval = (int) ARG_FRAMERATE *10;
    }

/* Bframes settings */
xvid_enc_create.max_bframes = 0;
xvid_enc_create.bquant_ratio = 150;
xvid_enc_create.bquant_offset = 100;

/* Dropping ratio frame -- we don't need that */
xvid_enc_create.frame_drop_ratio = 0;

/* Global encoder options */
xvid_enc_create.global = 0;

/* I use a small value here, since will not encode whole movies, but short clips */
xerr = xvid_encore(NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);

/* Retrieve the encoder instance from the structure */
enc_handle = xvid_enc_create.handle;

return (xerr);
}

int
enc_main(unsigned char *image,
unsigned char *bitstream,
int *key,
int *stats_type,
int *stats_quant,
int *stats_length,
int sse[3])
{
int ret;

xvid_enc_frame_t xvid_enc_frame;
xvid_enc_stats_t xvid_enc_stats;

/* Version for the frame and the stats */
memset(&xvid_enc_frame, 0, sizeof(xvid_enc_frame));
xvid_enc_frame.version = XVID_VERSION;

memset(&xvid_enc_stats, 0, sizeof(xvid_enc_stats));
xvid_enc_stats.version = XVID_VERSION;

/* Bind output buffer */
xvid_enc_frame.bitstream = bitstream;
xvid_enc_frame.length = -1;

/* Initialize input image fields */
if (image) {
xvid_enc_frame.input.plane[0] = image;
xvid_enc_frame.input.csp = XVID_CSP_YUY2;
xvid_enc_frame.input.stride[0] = XDIM*2;
} else {
xvid_enc_frame.input.csp = XVID_CSP_NULL;
}

/* Set up core's general features */
xvid_enc_frame.vol_flags = 0;

/* Set up core's general features */
xvid_enc_frame.vop_flags = vop_presets[ARG_QUALITY];

/* Frame type -- let core decide for us */
xvid_enc_frame.type = XVID_TYPE_AUTO;

/* Force the right quantizer -- It is internally managed by RC plugins */
xvid_enc_frame.quant = 3;

/* Set up motion estimation flags */
xvid_enc_frame.motion = motion_presets[ARG_QUALITY];

/* We don't use special matrices */
xvid_enc_frame.quant_intra_matrix = NULL;
xvid_enc_frame.quant_inter_matrix = NULL;

/* Encode the frame */
ret = xvid_encore(enc_handle, XVID_ENC_ENCODE, &xvid_enc_frame,
  &xvid_enc_stats);

*key = (xvid_enc_frame.out_flags & XVID_KEYFRAME);
*stats_type = xvid_enc_stats.type;
*stats_quant = xvid_enc_stats.quant;
*stats_length = xvid_enc_stats.length;
sse[0] = xvid_enc_stats.sse_y;
sse[1] = xvid_enc_stats.sse_u;
sse[2] = xvid_enc_stats.sse_v;
return (ret);
}

int
enc_stop()
{
int xerr;

/* Destroy the encoder instance */
xerr = xvid_encore(enc_handle, XVID_ENC_DESTROY, NULL, NULL);

return (xerr);
}