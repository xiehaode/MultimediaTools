/*
 * converter_wrapper.c - Native fallback implementation
 * 注意：该实现为最小可编译版本，用于解决缺失文件导致的编译失败。
 */
#include "converter_wrapper.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>


static char* g_error_buffer = NULL;

static int stricmp_local(const char* a, const char* b) {
    unsigned char ca, cb;
    if (!a || !b) return (a == b) ? 0 : (a ? 1 : -1);
    while (*a && *b) {
        ca = (unsigned char)(*a);
        cb = (unsigned char)(*b);
        if (ca >= 'A' && ca <= 'Z') ca = (unsigned char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (unsigned char)(cb - 'A' + 'a');
        if (ca != cb) return (int)ca - (int)cb;
        ++a; ++b;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static void set_error(const char* msg) {
    if (g_error_buffer) {
        free(g_error_buffer);
        g_error_buffer = NULL;
    }
    if (msg) {
        size_t len = strlen(msg);
        g_error_buffer = (char*)malloc(len + 1);
        if (g_error_buffer) {
            memcpy(g_error_buffer, msg, len + 1);
        }
    }
}

int convert_file(const char* input_path, const char* output_path) {
    if (!input_path || !output_path) {
        set_error("Invalid input parameters");
        return 0;
    }
    set_error("Native converter not implemented in this build");
    return 0;
}

int convert_files_batch(const char* const* input_paths, int file_count,
                        const char* output_dir, const char* output_format) {
    (void)input_paths;
    (void)file_count;
    (void)output_dir;
    (void)output_format;
    set_error("Batch conversion not implemented in this build");
    return 0;
}

int check_file_support(const char* file_path) {
    if (!file_path) {
        set_error("Invalid file path");
        return 0;
    }
    // 仅支持 .pdf / .docx 的简单判断
    const char* ext = strrchr(file_path, '.');
    if (!ext) return 0;
    if (stricmp_local(ext, ".pdf") == 0 || stricmp_local(ext, ".docx") == 0) {
        return 1;
    }
    return 0;
}

const char* get_supported_formats() {
    return "{\"pdf\": \"docx\", \"docx\": \"pdf\"}";
}

const char* get_file_type(const char* file_path) {
    if (!file_path) return "unknown";
    const char* ext = strrchr(file_path, '.');
    if (!ext) return "unknown";
    if (stricmp_local(ext, ".pdf") == 0) return "pdf";
    if (stricmp_local(ext, ".docx") == 0) return "docx";
    return "unknown";
}

char* get_last_error() {
    return g_error_buffer ? g_error_buffer : "";
}

void clear_error() {
    if (g_error_buffer) {
        free(g_error_buffer);
        g_error_buffer = NULL;
    }
}
