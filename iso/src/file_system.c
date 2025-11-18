/* file_system.c - Basic file system implementation
   In-memory hierarchical filesystem with fixed-size directory entries and
   simple linear block allocation over a contiguous data area. Designed for
   clarity over completeness; no on-disk persistence or fragmentation management. */

#include "file_system.h"
#include "error_codes.h"
#include "kernel.h"
#include <string.h>

/* Helper function to find a free file entry */
static int find_free_entry(FileSystem* fs) {
    /* Scan the fixed-size table for an unused entry; returns index or -1. */
    for (uint32_t i = 0; i < MAX_FILES_PER_DIR * 2; i++) {
        if (!fs->files[i].used) {
            return i;
        }
    }
    return -1;
}

/* Helper function to allocate data blocks */
static int allocate_blocks(FileSystem* fs, uint32_t* blocks, uint32_t block_count) {
    /* Linear allocation: hands out sequential block numbers. */
    if (fs->next_free_block + block_count > fs->total_blocks) {
        return FS_ERROR_NO_SPACE;
    }
    
    for (uint32_t i = 0; i < block_count; i++) {
        blocks[i] = fs->next_free_block++;
    }
    return FS_SUCCESS;
}

/* Helper function to free data blocks */
static void free_blocks(FileSystem* fs, uint32_t* blocks, uint32_t block_count) {
    /* Naive free: only rewinds if last freed range sits at the end. */
    /* Simple implementation - just mark as free by adjusting next_free_block */
    /* In a real filesystem, we'd maintain a free block bitmap */
    if (block_count > 0 && blocks[block_count - 1] + 1 == fs->next_free_block) {
        fs->next_free_block = blocks[0];
    }
}

/* Initialize the file system */
int32_t fs_init(FileSystem* fs, uint8_t* data_memory, uint32_t memory_size) {
    /* Initialize root directory and set up the data block pool. */
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs || !data_memory || memory_size < BLOCK_SIZE) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Clear all file entries */
    memset(fs->files, 0, sizeof(fs->files));
    fs->file_count = 0;
    fs->data_blocks = data_memory;
    fs->total_blocks = memory_size / BLOCK_SIZE;
    fs->next_free_block = 0;
    
    /* Validate block calculation */
    if (fs->total_blocks == 0) {
        error_code = ERR_OUT_OF_MEMORY;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Create root directory */
    int result = fs_create_directory(fs, "/", 0);
    if (result < 0) {
        error_code = ERR_FILE_SYSTEM_INIT_FAILED;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    return ERR_SUCCESS;
}

/* Create a new file */
int32_t fs_create_file(FileSystem* fs, const char* name, uint32_t parent_dir) {
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs || !name) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    if (strlen(name) >= MAX_FILENAME_LENGTH) {
        error_code = ERR_FILE_NAME_TOO_LONG;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Validate parent directory exists */
    if (parent_dir != 0) {  /* Root directory is always valid */
        if (parent_dir >= MAX_FILES_PER_DIR * 2 || !fs->files[parent_dir].used) {
            error_code = ERR_INVALID_DIRECTORY;
            HANDLE_ERROR(error_code);
            return error_code;
        }
        if (fs->files[parent_dir].type != FILE_TYPE_DIRECTORY) {
            error_code = ERR_NOT_A_DIRECTORY;
            HANDLE_ERROR(error_code);
            return error_code;
        }
    }
    
    /* Check if file already exists */
    if (fs_find_file(fs, name, parent_dir) >= 0) {
        error_code = ERR_FILE_EXISTS;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Find free entry */
    int entry_index = find_free_entry(fs);
    if (entry_index < 0) {
        error_code = ERR_FILE_SYSTEM_FULL;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Initialize file entry */
    File* file = &fs->files[entry_index];
    memset(file, 0, sizeof(File));
    strcpy(file->name, name);
    file->type = FILE_TYPE_FILE;
    file->size = 0;
    file->block_count = 0;
    file->parent_dir = parent_dir;
    file->used = 1;
    
    fs->file_count++;
    return entry_index;
}

/* Create a new directory */
int32_t fs_create_directory(FileSystem* fs, const char* name, uint32_t parent_dir) {
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs || !name) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    if (strlen(name) >= MAX_FILENAME_LENGTH) {
        error_code = ERR_FILE_NAME_TOO_LONG;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Validate parent directory exists */
    if (parent_dir != 0) {  /* Root directory is always valid */
        if (parent_dir >= MAX_FILES_PER_DIR * 2 || !fs->files[parent_dir].used) {
            error_code = ERR_INVALID_DIRECTORY;
            HANDLE_ERROR(error_code);
            return error_code;
        }
        if (fs->files[parent_dir].type != FILE_TYPE_DIRECTORY) {
            error_code = ERR_NOT_A_DIRECTORY;
            HANDLE_ERROR(error_code);
            return error_code;
        }
    }
    
    /* Check if directory already exists */
    if (fs_find_file(fs, name, parent_dir) >= 0) {
        error_code = ERR_FILE_EXISTS;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Find free entry */
    int entry_index = find_free_entry(fs);
    if (entry_index < 0) {
        error_code = ERR_FILE_SYSTEM_FULL;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Initialize directory entry */
    File* dir = &fs->files[entry_index];
    memset(dir, 0, sizeof(File));
    strcpy(dir->name, name);
    dir->type = FILE_TYPE_DIRECTORY;
    dir->size = 0;
    dir->block_count = 0;
    dir->parent_dir = parent_dir;
    dir->used = 1;
    
    fs->file_count++;
    return entry_index;
}

/* Read data from a file */
int32_t fs_read_file(FileSystem* fs, uint32_t file_index, uint8_t* buffer, uint32_t size, uint32_t offset) {
    /* Read up to size bytes starting at offset, walking block-by-block with bounds checks. */
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs || !buffer) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    if (size == 0) {
        return 0;  /* Nothing to read */
    }
    
    if (file_index >= MAX_FILES_PER_DIR * 2 || !fs->files[file_index].used) {
        error_code = ERR_INVALID_FILE_HANDLE;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    File* file = &fs->files[file_index];
    if (file->type != FILE_TYPE_FILE) {
        error_code = ERR_NOT_A_FILE;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Check bounds */
    if (offset >= file->size) {
        return 0;  /* Nothing to read */
    }
    
    uint32_t read_size = size;
    if (offset + size > file->size) {
        read_size = file->size - offset;
    }
    
    /* Read data block by block */
    uint32_t bytes_read = 0;
    uint32_t current_offset = offset;
    
    while (bytes_read < read_size && current_offset < file->size) {
        uint32_t block_index = current_offset / BLOCK_SIZE;
        uint32_t block_offset = current_offset % BLOCK_SIZE;
        uint32_t bytes_in_block = BLOCK_SIZE - block_offset;
        uint32_t bytes_to_read = (read_size - bytes_read) < bytes_in_block ? 
                                (read_size - bytes_read) : bytes_in_block;
        
        if (block_index >= file->block_count) {
            error_code = ERR_FILE_CORRUPTED;
            HANDLE_ERROR(error_code);
            break;
        }
        
        uint32_t block_num = file->blocks[block_index];
        if (block_num >= fs->total_blocks) {
            error_code = ERR_FILE_CORRUPTED;
            HANDLE_ERROR(error_code);
            break;
        }
        
        uint8_t* block_data = fs->data_blocks + (block_num * BLOCK_SIZE);
        
        memcpy(buffer + bytes_read, block_data + block_offset, bytes_to_read);
        
        bytes_read += bytes_to_read;
        current_offset += bytes_to_read;
    }
    
    return bytes_read;
}

/* Write data to a file */
int32_t fs_write_file(FileSystem* fs, uint32_t file_index, const uint8_t* data, uint32_t size, uint32_t offset) {
    /* Grow file if needed by allocating additional blocks, then write per-block. */
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs || !data) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    if (size == 0) {
        return 0;  /* Nothing to write */
    }
    
    if (file_index >= MAX_FILES_PER_DIR * 2 || !fs->files[file_index].used) {
        error_code = ERR_INVALID_FILE_HANDLE;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    File* file = &fs->files[file_index];
    if (file->type != FILE_TYPE_FILE) {
        error_code = ERR_NOT_A_FILE;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Calculate required blocks */
    uint32_t required_size = offset + size;
    uint32_t required_blocks = (required_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    /* Check maximum file size */
    if (required_blocks > MAX_BLOCKS_PER_FILE) {
        error_code = ERR_FILE_TOO_LARGE;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Allocate more blocks if needed */
    if (required_blocks > file->block_count) {
        uint32_t additional_blocks = required_blocks - file->block_count;
        
        int result = allocate_blocks(fs, file->blocks + file->block_count, additional_blocks);
        if (result != FS_SUCCESS) {
            error_code = ERR_OUT_OF_SPACE;
            HANDLE_ERROR(error_code);
            return error_code;
        }
        file->block_count += additional_blocks;
    }
    
    /* Write data block by block */
    uint32_t bytes_written = 0;
    uint32_t current_offset = offset;
    
    while (bytes_written < size) {
        uint32_t block_index = current_offset / BLOCK_SIZE;
        uint32_t block_offset = current_offset % BLOCK_SIZE;
        uint32_t bytes_in_block = BLOCK_SIZE - block_offset;
        uint32_t bytes_to_write = (size - bytes_written) < bytes_in_block ? 
                                 (size - bytes_written) : bytes_in_block;
        
        if (block_index >= file->block_count) {
            error_code = ERR_FILE_CORRUPTED;
            HANDLE_ERROR(error_code);
            break;
        }
        
        uint32_t block_num = file->blocks[block_index];
        if (block_num >= fs->total_blocks) {
            error_code = ERR_FILE_CORRUPTED;
            HANDLE_ERROR(error_code);
            break;
        }
        
        uint8_t* block_data = fs->data_blocks + (block_num * BLOCK_SIZE);
        
        memcpy(block_data + block_offset, data + bytes_written, bytes_to_write);
        
        bytes_written += bytes_to_write;
        current_offset += bytes_to_write;
    }
    
    /* Update file size */
    if (current_offset > file->size) {
        file->size = current_offset;
    }
    
    return bytes_written;
}

/* Delete a file or directory */
int32_t fs_delete(FileSystem* fs, uint32_t file_index) {
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    if (file_index >= MAX_FILES_PER_DIR * 2 || !fs->files[file_index].used) {
        error_code = ERR_INVALID_FILE_HANDLE;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    File* file = &fs->files[file_index];
    
    /* Check if directory is empty (for directories) */
    if (file->type == FILE_TYPE_DIRECTORY) {
        for (uint32_t i = 0; i < MAX_FILES_PER_DIR * 2; i++) {
            if (fs->files[i].used && fs->files[i].parent_dir == file_index) {
                error_code = ERR_DIRECTORY_NOT_EMPTY;
                HANDLE_ERROR(error_code);
                return error_code;
            }
        }
    }
    
    /* Free data blocks */
    free_blocks(fs, file->blocks, file->block_count);
    
    /* Mark entry as unused */
    file->used = 0;
    fs->file_count--;
    
    return ERR_SUCCESS;
}

/* Find a file by name */
int32_t fs_find_file(FileSystem* fs, const char* name, uint32_t parent_dir) {
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs || !name) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Validate parent directory exists */
    if (parent_dir != 0) {  /* Root directory is always valid */
        if (parent_dir >= MAX_FILES_PER_DIR * 2 || !fs->files[parent_dir].used) {
            error_code = ERR_INVALID_DIRECTORY;
            HANDLE_ERROR(error_code);
            return error_code;
        }
        if (fs->files[parent_dir].type != FILE_TYPE_DIRECTORY) {
            error_code = ERR_NOT_A_DIRECTORY;
            HANDLE_ERROR(error_code);
            return error_code;
        }
    }
    
    for (uint32_t i = 0; i < MAX_FILES_PER_DIR * 2; i++) {
        if (fs->files[i].used && 
            fs->files[i].parent_dir == parent_dir &&
            strcmp(fs->files[i].name, name) == 0) {
            return i;
        }
    }
    
    return ERR_FILE_NOT_FOUND;
}

/* Get file information */
int32_t fs_get_file_info(FileSystem* fs, uint32_t file_index, File* info) {
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs || !info) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    if (file_index >= MAX_FILES_PER_DIR * 2 || !fs->files[file_index].used) {
        error_code = ERR_INVALID_FILE_HANDLE;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    memcpy(info, &fs->files[file_index], sizeof(File));
    return ERR_SUCCESS;
}

/* List directory contents */
int32_t fs_list_directory(FileSystem* fs, uint32_t dir_index, File* entries, uint32_t max_entries) {
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs || !entries) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    if (max_entries == 0) {
        return 0;  /* Nothing to list */
    }
    
    if (dir_index >= MAX_FILES_PER_DIR * 2 || !fs->files[dir_index].used) {
        error_code = ERR_INVALID_DIRECTORY;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    if (fs->files[dir_index].type != FILE_TYPE_DIRECTORY) {
        error_code = ERR_NOT_A_DIRECTORY;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_FILES_PER_DIR * 2 && count < max_entries; i++) {
        if (fs->files[i].used && fs->files[i].parent_dir == dir_index) {
            memcpy(&entries[count], &fs->files[i], sizeof(File));
            count++;
        }
    }
    
    return count;
}

/* Format file system (clear all files) */
int32_t fs_format(FileSystem* fs) {
    int32_t error_code = ERR_SUCCESS;
    
    /* Validate parameters */
    if (!fs) {
        error_code = ERR_NULL_POINTER;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    /* Clear all file entries */
    memset(fs->files, 0, sizeof(fs->files));
    fs->file_count = 0;
    fs->next_free_block = 0;
    
    /* Create root directory */
    int result = fs_create_directory(fs, "/", 0);
    if (result < 0) {
        error_code = ERR_FILE_SYSTEM_INIT_FAILED;
        HANDLE_ERROR(error_code);
        return error_code;
    }
    
    return ERR_SUCCESS;
}

/* Get error string for error code */
const char* fs_error_string(int32_t error_code) {
    switch (error_code) {
        case ERR_SUCCESS:               return "Success";
        case ERR_NULL_POINTER:          return "Null pointer parameter";
        case ERR_OUT_OF_MEMORY:         return "Out of memory";
        case ERR_INVALID_PARAMETER:     return "Invalid parameter";
        case ERR_FILE_NOT_FOUND:        return "File not found";
        case ERR_FILE_EXISTS:           return "File already exists";
        case ERR_FILE_NAME_TOO_LONG:    return "File name too long";
        case ERR_FILE_TOO_LARGE:        return "File too large";
        case ERR_FILE_CORRUPTED:        return "File corrupted";
        case ERR_INVALID_FILE_HANDLE:   return "Invalid file handle";
        case ERR_NOT_A_FILE:            return "Not a file";
        case ERR_NOT_A_DIRECTORY:       return "Not a directory";
        case ERR_INVALID_DIRECTORY:     return "Invalid directory";
        case ERR_DIRECTORY_NOT_EMPTY:   return "Directory not empty";
        case ERR_FILE_SYSTEM_FULL:      return "File system full";
        case ERR_OUT_OF_SPACE:          return "No space left";
        case ERR_FILE_SYSTEM_INIT_FAILED: return "File system initialization failed";
        
        default:                        return "Unknown file system error";
    }
}