#ifndef INITRD_H
#define INITRD_H

#include "fs.h"

/* Initialize the initial ramdisk */
fs_node_t *initialise_initrd(void);

#endif
