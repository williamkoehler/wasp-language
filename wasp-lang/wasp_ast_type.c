#include "wasp_ast_type.h"

void wasp_ast_print_type(wasp_ast_type type, size_t depth)
{
    for (size_t i = 0; i < depth; i++)
        printf("   ");
    depth++;

    if (type.generics.first_item == NULL)
    {
        if (type.is_reference)
            printf("|- %.*s data type reference\n", (int)type.name.data_length, type.name.data);
        else
            printf("|- %.*s data type\n", (int)type.name.data_length, type.name.data);
    }
    else
    {
        if (type.is_reference)
            printf("|- %.*s data type reference with generics\n", (int)type.name.data_length, type.name.data);
        else
            printf("|- %.*s data type with generics\n", (int)type.name.data_length, type.name.data);

        for (wasp_ast_type_list_item* item = type.generics.first_item; item != NULL; item = item->next_item)
            wasp_ast_print_type(item->type, depth);
    }
}