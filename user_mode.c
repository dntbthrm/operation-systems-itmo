#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include "core_mode.h"


static void print_slabinfo(struct my_slabinfo* slb){
        printf("Slabinfo \n");
        printf("number of objects = %lu, \n", slb->num_objs);
        printf("active objects = %lu, \n", slb->active_objs);
        double usage = 0.0;
        if (slb->num_objs != 0){
        	usage = (double)slb->active_objs / (double)slb->num_objs * 100.0;
        	}
       	printf("use = %.2f %%, \n", usage);
       	double obj_kb = slb->obj_size / 1024.0;
       	printf("object size = %.2fK\n", obj_kb);
        printf("number of slabs = %lu, \n", slb->num_slabs);
        printf("objects per slab = %u, \n", slb->objects_per_slab);
        double cache_kb = (slb->active_objs * slb->obj_size)/1024.0;
        printf("cache size = %.2fK, \n", cache_kb);
        printf("\n");
}



static void print_answer(struct answer* a){

        print_slabinfo(&(a->sld));


}

int main(int argc, char *argv[]){

	printf("Select: \n 1 - vm_area_struct \n 2 - dentry \n 3 - vmap_area \n 4 - anon_vma_chain \n");
        if (argc < 1) {
                printf("The program needs an argument\n");
                return -1;
        }

        int fd;
        int32_t value, number;

        long numer = strtol(argv[1], NULL, 10);
        switch (numer){
        	case 1:
        		printf("it is vm_area_struct");
        		break;
        	case 2:
        		printf("it is dentry");
        		break;
        	case 3:
        		printf("it is vmap_area");
        		break;
        	case 4:
        		printf("it is anon_vma_chain");
        		break;
        }


        struct answer answer;

        printf("\nOpening a driver...\n");
        fd = open("/dev/DEVICESLABTOP", O_WRONLY);
        if (fd < 0) {
            printf("Cannot open device file\n");
            return 0;
        }


        answer.st_num = numer;

        ioctl(fd, RW_ANS, (struct answer *) &answer);

        print_answer(&answer);

        printf("Closing Driver...\n");
        close(fd);
}