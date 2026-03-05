#include "initrd.h"
#include "kheap.h"
#include "string.h"
#include "vga.h"

#define MAX_FILES 64

typedef struct {
	char name[128];
	uint8_t *data;
	uint32_t size;
	uint32_t flags; /* FS_FILE or FS_DIRECTORY */
	int parent_inode; /* Inode of the parent directory */
} initrd_file_t;

static initrd_file_t files[MAX_FILES];
static int file_count = 0;

static struct dirent dirent_buf;

static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
	initrd_file_t *file = &files[node->inode];
	if (offset >= file->size) return 0;
	if (offset + size > file->size) size = file->size - offset;
	memcpy(buffer, (uint8_t*)(file->data + offset), size);
	return size;
}

static uint32_t initrd_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
	initrd_file_t *file = &files[node->inode];
	
	if (offset + size > file->size) {
		uint8_t *new_data = (uint8_t*)kmalloc(offset + size);
		if (file->data) {
			memcpy(new_data, file->data, file->size);
		}
		file->data = new_data;
		file->size = offset + size;
		node->length = file->size;
	}

	memcpy(file->data + offset, buffer, size);
	return size;
}

static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index) {
	uint32_t count = 0;
	int parent_id = node->inode;

	for (int i = 0; i < file_count; i++) {
		if (files[i].name[0] != '\0' && files[i].parent_inode == parent_id) {
			if (count == index) {
				strcpy(dirent_buf.name, files[i].name);
				dirent_buf.ino = i;
				return &dirent_buf;
			}
			count++;
		}
	}
	return 0;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name) {
	int parent_id = node->inode;

	for (int i = 0; i < file_count; i++) {
		if (files[i].name[0] != '\0' && files[i].parent_inode == parent_id && strcmp(name, files[i].name) == 0) {
			fs_node_t *fsnode = (fs_node_t*)kmalloc(sizeof(fs_node_t));
			strcpy(fsnode->name, files[i].name);
			fsnode->flags = files[i].flags;
			fsnode->inode = i;
			fsnode->length = files[i].size;
			fsnode->read = &initrd_read;
			fsnode->write = &initrd_write;
			fsnode->readdir = &initrd_readdir;
			fsnode->finddir = &initrd_finddir;
			fsnode->parent = node;
			return fsnode;
		}
	}
	return 0;
}

static int get_free_slot(void) {
	for (int i = 0; i < file_count; i++) {
		if (files[i].name[0] == '\0') return i;
	}
	if (file_count < MAX_FILES) return file_count++;
	return -1;
}

static void initrd_mkdir(fs_node_t *node, char *name, uint16_t permission) {
	(void)permission;
	int slot = get_free_slot();
	if (slot == -1) return;

	strcpy(files[slot].name, name);
	files[slot].size = 0;
	files[slot].data = 0;
	files[slot].flags = FS_DIRECTORY;
	files[slot].parent_inode = node->inode;
}

static void initrd_create(fs_node_t *node, char *name, uint16_t permission) {
	(void)permission;
	int slot = get_free_slot();
	if (slot == -1) return;

	strcpy(files[slot].name, name);
	files[slot].size = 0;
	files[slot].data = 0;
	files[slot].flags = FS_FILE;
	files[slot].parent_inode = node->inode;
}

static void initrd_unlink(fs_node_t *node, char *name) {
	int parent_id = node->inode;
	for (int i = 0; i < file_count; i++) {
		if (files[i].name[0] != '\0' && files[i].parent_inode == parent_id && strcmp(name, files[i].name) == 0) {
			files[i].name[0] = '\0';
			return;
		}
	}
}

static void add_file_to(int parent_inode, char *name, char *contents) {
	int slot = get_free_slot();
	if (slot == -1) return;
	strcpy(files[slot].name, name);
	files[slot].size = strlen(contents);
	files[slot].data = (uint8_t*)kmalloc(files[slot].size + 1);
	memcpy(files[slot].data, (uint8_t*)contents, files[slot].size);
	files[slot].flags = FS_FILE;
	files[slot].parent_inode = parent_inode;
}

fs_node_t *bin_dir = 0;

fs_node_t *initialise_initrd(void) {
	/* Initialize all slots to empty */
	for (int i = 0; i < MAX_FILES; i++) {
		files[i].name[0] = '\0';
		files[i].parent_inode = -1;
	}
	file_count = 0;

	/* Root node entry (index 0) */
	files[0].flags = FS_DIRECTORY;
	strcpy(files[0].name, "root");
	files[0].parent_inode = -1; /* Root has no parent */
	file_count = 1;

	fs_node_t *initrd_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
	strcpy(initrd_root->name, "initrd");
	initrd_root->flags = FS_DIRECTORY;
	initrd_root->inode = 0;
	initrd_root->readdir = &initrd_readdir;
	initrd_root->finddir = &initrd_finddir;
	initrd_root->mkdir = &initrd_mkdir;
	initrd_root->create = &initrd_create;
	initrd_root->unlink = &initrd_unlink;
	initrd_root->parent = 0;
	fs_root = initrd_root;

	/* Create /bin directory */
	initrd_mkdir(fs_root, "bin", 0);
	bin_dir = initrd_finddir(fs_root, "bin");

	add_file_to(0, "readme.txt", "Welcome to RudzaniOS!\nThis is a simple os with a filesystem and games.\n I am planning to build on it more but aye mate we ");
	add_file_to(0, "author.txt", "Created by Ndaedzo Mudau and Obviously the help of AI its 2026 who isnt using AI to help speed up development these days.\n If you wanna do some to this OS well then go ahead shii im not gonna stop you at All");
	
	/* Add scripts to /bin */
	if (bin_dir) {
		add_file_to(bin_dir->inode, "version.sh", "echo 'RudzaniOS v0.1.0 Experimental'\n");
	}

	return initrd_root;
}
