/* file_system.h - Basic file system structures and function declarations */

#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <stddef.h>

/* File system constants */
#define MAX_FILENAME_LENGTH 32
#define MAX_FILES_PER_DIR   16
#define MAX_FILE_SIZE       4096    /* 4KB max file size */
#define BLOCK_SIZE          512     /* 512 bytes per block */
#define MAX_BLOCKS_PER_FILE 8

/* File types */
#define FILE_TYPE_FILE      0
#define FILE_TYPE_DIRECTORY 1

/* Error codes */
#define FS_SUCCESS          0
#define FS_ERROR_EXISTS     -1
#define FS_ERROR_NOT_FOUND  -2
#define FS_ERROR_FULL       -3
#define FS_ERROR_INVALID    -4
#define FS_ERROR_NO_SPACE   -5

/* File structure */
typedef struct {
    char name[MAX_FILENAME_LENGTH];
    uint32_t size;
    uint8_t type;           /* FILE_TYPE_FILE or FILE_TYPE_DIRECTORY */
    uint32_t blocks[MAX_BLOCKS_PER_FILE];  /* Block pointers to data */
    uint32_t block_count;
    uint32_t parent_dir;    /* Parent directory index */
    uint8_t used;          /* Whether this entry is in use */
} File;

/* Directory structure (same as File for simplicity) */
typedef File Directory;

/* File system structure */
typedef struct {
    File files[MAX_FILES_PER_DIR * 2];  /* Allow files and directories */
    uint32_t file_count;
    uint32_t next_free_block;  /* Next available data block */
    uint8_t* data_blocks;      /* Pointer to data block memory */
    uint32_t total_blocks;     /* Total number of data blocks */
} FileSystem;

/* File system operations */
/* Initialize the file system */
int32_t fs_init(FileSystem* fs, uint8_t* data_memory, uint32_t memory_size);

/* Create a new file */
int32_t fs_create_file(FileSystem* fs, const char* name, uint32_t parent_dir);

/* Create a new directory */
int32_t fs_create_directory(FileSystem* fs, const char* name, uint32_t parent_dir);

/* Read data from a file */
int32_t fs_read_file(FileSystem* fs, uint32_t file_index, uint8_t* buffer, uint32_t size, uint32_t offset);

/* Write data to a file */
int32_t fs_write_file(FileSystem* fs, uint32_t file_index, const uint8_t* data, uint32_t size, uint32_t offset);

/* Delete a file or directory */
int32_t fs_delete(FileSystem* fs, uint32_t file_index);

/* Find a file by name */
int32_t fs_find_file(FileSystem* fs, const char* name, uint32_t parent_dir);

/* Get file information */
int32_t fs_get_file_info(FileSystem* fs, uint32_t file_index, File* info);

/* List directory contents */
int32_t fs_list_directory(FileSystem* fs, uint32_t dir_index, File* entries, uint32_t max_entries);

/* Format file system (clear all files) */
int32_t fs_format(FileSystem* fs);

/* Get error string for error code */
const char* fs_error_string(int32_t error_code);

#endif /* FILE_SYSTEM_H */