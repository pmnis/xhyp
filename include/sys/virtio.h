#ifndef _XHYP_VIRTIO_H
#define _XHYP_VIRTIO_H

#include <xhyp/types.h>

struct vring_desc {
	u64 addr;
	u32 len;
	u16 flags;
	u16 next;
};

struct vring_avail {
	u16 flags;
	u16 idx;
	u16 ring[];
};

struct vring_used_elem {
	u32 id;
	u32 len;
};

struct vring_used {
	u16 flags;
	u16 idx;
	struct vring_used_elem ring[];
};

struct vring {
	unsigned int num;
	struct vring_desc *desc;
	struct vring_avail *avail;
	struct vring_used *used;
};

static inline void vring_init(struct vring *vr, unsigned int num, void *p,
			      unsigned long align)
{
	vr->num = num;
	vr->desc = p; 
	vr->avail = p + num*sizeof(struct vring_desc);
	vr->used = (void *)(((unsigned long)&vr->avail->ring[num] + sizeof(u16)
		+ align-1) & ~(align - 1));
}

static inline unsigned vring_size(unsigned int num, unsigned long align)
{
	return ((sizeof(struct vring_desc) * num + sizeof(u16) * (3 + num)
		+ align - 1) & ~(align - 1))
		+ sizeof(u16) * 3 + sizeof(struct vring_used_elem) * num;
}


#endif
