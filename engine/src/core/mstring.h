#pragma once

#include "defines.h"
#include "math/math_types.h"

// Returns the length of the given string.
MAPI u64 string_length(const char *str);

MAPI char *string_duplicate(const char *str);

// Case-sensitive string comparison. True if the same, otherwise false.
MAPI b8 strings_equal(const char *str0, const char *str1);

// Case-insensitive string comparison. True if the same, otherwise false.
MAPI b8 strings_equali(const char *str0, const char *str1);

// Performs string formatting to dest given format string and parameters
MAPI i32 string_format(char *dest, const char *format, ...);

/**
 * @brief Performs variadic string formatting to dest given format string and va_list.
 * 
 * @param dest The destination for the formatted string.
 * @param format The string to be formatted.
 * @param va_list The variadic argument list.
 * @return The size of the data written.
 */
MAPI i32 string_format_v(char *dest, const char *format, void *va_listp);

MAPI char *string_empty(char *str);

MAPI char *string_copy(char *dest, const char *source);
MAPI char *string_ncopy(char *dest, const char *source, i64 length);

MAPI char *string_trim(char *str);

MAPI void string_mid(char *dest, const char *source, i32 start, i32 length);

// Returns the index of the first occurance of c in str; otherwise -1
MAPI i32 string_index_of(char *str, char c);

MAPI b8 string_to_vec4(char *str, vec4 *out_vector);
MAPI b8 string_to_vec3(char *str, vec3 *out_vector);
MAPI b8 string_to_vec2(char *str, vec2 *out_str);

MAPI b8 string_to_f32(char *str, f32 *f);
MAPI b8 string_to_f64(char *str, f64 *f);

MAPI b8 string_to_i8(char *str, i8 *i);
MAPI b8 string_to_i16(char *str, i16 *i);
MAPI b8 string_to_i32(char *str, i32 *i);
MAPI b8 string_to_i64(char *str, i64 *i);

MAPI b8 string_to_u8(char *str, u8 *u);
MAPI b8 string_to_u16(char *str, u16 *u);
MAPI b8 string_to_u32(char *str, u32 *u);
MAPI b8 string_to_u64(char *str, u64 *u);

MAPI b8 string_to_bool(char *str, b8 *b);