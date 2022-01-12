#pragma once

#include "defines.h"

// Holds a handle to a file
typedef struct file_handle
{
    // Opaque handle to internal file handle.
    void *handle;
    b8 is_valid;
} file_handle;

typedef enum file_modes
{
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
} file_modes;

/**
 * @brief Checks if a file with the given path exists.
 * 
 * @param path The path of the file to be checked. 
 * @return True if exists, otherwise false.
 */
MAPI b8 filesystem_exists(const char *path);

/**
 * @brief Attempts to open file located at path.
 * 
 * @param path The path of the file to be opened.
 * @param mode Mode flags for the file when opened (read / write). See file_modes enum in filesystem.
 * @param binary Indicates if the file should be opened in binary ode
 * @param out_handle A pointer to the file_handle structure which holds the handle information.
 * @return True if opened succesfully, otherwise false.
 */
MAPI b8 filesystem_open(const char *path, file_modes mode, b8 binary, file_handle *out_handle);

/**
 * @brief Closes the provided handle to a file.
 * 
 * @param handle A pointer to the file_handle structure to close.
 */
MAPI void filesystem_close(file_handle *handle);

/**
* Reads up to a newline or EOF
* @param handle A pointer to a file_handle structure.
* @param max_length The maximum length to be read from the line.
* @param line_buf A pointer to the character array populated by this method, must be already allocated.
* @param out_line_length A pointer to hold the line length read from the file.
* @returns True if successful; otherwise false.
*/
MAPI b8 filesystem_read_line(file_handle *handle, u64 max_length, char **line_buf, u64 *out_line_length);

/**
 * @brief Writes text to the provided file, appending a '\n' afterward.
 * 
 * @param handle A pointer to the file_handle structure.
 * @param text The text to be written.
 * @return True if successful; otherwise false.
 */
MAPI b8 filesystem_write_line(file_handle *handle, const char *text);

/**
 * @brief Reads up to data_size bytes of data into out_bytes_read.
 * Allocates *out_data, which must be freed by the caller.
 * 
 * @param handle A pointer to the file_handle structure.
 * @param data_size The number of bytes to be read.
 * @param out_data A pointer to the block of memory to be populated.
 * @param out_bytes_read A pointer to a number which will be populated by the number of bytes read.
 * @return True if successful; otherwise false.
 */
MAPI b8 filesystem_read(file_handle *handle, u64 data_size, void *out_data, u64 *out_bytes_read);

/**
 * @brief Reads all bytes of data into out_bytes_read.
 * Allocates **out_bytes, which must be freed by the caller.
 * 
 * @param handle A pointer to the file_handle structure.
 * @param out_bytes A pointer to the byte array which will be allocated and populated by this method.
 * @param out_bytes_read A pointer to the numer which will be poulated by the number of bytes read.
 * @return True if successful; otherwise false.
 */
MAPI b8 filesystem_read_all_bytes(file_handle *handle, u8 **out_bytes, u64 *out_bytes_read);

/**
 * @brief Writes up to data_size bytes of data out of *data.
 * 
 * @param handle A pointer to the file_handle structure.
 * @param data_size The number of bytes to be read.
 * @param data A pointer to the block of memory to be read from.
 * @param out_bytes_written A pointer to a number which will be populated by the number of bytes written.
 * @return True if successful; otherwise false.
 */
MAPI b8 filesystem_write(file_handle *handle, u64 data_size, const void *data, u64 *out_bytes_written);