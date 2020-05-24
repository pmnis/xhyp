#define VIRTIO_MMIO_BASE	0x00208000
#define VIRTIO_MMIO_SIZE	0x00000200
#define VIRTIO_MMIO_DEV_MAX	3
#define VIRTIO_MMIO_MAGIC	0x74726976

struct virtio_registers {
	u32 magic;
	u32 version;
	u32 device_id;
	u32 vendor_id;
	u32 host_features;
	u32 host_features_sel;
	u32 reserved_18;
	u32 reserved_1c;
	u32 guest_features;
	u32 guest_features_sel;
	u32 reserved_28;
	u32 reserved_2c;
	u32 queue_sel;
	u32 queue_num_max;
	u32 queue_num;
	u32 queue_align;
	u32 queue_pfn;
	u32 queue_ready;
	u32 reserved_48;
	u32 reserved_4c;
	u32 queue_notify;
	u32 reserved_54;
	u32 reserved_58;
	u32 reserved_5c;
	u32 irq_status;
	u32 irq_ack;
	u32 reserved_68;
	u32 reserved_6c;
	u32 status;
	u32 reserved_74;
	u32 reserved_78;
	u32 reserved_7c;
	u32 queue_desc_low;
	u32 queue_desc_high;
	u32 reserved_88;
	u32 reserved_8c;
	u32 queue_avail_low;
	u32 queue_avail_high;
	u32 reserved_98;
	u32 reserved_9c;
	u32 queue_used_low;
	u32 queue_used_high;
	u8 reserved[0x100 - 0xa8];
	u8 config_space[0x100];
};
