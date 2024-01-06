#define RW_ANS _IOR('a', 'a', struct answer*)

#define BUFFER_SIZE 1024
#define DEVICE_NAME "DEVICESLABTOP"
#define DNAME_INLINE_LEN 32

struct my_slabinfo {
	unsigned long active_objs;
	unsigned long num_objs;
	unsigned int use;
	unsigned long obj_size;
	unsigned long num_slabs;
	unsigned int objects_per_slab;
	unsigned long cache_size;
};

struct answer {
        struct my_slabinfo sld;
	char* status;
        int st_num;
};
