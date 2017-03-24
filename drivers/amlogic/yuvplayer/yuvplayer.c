/*
 * Ion Video driver - This code emulates a real video device with v4l2 api,
 * used for surface video display.
 *
 *Author: Jintao Xu <jintao.xu@amlogic.com>
 *
 */
#include "yuvplayer.h"
#include <linux/kfifo.h>
#include <media/videobuf2-core.h>

#define CREATE_TRACE_POINTS
//#include "trace/yuvplayer.h"
#define YUVPLAYER_MODULE_NAME "yuvplayer"

#define YUVPLAYER_VERSION "1.0"
#define PROVIDER_NAME "yuvplayer"


static int is_actived = 0;

static unsigned video_nr = 20;

//static u64 last_pts_us64 = 0;

module_param(video_nr, uint, 0644);
MODULE_PARM_DESC(video_nr, "videoX start number, 13 is autodetect");

static unsigned n_devs = 1;
module_param(n_devs, uint, 0644);
MODULE_PARM_DESC(n_devs, "number of video devices to create");

static unsigned debug = 0;
module_param(debug, uint, 0644);
MODULE_PARM_DESC(debug, "activates debug info");

static unsigned int vid_limit = 16;
module_param(vid_limit, uint, 0644);
MODULE_PARM_DESC(vid_limit, "capture memory limit in megabytes");

static unsigned int freerun_mode = 1;
module_param(freerun_mode, uint, 0664);
MODULE_PARM_DESC(freerun_mode, "av synchronization");

static unsigned int skip_frames = 0;
module_param(skip_frames, uint, 0664);
MODULE_PARM_DESC(skip_frames, "skip frames");

static DECLARE_KFIFO(display_q, vframe_t *, PPMGR2_MAX_CANVAS);
static DECLARE_KFIFO(recycle_q, vframe_t *, PPMGR2_MAX_CANVAS);

static DEFINE_SPINLOCK(lock);


static struct yuvplayer_dmaqueue* cur_dma_q = NULL;

static const struct yuvplayer_fmt formats[] = {
    {
        .name = "RGB32 (LE)",
        .fourcc = V4L2_PIX_FMT_RGB32, /* argb */
        .depth = 32,
    },
    {
        .name = "RGB565 (LE)",
        .fourcc = V4L2_PIX_FMT_RGB565, /* gggbbbbb rrrrrggg */
        .depth = 16,
    },
    {
        .name = "RGB24 (LE)",
        .fourcc = V4L2_PIX_FMT_RGB24, /* rgb */
        .depth = 24,
    },
    {
        .name = "RGB24 (BE)",
        .fourcc = V4L2_PIX_FMT_BGR24, /* bgr */
        .depth = 24,
    },
    {
        .name = "12  Y/CbCr 4:2:0",
        .fourcc   = V4L2_PIX_FMT_NV12,
        .depth    = 12,
    },
    {
        .name     = "12  Y/CrCb 4:2:0",
        .fourcc   = V4L2_PIX_FMT_NV21,
        .depth    = 12,
    },
    {
        .name     = "YUV420P",
        .fourcc   = V4L2_PIX_FMT_YUV420,
        .depth    = 12,
    },
    {
        .name     = "YVU420P",
        .fourcc   = V4L2_PIX_FMT_YVU420,
        .depth    = 12,
    }
};

static vframe_t *yuvplayer_vf_peek(void* op_arg)
{
    vframe_t *vf;
    if (kfifo_peek(&display_q, &vf))
        return vf;

    return NULL;
}

static vframe_t *yuvplayer_vf_get(void* op_arg)
{
    vframe_t *vf;
    if (kfifo_get(&display_q, &vf))
        return vf;

    return NULL;
}

static void yuvplayer_vf_put(vframe_t *vf, void* op_arg)
{
    struct vb2_buffer* vb = (struct vb2_buffer*)vf->private_data;
    vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);
    //dma_q->vb_ready++;
}

static int yuvplayer_event_cb(int type, void *data, void *private_data)
{
    return 0;
}

static int  yuvplayer_vf_states(vframe_states_t *states, void* op_arg)
{
    unsigned long flags;
    spin_lock_irqsave(&lock, flags);

    //states->vf_pool_size = VF_POOL_SIZE;
    states->buf_avail_num = kfifo_len(&display_q);
    //states->buf_recycle_num = kfifo_len(&recycle_q);
    
    spin_unlock_irqrestore(&lock, flags);
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);

    return 0;
}

static const struct vframe_operations_s yuvplayer_provider_operations = {
    .peek = yuvplayer_vf_peek,
    .get = yuvplayer_vf_get,
    .put = yuvplayer_vf_put,
    .event_cb = yuvplayer_event_cb,
    .vf_states = yuvplayer_vf_states,
};

static const struct yuvplayer_fmt *__get_format(u32 pixelformat)
{
    const struct yuvplayer_fmt *fmt;
    unsigned int k;
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);

    for (k = 0; k < ARRAY_SIZE(formats); k++) {
        fmt = &formats[k];
        if (fmt->fourcc == pixelformat)
            break;
    }

    if (k == ARRAY_SIZE(formats))
        return NULL;
	//printk("[%s %d] k:%d\n", __FUNCTION__, __LINE__, k);
    return &formats[k];
}

static const struct yuvplayer_fmt *get_format(struct v4l2_format *f)
{
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);

    return __get_format(f->fmt.pix.pixelformat);
}

static LIST_HEAD (yuvplayer_devlist);

//static DEFINE_SPINLOCK(ion_states_lock);

/* ------------------------------------------------------------------
 DMA and thread functions
 ------------------------------------------------------------------*/
unsigned get_yuvplayer_debug(void) {
    return debug;
}
EXPORT_SYMBOL(get_yuvplayer_debug);

int is_yuvplayer_active(void) {
    return is_actived;
}
EXPORT_SYMBOL(is_yuvplayer_active);


static int yuvplayer_fillbuff(struct yuvplayer_dev *dev, struct yuvplayer_buffer *buf) {
    struct vb2_buffer *vb = &(buf->vb);
    //int ret = 0;
//-------------------------------------------------------

    vframe_t *vf = &dev->vfpool[vb->v4l2_buf.index];

    vf->width = dev->width;
    vf->height = dev->height;
    vf->bufWidth = dev->width;
    vf->flag = 0;
    vf->orientation = 0;
    //buf->vb.v4l2_buf.timestamp.tv_sec = dev->pts >> 32;
    //buf->vb.v4l2_buf.timestamp.tv_usec = dev->pts & 0xFFFFFFFF;
    //vf->pts = 0;
    //vf->pts_us64 = 0;
    vf->duration = 0x3000;
    vf->duration_pulldown = 0x3000;
    vf->type = VIDTYPE_PROGRESSIVE | VIDTYPE_VIU_FIELD | VIDTYPE_LITTLE_ENDIAN;
    //vf->canvas0Addr = vf->canvas1Addr = canvasindex;
    vf->private_data = (void*)vb;
    //printk("/vf->pts=%d,vf->pts_us64=%lld\n", vf->pts, vf->pts_us64);
    kfifo_put(&display_q, (const vframe_t **)&vf);
    vf_notify_receiver(PROVIDER_NAME,VFRAME_EVENT_PROVIDER_VFRAME_READY,NULL);
    
//-------------------------------------------------------
    return 0;
}

static void yuvplayer_thread_tick(struct yuvplayer_dev *dev) {
    struct yuvplayer_dmaqueue *dma_q = &dev->vidq;
    struct yuvplayer_buffer *buf;
    unsigned long flags = 0;
   // struct vframe_s* vf;
    static int vf_wait_cnt = 0;

	//int w ,h ;
    dprintk(dev, 4, "Thread tick\n");

    if(!dev){
		return;
	}

    spin_lock_irqsave(&dev->slock, flags);
    if (list_empty(&dma_q->active)) {
        dprintk(dev, 3, "No active queue to serve\n");
        spin_unlock_irqrestore(&dev->slock, flags);
        schedule_timeout_interruptible(msecs_to_jiffies(20));
        return;
    }
    buf = list_entry(dma_q->active.next, struct yuvplayer_buffer, list);
    spin_unlock_irqrestore(&dev->slock, flags);
    if (yuvplayer_fillbuff(dev, buf)) {
        return;
    }
    vf_wait_cnt = 0;

    spin_lock_irqsave(&dev->slock, flags);
    list_del(&buf->list);
    spin_unlock_irqrestore(&dev->slock, flags);
    //vb2_buffer_done(&buf->vb, VB2_BUF_STATE_DONE);
    //dma_q->vb_ready++;
    dprintk(dev, 4, "[%p/%d] done\n", buf, buf->vb.v4l2_buf.index);
}

#define frames_to_ms(frames)					\
    ((frames * WAKE_NUMERATOR * 1000) / WAKE_DENOMINATOR)

static void yuvplayer_sleep(struct yuvplayer_dev *dev) {
    struct yuvplayer_dmaqueue *dma_q = &dev->vidq;
    //int timeout;
    DECLARE_WAITQUEUE(wait, current);

    dprintk(dev, 4, "%s dma_q=0x%08lx\n", __func__, (unsigned long)dma_q);

    add_wait_queue(&dma_q->wq, &wait);
    if (kthread_should_stop())
        goto stop_task;

    /* Calculate time to wake up */
    //timeout = msecs_to_jiffies(frames_to_ms(1));

    yuvplayer_thread_tick(dev);

    //schedule_timeout_interruptible(timeout);

stop_task:
    remove_wait_queue(&dma_q->wq, &wait);
    try_to_freeze();
}

static int yuvplayer_thread(void *data) {
    struct yuvplayer_dev *dev = data;

    dprintk(dev, 2, "thread started\n");
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);

    set_freezable();

    for (;;) {
        yuvplayer_sleep(dev);

        if (kthread_should_stop())
            break;
    }
    dprintk(dev, 2, "thread: exit\n");
    return 0;
}

static int yuvplayer_start_generating(struct yuvplayer_dev *dev) {
    struct yuvplayer_dmaqueue *dma_q = &dev->vidq;
    dev->is_omx_video_started = 1;
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);

    dprintk(dev, 2, "%s\n", __func__);

    /* Resets frame counters */
    dev->ms = 0;
    //dev->jiffies = jiffies;

    dma_q->kthread = kthread_run(yuvplayer_thread, dev, dev->v4l2_dev.name);

    if (IS_ERR(dma_q->kthread)) {
        v4l2_err(&dev->v4l2_dev, "kernel_thread() failed\n");
        return PTR_ERR(dma_q->kthread);
    }
    /* Wakes thread */
    wake_up_interruptible(&dma_q->wq);

    dprintk(dev, 2, "returning from %s\n", __func__);
    return 0;
}

static void yuvplayer_stop_generating(struct yuvplayer_dev *dev) {
    struct yuvplayer_dmaqueue *dma_q = &dev->vidq;
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);

    dprintk(dev, 2, "%s\n", __func__);

    /* shutdown control thread */
    if (dma_q->kthread) {
        kthread_stop(dma_q->kthread);
        dma_q->kthread = NULL;
    }

    /*
     * Typical driver might need to wait here until dma engine stops.
     * In this case we can abort imiedetly, so it's just a noop.
     */

    /* Release all active buffers */
    while (!list_empty(&dma_q->active)) {
        struct yuvplayer_buffer *buf;
        buf = list_entry(dma_q->active.next, struct yuvplayer_buffer, list);
        list_del(&buf->list);
        vb2_buffer_done(&buf->vb, VB2_BUF_STATE_ERROR);
        dprintk(dev, 2, "[%p/%d] done\n", buf, buf->vb.v4l2_buf.index);
    }
}
/* ------------------------------------------------------------------
 Videobuf operations
 ------------------------------------------------------------------*/
static int queue_setup(struct vb2_queue *vq, const struct v4l2_format *fmt, unsigned int *nbuffers, unsigned int *nplanes, unsigned int sizes[], void *alloc_ctxs[]) {
    struct yuvplayer_dev *dev = vb2_get_drv_priv(vq);
    unsigned long size;

    if (fmt)
        size = fmt->fmt.pix.sizeimage;
    else
        size = (dev->width * dev->height * dev->pixelsize) >> 3;

    if (size == 0)
        return -EINVAL;
	
	//printk("[%s %d]nbuffers:%d\n", __FUNCTION__, __LINE__, *nbuffers);

    if (0 == *nbuffers)
        *nbuffers = 32;

	//printk("[%s %d]size:%lu vid_limit:%u\n", __FUNCTION__, __LINE__, size, vid_limit);

    while (size * *nbuffers > vid_limit * MAX_WIDTH * MAX_HEIGHT)
        (*nbuffers)--;

    *nplanes = 1;

    sizes[0] = size;

    /*
     * videobuf2-vmalloc allocator is context-less so no need to set
     * alloc_ctxs array.
     */

    //printk("%s, count=%d, size=%ld\n", __func__, *nbuffers, size);

    return 0;
}

static int buffer_prepare(struct vb2_buffer *vb) {
    struct yuvplayer_dev *dev = vb2_get_drv_priv(vb->vb2_queue);
    struct yuvplayer_buffer *buf = container_of(vb, struct yuvplayer_buffer, vb);
    unsigned long size;

    dprintk(dev, 2, "%s, field=%d\n", __func__, vb->v4l2_buf.field);
	

    BUG_ON(NULL == dev->fmt);

    /*
     * Theses properties only change when queue is idle, see s_fmt.
     * The below checks should not be performed here, on each
     * buffer_prepare (i.e. on each qbuf). Most of the code in this function
     * should thus be moved to buffer_init and s_fmt.
     */
    if (dev->width < 48 || dev->width > MAX_WIDTH || dev->height < 32 || dev->height > MAX_HEIGHT)
        return -EINVAL;

    size = (dev->width * dev->height * dev->pixelsize) >> 3;
    if (vb2_plane_size(vb, 0) < size) {
        dprintk(dev, 1, "%s data will not fit into plane (%lu < %lu)\n", __func__, vb2_plane_size(vb, 0), size);
        return -EINVAL;
    }

	//printk("[%s %d] size=%lu pixelsize:%u\n", __func__, __LINE__, size, dev->pixelsize);

    vb2_set_plane_payload(&buf->vb, 0, size);

    buf->fmt = dev->fmt;

    return 0;
}

static void buffer_queue(struct vb2_buffer *vb) {
    struct yuvplayer_dev *dev = vb2_get_drv_priv(vb->vb2_queue);
    struct yuvplayer_buffer *buf = container_of(vb, struct yuvplayer_buffer, vb);
    struct yuvplayer_dmaqueue *vidq = &dev->vidq;
    unsigned long flags = 0;

    dprintk(dev, 2, "%s\n", __func__);

	//printk("[%s %d] index:%d\n", __FUNCTION__, __LINE__, vb->v4l2_buf.index);

    spin_lock_irqsave(&dev->slock, flags);
    list_add_tail(&buf->list, &vidq->active);
    spin_unlock_irqrestore(&dev->slock, flags);
}

static int start_streaming(struct vb2_queue *vq, unsigned int count) {
    struct yuvplayer_dev *dev = vb2_get_drv_priv(vq);
    struct yuvplayer_dmaqueue *dma_q = &dev->vidq;

    cur_dma_q = dma_q;
    is_actived = 1;
    dma_q->vb_ready = 0;
    dprintk(dev, 2, "%s\n", __func__);

	//printk("[%s %d]\n", __FUNCTION__, __LINE__);
    return yuvplayer_start_generating(dev);
}

/* abort streaming and wait for last buffer */
static int stop_streaming(struct vb2_queue *vq) {
    struct yuvplayer_dev *dev = vb2_get_drv_priv(vq);
    cur_dma_q = NULL;
    is_actived = 0;
    dprintk(dev, 2, "%s\n", __func__);
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);
    yuvplayer_stop_generating(dev);

    return 0;
}

static void yuvplayer_lock(struct vb2_queue *vq) {
    struct yuvplayer_dev *dev = vb2_get_drv_priv(vq);
    mutex_lock(&dev->mutex);
}

static void yuvplayer_unlock(struct vb2_queue *vq) {
    struct yuvplayer_dev *dev = vb2_get_drv_priv(vq);
    mutex_unlock(&dev->mutex);
}

static const struct vb2_ops yuvplayer_video_qops = {
    .queue_setup = queue_setup,
    .buf_prepare = buffer_prepare,
    .buf_queue = buffer_queue,
    .start_streaming = start_streaming,
    .stop_streaming = stop_streaming,
    .wait_prepare = yuvplayer_unlock,
    .wait_finish = yuvplayer_lock,
};

/* ------------------------------------------------------------------
 IOCTL vidioc handling
 ------------------------------------------------------------------*/
static int vidioc_open(struct file *file) {
    struct yuvplayer_dev *dev = video_drvdata(file);
    u32 framerate = 0;
    if (dev->fd_num > 0 || ppmgr3_init(&(dev->ppmgr2_dev)) < 0) {
        return -EBUSY;
    }
    dev->fd_num++;
    dev->pts = 0;
    dev->c_width = 0;
    dev->c_height = 0;
    dev->once_record = 1;
    dev->ppmgr2_dev.bottom_first = 0;
    skip_frames = 0;
    //printk("yuvplayer open\n");
    INIT_KFIFO(display_q);
	INIT_KFIFO(recycle_q);
    vf_provider_init(&dev->video_vf_prov, PROVIDER_NAME, &yuvplayer_provider_operations, NULL);
    vf_reg_provider(&dev->video_vf_prov);
    vf_notify_receiver(PROVIDER_NAME,VFRAME_EVENT_PROVIDER_START,NULL);
    
    vf_notify_receiver(PROVIDER_NAME, VFRAME_EVENT_PROVIDER_FR_HINT, (void *)framerate);
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);
    return v4l2_fh_open(file);
}

static int vidioc_release(struct file *file) {
    struct yuvplayer_dev *dev = video_drvdata(file);
    yuvplayer_stop_generating(dev);
    //printk("yuvplayer_stop_generating!!!!\n");
    ppmgr3_release(&(dev->ppmgr2_dev));
    dprintk(dev, 2, "vidioc_release\n");
    //printk("yuvplayer release\n");
    if (dev->fd_num > 0) {
        dev->fd_num--;
    }
    dev->once_record = 0;
    vf_unreg_provider(&dev->video_vf_prov);
    //printk("[%s %d]\n", __FUNCTION__, __LINE__);
    return vb2_fop_release(file);
}

static int vidioc_querycap(struct file *file, void *priv, struct v4l2_capability *cap) {
    struct yuvplayer_dev *dev = video_drvdata(file);

    strcpy(cap->driver, "yuvplayer");
    strcpy(cap->card, "yuvplayer");
    snprintf(cap->bus_info, sizeof(cap->bus_info), "platform:%s", dev->v4l2_dev.name);
    cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING | V4L2_CAP_READWRITE;
    cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	//printk("[%s %d]\n", __FUNCTION__, __LINE__);
    return 0;
}

static int vidioc_enum_fmt_vid_cap(struct file *file, void *priv, struct v4l2_fmtdesc *f) {
    const struct yuvplayer_fmt *fmt;

    if (f->index >= ARRAY_SIZE(formats))
        return -EINVAL;

    fmt = &formats[f->index];

    strlcpy(f->description, fmt->name, sizeof(f->description));
    f->pixelformat = fmt->fourcc;
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);
    return 0;
}

static int vidioc_g_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *f) {
    struct yuvplayer_dev *dev = video_drvdata(file);
    struct vb2_queue *q = &dev->vb_vidq;
    int ret = 0;
    unsigned long flags;

    if (freerun_mode == 0) {
        if (dev->c_width == 0 || dev->c_height == 0) {
            return -EINVAL;
        }
        f->fmt.pix.width = dev->c_width;
        f->fmt.pix.height = dev->c_height;
        spin_lock_irqsave(&q->done_lock, flags);
        ret = list_empty(&q->done_list);
        spin_unlock_irqrestore(&q->done_lock, flags);
        if (!ret) {
            return -EAGAIN;
        }
    } else {
        f->fmt.pix.width = dev->width;
        f->fmt.pix.height = dev->height;
    }
    f->fmt.pix.field = V4L2_FIELD_INTERLACED;
    f->fmt.pix.pixelformat = dev->fmt->fourcc;
    f->fmt.pix.bytesperline = (f->fmt.pix.width * dev->fmt->depth) >> 3;
    f->fmt.pix.sizeimage = f->fmt.pix.height * f->fmt.pix.bytesperline;
    if (dev->fmt->is_yuv)
        f->fmt.pix.colorspace = V4L2_COLORSPACE_SMPTE170M;
    else
        f->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
	
	//printk("[%s %d]\n", __FUNCTION__, __LINE__);

    return 0;
}

static int vidioc_try_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *f) {
    struct yuvplayer_dev *dev = video_drvdata(file);
    const struct yuvplayer_fmt *fmt;

    fmt = get_format(f);
    if (!fmt) {
        dprintk(dev, 1, "Fourcc format (0x%08x) unknown.\n", f->fmt.pix.pixelformat);
        return -EINVAL;
    }

    f->fmt.pix.field = V4L2_FIELD_INTERLACED;
    v4l_bound_align_image(&f->fmt.pix.width, 48, MAX_WIDTH, 4, &f->fmt.pix.height, 32, MAX_HEIGHT, 0, 0);
    f->fmt.pix.bytesperline = (f->fmt.pix.width * fmt->depth) >> 3;
    f->fmt.pix.sizeimage = f->fmt.pix.height * f->fmt.pix.bytesperline;
    if (fmt->is_yuv)
        f->fmt.pix.colorspace = V4L2_COLORSPACE_SMPTE170M;
    else
        f->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
    f->fmt.pix.priv = 0;
    return 0;
}

static int vidioc_s_fmt_vid_cap(struct file *file, void *priv, struct v4l2_format *f) {
    struct yuvplayer_dev *dev = video_drvdata(file);
    struct vb2_queue *q = &dev->vb_vidq;

    int ret = vidioc_try_fmt_vid_cap(file, priv, f);
    if (ret < 0)
        return ret;

    if (vb2_is_busy(q)) {
        dprintk(dev, 1, "%s device busy\n", __func__);
        return -EBUSY;
    }
    dev->fmt = get_format(f);
    dev->pixelsize = dev->fmt->depth;
    dev->width = f->fmt.pix.width;
    dev->height = f->fmt.pix.height;

	//printk("[%s %d] w:%d h:%d\n", __FUNCTION__, __LINE__, dev->width, dev->height);

    return 0;
}

static int vidioc_enum_framesizes(struct file *file, void *fh, struct v4l2_frmsizeenum *fsize) {
    static const struct v4l2_frmsize_stepwise sizes = { 48, MAX_WIDTH, 4, 32, MAX_HEIGHT, 1 };
    int i;

    if (fsize->index)
        return -EINVAL;
    for (i = 0; i < ARRAY_SIZE(formats); i++)
        if (formats[i].fourcc == fsize->pixel_format)
            break;
    if (i == ARRAY_SIZE(formats))
        return -EINVAL;
    fsize->type = V4L2_FRMSIZE_TYPE_STEPWISE;
    fsize->stepwise = sizes;
    return 0;
}

static int vidioc_qbuf(struct file *file, void *priv, struct v4l2_buffer *p) {
    struct yuvplayer_dev *dev = video_drvdata(file);
    struct yuvplayer_dmaqueue *dma_q = &dev->vidq;
    struct ppmgr2_device* ppmgr2_dev = &(dev->ppmgr2_dev);
    int ret = 0;

	//printk("[%s %d] index:%d\n", __func__, __LINE__, p->index);
	
    ret = vb2_ioctl_qbuf(file, priv, p);
    if (ret != 0) { return ret; }

    if (!ppmgr2_dev->phy_addr[p->index]){
        struct vb2_buffer *vb;
        struct vb2_queue *q;
        void* phy_addr = NULL;
        q = dev->vdev.queue;
        vb = q->bufs[p->index];
        phy_addr = vb2_plane_cookie(vb, 0);
		//printk("[%s %d] index:%d phy_addr:%x name:%s\n", __func__, __LINE__, p->index, (int)phy_addr, dev->fmt->name);
        if (phy_addr) {
            ret = ppmgr3_canvas_config(ppmgr2_dev, dev->width, dev->height, dev->fmt->fourcc, phy_addr, p->index);
        } else {
            return -ENOMEM;
        }

        dev->vfpool[p->index].index = p->index;
        dev->vfpool[p->index].canvas0Addr = dev->vfpool[p->index].canvas1Addr = ppmgr2_dev->canvas_id[p->index];
    }
    
    dev->vfpool[p->index].pts_us64 = p->timestamp.tv_sec & 0xFFFFFFFF;
    dev->vfpool[p->index].pts_us64 <<= 32;
    dev->vfpool[p->index].pts_us64 += p->timestamp.tv_usec & 0xFFFFFFFF;
    //dev->vfpool[p->index].pts = div_u64(dev->vfpool[p->index].pts_us64 * 9, 100);
	dev->vfpool[p->index].pts = p->timestamp.tv_usec & 0xFFFFFFFF;

	//printk("[%s %d] index:%d\n", __FUNCTION__, __LINE__, p->index);

    wake_up_interruptible(&dma_q->wq);
    return ret;
}

static int vidioc_dqbuf(struct file *file, void *priv, struct v4l2_buffer *p){
    struct yuvplayer_dev *dev = video_drvdata(file);
    struct yuvplayer_dmaqueue *dma_q = &dev->vidq;
    int ret = 0;

    ret = vb2_ioctl_dqbuf(file, priv, p);

    if (ret == 0) {
		//printk("vidioc_dqbuf_ret vb_ready=%d index:%d\n",dma_q->vb_ready, p->index);
        dma_q->vb_ready--;
    }
    return ret;
}

#define NUM_INPUTS 10
/* only one input in this sample driver */
static int vidioc_enum_input(struct file *file, void *priv, struct v4l2_input *inp) {
    if (inp->index >= NUM_INPUTS)
        return -EINVAL;

    inp->type = V4L2_INPUT_TYPE_CAMERA;
    sprintf(inp->name, "Camera %u", inp->index);
	printk("[%s %d] name:%s\n", __FUNCTION__, __LINE__, inp->name);
    return 0;
}

static int vidioc_g_input(struct file *file, void *priv, unsigned int *i) {
    struct yuvplayer_dev *dev = video_drvdata(file);

    *i = dev->input;

	printk("[%s %d] input:%d\n", __FUNCTION__, __LINE__, dev->input);
	
    return 0;
}

static int vidioc_s_input(struct file *file, void *priv, unsigned int i) {
    struct yuvplayer_dev *dev = video_drvdata(file);

    if (i >= NUM_INPUTS)
        return -EINVAL;

	printk("[%s %d] i:%d\n", __FUNCTION__, __LINE__, i);

    if (i == dev->input)
        return 0;
	
	printk("[%s %d] i:%d\n", __FUNCTION__, __LINE__, i);

    dev->input = i;
    return 0;
}

/* ------------------------------------------------------------------
 File operations for the device
 ------------------------------------------------------------------*/
static const struct v4l2_file_operations yuvplayer_fops = {
    .owner = THIS_MODULE,
    .open = vidioc_open,
    .release = vidioc_release,
    .read = vb2_fop_read,
    .poll = vb2_fop_poll,
    .unlocked_ioctl = video_ioctl2, /* V4L2 ioctl handler */
    .mmap = vb2_fop_mmap,
};

static const struct v4l2_ioctl_ops yuvplayer_ioctl_ops = {
    .vidioc_querycap = vidioc_querycap,
    .vidioc_enum_fmt_vid_cap = vidioc_enum_fmt_vid_cap,
    .vidioc_g_fmt_vid_cap = vidioc_g_fmt_vid_cap,
    .vidioc_try_fmt_vid_cap = vidioc_try_fmt_vid_cap,
    .vidioc_s_fmt_vid_cap = vidioc_s_fmt_vid_cap,
    .vidioc_enum_framesizes = vidioc_enum_framesizes,
    .vidioc_reqbufs = vb2_ioctl_reqbufs,
    .vidioc_create_bufs = vb2_ioctl_create_bufs,
    .vidioc_prepare_buf = vb2_ioctl_prepare_buf,
    .vidioc_querybuf = vb2_ioctl_querybuf,
    .vidioc_qbuf = vidioc_qbuf,
    .vidioc_dqbuf = vidioc_dqbuf,
    .vidioc_enum_input = vidioc_enum_input,
    .vidioc_g_input = vidioc_g_input,
    .vidioc_s_input = vidioc_s_input,
    .vidioc_streamon = vb2_ioctl_streamon,
    .vidioc_streamoff = vb2_ioctl_streamoff,
    .vidioc_log_status = v4l2_ctrl_log_status,
    .vidioc_subscribe_event = v4l2_ctrl_subscribe_event,
    .vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};

static const struct video_device yuvplayer_template = {
    .name = "yuvplayer",
    .fops = &yuvplayer_fops,
    .ioctl_ops = &yuvplayer_ioctl_ops,
    .release = video_device_release_empty,
};

/* -----------------------------------------------------------------
 Initialization and module stuff
 ------------------------------------------------------------------*/
//struct vb2_dc_conf * yuvplayer_dma_ctx = NULL;
static int yuvplayer_release(void) {
    struct yuvplayer_dev *dev;
    struct list_head *list;

    while (!list_empty(&yuvplayer_devlist)) {
        list = yuvplayer_devlist.next;
        list_del(list);
        dev = list_entry(list, struct yuvplayer_dev, yuvplayer_devlist);

        v4l2_info(&dev->v4l2_dev, "unregistering %s\n", video_device_node_name(&dev->vdev));
        video_unregister_device(&dev->vdev);
        v4l2_device_unregister(&dev->v4l2_dev);
        kfree(dev);
    }
    //vb2_dma_contig_cleanup_ctx(yuvplayer_dma_ctx);

    return 0;
}

static int __init yuvplayer_create_instance(int inst)
{
    struct yuvplayer_dev *dev;
    struct video_device *vfd;
    struct vb2_queue *q;
    int ret;
   // u32 framerate = 0;

    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
    return -ENOMEM;

    snprintf(dev->v4l2_dev.name, sizeof(dev->v4l2_dev.name),
            "%s-%03d", YUVPLAYER_MODULE_NAME, inst);
    ret = v4l2_device_register(NULL, &dev->v4l2_dev);
    if (ret)
    goto free_dev;

    dev->fmt = &formats[0];
    dev->width = 640;
    dev->height = 480;
    dev->pixelsize = dev->fmt->depth;
    dev->fd_num = 0;

    /* initialize locks */
    spin_lock_init(&dev->slock);

    /* initialize queue */
    q = &dev->vb_vidq;
    q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF | VB2_READ;
    q->drv_priv = dev;
    q->buf_struct_size = sizeof(struct yuvplayer_buffer);
    q->ops = &yuvplayer_video_qops;
    q->mem_ops = &vb2_ion_memops;
    q->timestamp_type = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;

    ret = vb2_queue_init(q);
    if (ret)
    goto unreg_dev;

    mutex_init(&dev->mutex);

    /* init video dma queues */
    INIT_LIST_HEAD(&dev->vidq.active);
    init_waitqueue_head(&dev->vidq.wq);
    dev->vidq.pdev = dev;

    vfd = &dev->vdev;
    *vfd = yuvplayer_template;
    vfd->debug = debug;
    vfd->v4l2_dev = &dev->v4l2_dev;
    vfd->queue = q;
    set_bit(V4L2_FL_USE_FH_PRIO, &vfd->flags);

    /*
     * Provide a mutex to v4l2 core. It will be used to protect
     * all fops and v4l2 ioctls.
     */
    vfd->lock = &dev->mutex;
    video_set_drvdata(vfd, dev);

    ret = video_register_device(vfd, VFL_TYPE_GRABBER, video_nr);
    if (ret < 0)
        goto unreg_dev;

    /* Now that everything is fine, let's add it to device list */
    list_add_tail(&dev->yuvplayer_devlist, &yuvplayer_devlist);
    v4l2_info(&dev->v4l2_dev, "V4L2 device registered as %s\n",
            video_device_node_name(vfd));
    return 0;

unreg_dev:
    v4l2_device_unregister(&dev->v4l2_dev);
free_dev:
    kfree(dev);
    return ret;
}

static struct class_attribute ion_video_class_attrs[] = {
    __ATTR_NULL
};
static struct class yuvplayer_class = {
        .name = "yuvplayer",
        .class_attrs = ion_video_class_attrs,
};

/* This routine allocates from 1 to n_devs virtual drivers.

 The real maximum number of virtual drivers will depend on how many drivers
 will succeed. This is limited to the maximum number of devices that
 videodev supports, which is equal to VIDEO_NUM_DEVICES.
 */
static int __init yuvplayer_init(void)
{
    int ret = 0, i;
    ret = class_register(&yuvplayer_class);
    if(ret<0)
        return ret;
    if (n_devs <= 0)
    n_devs = 1;

    for (i = 0; i < n_devs; i++) {
        ret = yuvplayer_create_instance(i);
        if (ret) {
            /* If some instantiations succeeded, keep driver */
            if (i)
            ret = 0;
            break;
        }
    }

    if (ret < 0) {
        printk(KERN_ERR "yuvplayer: error %d while loading driver\n", ret);
        return ret;
    }

    printk(KERN_INFO "Video Technology Magazine Ion Video "
            "Capture Board ver %s successfully loaded.\n",
            YUVPLAYER_VERSION);

    /* n_devs will reflect the actual number of allocated devices */
    n_devs = i;
    
    return ret;
}

static void __exit yuvplayer_exit(void)
{
    yuvplayer_release();
	class_unregister(&yuvplayer_class);
}

MODULE_DESCRIPTION("Video Technology Magazine Ion Video Capture Board");
MODULE_AUTHOR("Amlogic, Jintao Xu<jintao.xu@amlogic.com>");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION(YUVPLAYER_VERSION);

module_init (yuvplayer_init);
module_exit (yuvplayer_exit);
