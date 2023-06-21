#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/// @brief wasp string
/// 
typedef struct wasp_string
{
    unsigned char* data;
    size_t data_length;
} wasp_string;