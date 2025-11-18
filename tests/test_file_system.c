/* test_file_system.c - Unit tests for file system functions */

#include "unity.h"
#include "test_config.h"
#include "../src/error_codes.h"

/* Mock file system structures and functions */
typedef struct {
    char name[32];
    uint32_t size;
    uint8_t type;
    uint32_t blocks[8];
    uint32_t block_count;
    uint32_t parent_dir;
    uint8_t used;
} MockFile;

typedef struct {
    MockFile files[32];
    uint32_t file_count;
    uint32_t next_free_block;
    uint8_t* data_blocks;
    uint32_t total_blocks;
} MockFileSystem;

static MockFileSystem mock_fs;
static uint8_t mock_data_blocks[TEST_FS_SIZE];

/* Mock file system constants */
#define MOCK_FILE_TYPE_FILE      0
#define MOCK_FILE_TYPE_DIRECTORY 1
#define MOCK_MAX_FILENAME_LENGTH 32
#define MOCK_MAX_FILES_PER_DIR   16
#define MOCK_BLOCK_SIZE          512
#define MOCK_MAX_BLOCKS_PER_FILE 8

/* Mock file system functions */
int32_t mock_fs_init(MockFileSystem* fs, uint8_t* data_memory, uint32_t memory_size) {
    if (!fs || !data_memory || memory_size < MOCK_BLOCK_SIZE) {
        return ERR_NULL_POINTER;
    }
    
    memset(fs->files, 0, sizeof(fs->files));
    fs->file_count = 0;
    fs->data_blocks = data_memory;
    fs->total_blocks = memory_size / MOCK_BLOCK_SIZE;
    fs->next_free_block = 0;
    
    if (fs->total_blocks == 0) {
        return ERR_OUT_OF_MEMORY;
    }
    
    /* Create root directory */
    int result = mock_fs_create_directory(fs, "/", 0);
    return result >= 0 ? ERR_SUCCESS : result;
}

int32_t mock_fs_create_file(MockFileSystem* fs, const char* name, uint32_t parent_dir) {
    if (!fs || !name) {
        return ERR_NULL_POINTER;
    }
    
    if (strlen(name) >= MOCK_MAX_FILENAME_LENGTH) {
        return ERR_FILE_NAME_TOO_LONG;
    }
    
    /* Check if file already exists */
    for (uint32_t i = 0; i < MOCK_MAX_FILES_PER_DIR * 2; i++) {
        if (fs->files[i].used && 
            fs->files[i].parent_dir == parent_dir &&
            strcmp(fs->files[i].name, name) == 0) {
            return ERR_FILE_EXISTS;
        }
    }
    
    /* Find free entry */
    int entry_index = -1;
    for (uint32_t i = 0; i < MOCK_MAX_FILES_PER_DIR * 2; i++) {
        if (!fs->files[i].used) {
            entry_index = i;
            break;
        }
    }
    
    if (entry_index < 0) {
        return ERR_FILE_SYSTEM_FULL;
    }
    
    /* Initialize file entry */
    MockFile* file = &fs->files[entry_index];
    memset(file, 0, sizeof(MockFile));
    strcpy(file->name, name);
    file->type = MOCK_FILE_TYPE_FILE;
    file->size = 0;
    file->block_count = 0;
    file->parent_dir = parent_dir;
    file->used = 1;
    
    fs->file_count++;
    return entry_index;
}

int32_t mock_fs_create_directory(MockFileSystem* fs, const char* name, uint32_t parent_dir) {
    if (!fs || !name) {
        return ERR_NULL_POINTER;
    }
    
    if (strlen(name) >= MOCK_MAX_FILENAME_LENGTH) {
        return ERR_FILE_NAME_TOO_LONG;
    }
    
    /* Check if directory already exists */
    for (uint32_t i = 0; i < MOCK_MAX_FILES_PER_DIR * 2; i++) {
        if (fs->files[i].used && 
            fs->files[i].parent_dir == parent_dir &&
            strcmp(fs->files[i].name, name) == 0) {
            return ERR_FILE_EXISTS;
        }
    }
    
    /* Find free entry */
    int entry_index = -1;
    for (uint32_t i = 0; i < MOCK_MAX_FILES_PER_DIR * 2; i++) {
        if (!fs->files[i].used) {
            entry_index = i;
            break;
        }
    }
    
    if (entry_index < 0) {
        return ERR_FILE_SYSTEM_FULL;
    }
    
    /* Initialize directory entry */
    MockFile* dir = &fs->files[entry_index];
    memset(dir, 0, sizeof(MockFile));
    strcpy(dir->name, name);
    dir->type = MOCK_FILE_TYPE_DIRECTORY;
    dir->size = 0;
    dir->block_count = 0;
    dir->parent_dir = parent_dir;
    dir->used = 1;
    
    fs->file_count++;
    return entry_index;
}

int32_t mock_fs_write_file(MockFileSystem* fs, uint32_t file_index, const uint8_t* data, uint32_t size, uint32_t offset) {
    if (!fs || !data) {
        return ERR_NULL_POINTER;
    }
    
    if (size == 0) {
        return 0;
    }
    
    if (file_index >= MOCK_MAX_FILES_PER_DIR * 2 || !fs->files[file_index].used) {
        return ERR_INVALID_FILE_HANDLE;
    }
    
    MockFile* file = &fs->files[file_index];
    if (file->type != MOCK_FILE_TYPE_FILE) {
        return ERR_NOT_A_FILE;
    }
    
    /* Calculate required blocks */
    uint32_t required_size = offset + size;
    uint32_t required_blocks = (required_size + MOCK_BLOCK_SIZE - 1) / MOCK_BLOCK_SIZE;
    
    /* Check maximum file size */
    if (required_blocks > MOCK_MAX_BLOCKS_PER_FILE) {
        return ERR_FILE_TOO_LARGE;
    }
    
    /* Allocate more blocks if needed */
    if (required_blocks > file->block_count) {
        uint32_t additional_blocks = required_blocks - file->block_count;
        
        if (fs->next_free_block + additional_blocks > fs->total_blocks) {
            return ERR_OUT_OF_SPACE;
        }
        
        for (uint32_t i = 0; i < additional_blocks; i++) {
            file->blocks[file->block_count + i] = fs->next_free_block++;
        }
        file->block_count += additional_blocks;
    }
    
    /* Write data */
    uint32_t bytes_written = 0;
    uint32_t current_offset = offset;
    
    while (bytes_written < size) {
        uint32_t block_index = current_offset / MOCK_BLOCK_SIZE;
        uint32_t block_offset = current_offset % MOCK_BLOCK_SIZE;
        uint32_t bytes_in_block = MOCK_BLOCK_SIZE - block_offset;
        uint32_t bytes_to_write = (size - bytes_written) < bytes_in_block ? 
                                 (size - bytes_written) : bytes_in_block;
        
        if (block_index >= file->block_count) {
            break;
        }
        
        uint32_t block_num = file->blocks[block_index];
        if (block_num >= fs->total_blocks) {
            return ERR_FILE_CORRUPTED;
        }
        
        uint8_t* block_data = fs->data_blocks + (block_num * MOCK_BLOCK_SIZE);
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

int32_t mock_fs_read_file(MockFileSystem* fs, uint32_t file_index, uint8_t* buffer, uint32_t size, uint32_t offset) {
    if (!fs || !buffer) {
        return ERR_NULL_POINTER;
    }
    
    if (size == 0) {
        return 0;
    }
    
    if (file_index >= MOCK_MAX_FILES_PER_DIR * 2 || !fs->files[file_index].used) {
        return ERR_INVALID_FILE_HANDLE;
    }
    
    MockFile* file = &fs->files[file_index];
    if (file->type != MOCK_FILE_TYPE_FILE) {
        return ERR_NOT_A_FILE;
    }
    
    /* Check bounds */
    if (offset >= file->size) {
        return 0;  /* Nothing to read */
    }
    
    uint32_t read_size = size;
    if (offset + size > file->size) {
        read_size = file->size - offset;
    }
    
    /* Read data */
    uint32_t bytes_read = 0;
    uint32_t current_offset = offset;
    
    while (bytes_read < read_size && current_offset < file->size) {
        uint32_t block_index = current_offset / MOCK_BLOCK_SIZE;
        uint32_t block_offset = current_offset % MOCK_BLOCK_SIZE;
        uint32_t bytes_in_block = MOCK_BLOCK_SIZE - block_offset;
        uint32_t bytes_to_read = (read_size - bytes_read) < bytes_in_block ? 
                                (read_size - bytes_read) : bytes_in_block;
        
        if (block_index >= file->block_count) {
            return ERR_FILE_CORRUPTED;
        }
        
        uint32_t block_num = file->blocks[block_index];
        if (block_num >= fs->total_blocks) {
            return ERR_FILE_CORRUPTED;
        }
        
        uint8_t* block_data = fs->data_blocks + (block_num * MOCK_BLOCK_SIZE);
        memcpy(buffer + bytes_read, block_data + block_offset, bytes_to_read);
        
        bytes_read += bytes_to_read;
        current_offset += bytes_to_read;
    }
    
    return bytes_read;
}

int32_t mock_fs_delete(MockFileSystem* fs, uint32_t file_index) {
    if (!fs) {
        return ERR_NULL_POINTER;
    }
    
    if (file_index >= MOCK_MAX_FILES_PER_DIR * 2 || !fs->files[file_index].used) {
        return ERR_INVALID_FILE_HANDLE;
    }
    
    MockFile* file = &fs->files[file_index];
    
    /* Check if directory is empty (for directories) */
    if (file->type == MOCK_FILE_TYPE_DIRECTORY) {
        for (uint32_t i = 0; i < MOCK_MAX_FILES_PER_DIR * 2; i++) {
            if (fs->files[i].used && fs->files[i].parent_dir == file_index) {
                return ERR_DIRECTORY_NOT_EMPTY;
            }
        }
    }
    
    /* Mark entry as unused */
    file->used = 0;
    fs->file_count--;
    
    return ERR_SUCCESS;
}

/* Test setup and teardown */
void setUp(void) {
    memset(&mock_fs, 0, sizeof(mock_fs));
    memset(mock_data_blocks, 0, sizeof(mock_data_blocks));
}

void tearDown(void) {
    /* Cleanup after each test */
}

/* Test cases for file system */
void test_file_system_initialization_success(void) {
    int32_t result = mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    TEST_ASSERT_EQUAL(ERR_SUCCESS, result);
    TEST_ASSERT_EQUAL(1, mock_fs.file_count); /* Root directory */
    TEST_ASSERT_EQUAL(8, mock_fs.total_blocks); /* 4096 / 512 */
}

void test_file_system_initialization_null_fs(void) {
    int32_t result = mock_fs_init(NULL, mock_data_blocks, sizeof(mock_data_blocks));
    TEST_ASSERT_EQUAL(ERR_NULL_POINTER, result);
}

void test_file_system_initialization_null_memory(void) {
    int32_t result = mock_fs_init(&mock_fs, NULL, sizeof(mock_data_blocks));
    TEST_ASSERT_EQUAL(ERR_NULL_POINTER, result);
}

void test_file_system_initialization_insufficient_memory(void) {
    int32_t result = mock_fs_init(&mock_fs, mock_data_blocks, 100); /* Less than block size */
    TEST_ASSERT_EQUAL(ERR_OUT_OF_MEMORY, result);
}

void test_create_file_success(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    int32_t result = mock_fs_create_file(&mock_fs, "test.txt", 0);
    TEST_ASSERT_TRUE(result >= 0);
    TEST_ASSERT_EQUAL(2, mock_fs.file_count); /* Root + new file */
}

void test_create_file_null_fs(void) {
    int32_t result = mock_fs_create_file(NULL, "test.txt", 0);
    TEST_ASSERT_EQUAL(ERR_NULL_POINTER, result);
}

void test_create_file_null_name(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    int32_t result = mock_fs_create_file(&mock_fs, NULL, 0);
    TEST_ASSERT_EQUAL(ERR_NULL_POINTER, result);
}

void test_create_file_name_too_long(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    char long_name[100];
    memset(long_name, 'a', sizeof(long_name) - 1);
    long_name[sizeof(long_name) - 1] = '\0';
    
    int32_t result = mock_fs_create_file(&mock_fs, long_name, 0);
    TEST_ASSERT_EQUAL(ERR_FILE_NAME_TOO_LONG, result);
}

void test_create_file_duplicate(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    int32_t result1 = mock_fs_create_file(&mock_fs, "duplicate.txt", 0);
    TEST_ASSERT_TRUE(result1 >= 0);
    
    int32_t result2 = mock_fs_create_file(&mock_fs, "duplicate.txt", 0);
    TEST_ASSERT_EQUAL(ERR_FILE_EXISTS, result2);
}

void test_write_file_success(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    int32_t file_index = mock_fs_create_file(&mock_fs, "write_test.txt", 0);
    TEST_ASSERT_TRUE(file_index >= 0);
    
    const char* test_data = "Hello, File System!";
    uint32_t data_size = strlen(test_data);
    
    int32_t bytes_written = mock_fs_write_file(&mock_fs, file_index, (const uint8_t*)test_data, data_size, 0);
    TEST_ASSERT_EQUAL(data_size, bytes_written);
}

void test_write_file_null_data(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    int32_t file_index = mock_fs_create_file(&mock_fs, "write_test.txt", 0);
    TEST_ASSERT_TRUE(file_index >= 0);
    
    int32_t result = mock_fs_write_file(&mock_fs, file_index, NULL, 100, 0);
    TEST_ASSERT_EQUAL(ERR_NULL_POINTER, result);
}

void test_write_file_invalid_handle(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    const char* test_data = "Test data";
    int32_t result = mock_fs_write_file(&mock_fs, 999, (const uint8_t*)test_data, strlen(test_data), 0);
    TEST_ASSERT_EQUAL(ERR_INVALID_FILE_HANDLE, result);
}

void test_read_write_file_round_trip(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    int32_t file_index = mock_fs_create_file(&mock_fs, "roundtrip.txt", 0);
    TEST_ASSERT_TRUE(file_index >= 0);
    
    const char* original_data = "Round trip test data!";
    uint32_t original_size = strlen(original_data);
    
    /* Write data */
    int32_t bytes_written = mock_fs_write_file(&mock_fs, file_index, (const uint8_t*)original_data, original_size, 0);
    TEST_ASSERT_EQUAL(original_size, bytes_written);
    
    /* Read data back */
    uint8_t read_buffer[100];
    int32_t bytes_read = mock_fs_read_file(&mock_fs, file_index, read_buffer, sizeof(read_buffer), 0);
    TEST_ASSERT_EQUAL(original_size, bytes_read);
    
    /* Compare data */
    TEST_ASSERT_EQUAL_MEMORY(original_data, read_buffer, original_size);
}

void test_delete_file_success(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    int32_t file_index = mock_fs_create_file(&mock_fs, "delete_me.txt", 0);
    TEST_ASSERT_TRUE(file_index >= 0);
    TEST_ASSERT_EQUAL(2, mock_fs.file_count);
    
    int32_t result = mock_fs_delete(&mock_fs, file_index);
    TEST_ASSERT_EQUAL(ERR_SUCCESS, result);
    TEST_ASSERT_EQUAL(1, mock_fs.file_count);
}

void test_delete_file_null_fs(void) {
    int32_t result = mock_fs_delete(NULL, 0);
    TEST_ASSERT_EQUAL(ERR_NULL_POINTER, result);
}

void test_delete_file_invalid_handle(void) {
    mock_fs_init(&mock_fs, mock_data_blocks, sizeof(mock_data_blocks));
    
    int32_t result = mock_fs_delete(&mock_fs, 999);
    TEST_ASSERT_EQUAL(ERR_INVALID_FILE_HANDLE, result);
}

/* Test group runner */
int run_file_system_tests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_file_system_initialization_success);
    RUN_TEST(test_file_system_initialization_null_fs);
    RUN_TEST(test_file_system_initialization_null_memory);
    RUN_TEST(test_file_system_initialization_insufficient_memory);
    RUN_TEST(test_create_file_success);
    RUN_TEST(test_create_file_null_fs);
    RUN_TEST(test_create_file_null_name);
    RUN_TEST(test_create_file_name_too_long);
    RUN_TEST(test_create_file_duplicate);
    RUN_TEST(test_write_file_success);
    RUN_TEST(test_write_file_null_data);
    RUN_TEST(test_write_file_invalid_handle);
    RUN_TEST(test_read_write_file_round_trip);
    RUN_TEST(test_delete_file_success);
    RUN_TEST(test_delete_file_null_fs);
    RUN_TEST(test_delete_file_invalid_handle);
    
    return UNITY_END();
}