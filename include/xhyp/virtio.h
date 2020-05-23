
#define VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#define VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#define VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#define VIRTIO_CONFIG_S_FEATURES_OK     8
/* Device entered invalid state, driver must reset it */
#define VIRTIO_CONFIG_S_NEEDS_RESET     0x40
/* We've given up on this device. */
#define VIRTIO_CONFIG_S_FAILED          0x80


#define XHYP_QUEUE_SIZE 100

#if 0
struct virtq_desc {
	le64 addr;
	le32 len;
#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_DESC_F_INDIRECT 4
	le16 flags;
	le16 next;
};

struct virtq_avail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
	le16 flags;
	le16 idx;
	le16 ring[XHYP_QUEUE_SIZE];
	le16 used_event; /* Only if VIRTIO_F_EVENT_IDX */
};

struct virtq_used {
#define VIRTQ_USED_F_NO_NOTIFY 1
	le16 flags;
	le16 idx;
	struct virtq_used_elem ring[XHYP_QUEUE_SIZE];
	le16 avail_event; /* Only if VIRTIO_F_EVENT_IDX */
};

struct virtq_used_elem {
	le32 id;
	le32 len;
};

struct virtq {
	struct virtq_desc desc[XHYP_QUEUE_SIZE];
	struct virtq_avail avail;
	struct virtq_used used;
};
#endif
