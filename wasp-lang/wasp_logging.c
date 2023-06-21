#include "wasp_logging.h"
#include "wasp_common.h"

void wasp_create_default_logger(wasp_logger* logger)
{
    assert(logger != NULL);
    logger->info = &default_log_info;
    logger->warn = &default_log_warn;
    logger->err = &default_log_err;
}
void wasp_create_logger(wasp_logger* logger, log_function_ptr info_function_ptr, log_function_ptr warn_function_ptr,
                        log_function_ptr err_function_ptr)
{
    assert(logger != NULL);

    logger->info = info_function_ptr != NULL ? info_function_ptr : &default_log_info;
    logger->warn = warn_function_ptr != NULL ? warn_function_ptr : &default_log_warn;
    logger->err = err_function_ptr != NULL ? err_function_ptr : &default_log_err;
}

void default_log_info(const char* message)
{
    fputs("info: ", stderr);
    fputs(message, stderr);
    fputc('\n', stderr);
}
void default_log_warn(const char* message)
{
    fputs("warn: ", stderr);
    fputs(message, stderr);
    fputc('\n', stderr);
}
void default_log_err(const char* message)
{
    fputs("err : ", stderr);
    fputs(message, stderr);
    fputc('\n', stderr);
}