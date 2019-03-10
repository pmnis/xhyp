#ifndef _XHYP_VIRTIO_H
#define _XHYP_VIRTIO_H

#include <xhyp/types.h>

struct vring_desc {
        u64 addr;
        u32 len;
        u16 flags;
        u16 next;
};

#endif
