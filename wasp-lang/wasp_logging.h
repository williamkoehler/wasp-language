#pragma once

typedef void (*log_function_ptr)(const char* message);

typedef struct wasp_logger
{
    log_function_ptr info;
    log_function_ptr warn;
    log_function_ptr err;
} wasp_logger;

void wasp_create_default_logger(wasp_logger* logger);
void wasp_create_logger(wasp_logger* logger, log_function_ptr info_function_ptr, log_function_ptr warn_function_ptr, log_function_ptr err_function_ptr);

void default_log_info(const char* message);
void default_log_warn(const char* message);
void default_log_err(const char* message);