#include <fstream>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
extern "C"
{
#include <wasp-lang/wasp_ast_expr.h>
#include <wasp-lang/wasp_compiler.h>
}

int main(int argc, char** argv)
{
    std::string text;

    {
        std::ifstream file("test.wasp");
        if (!file.is_open())
            return -1;

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        text = std::string(size, ' ');
        file.seekg(0);
        file.read(&text[0], size);
    }

    wasp_compiler compiler;
    wasp_create_compiler(&compiler, (unsigned char*)text.data(), text.size());

    if (wasp_compiler_set_allocation_jmp(&compiler))
    {
        wasp_ast_expr* expr;
        wasp_compiler_parse_expression_block(&compiler, &expr);
        wasp_ast_print_expr(expr, 0);
        // wasp_ast_print_expr(expr_node, 0);
    }

    return 0;

    /* // wasp_vm vm;

     // wasp_vm_initialize(&vm);

     // wasp_compiler_compile_from_file(&vm, "./test/main.wasp");

     // wasp_vm_execute(&vm, "Progam", "main");

     // wasp_vm_terminate(&vm);

     bool result = false;

     // Open file
     FILE *file = fopen("../../test/test3.wasp", "r");
     if (file == NULL)
         return false;

     // Read file size
     fseek(file, 0, SEEK_END);
     size_t length = ftell(file);
     fseek(file, 0, SEEK_SET);

     // Allocate memory
     uint8_t *buffer = (uint8_t *)malloc(length);
     if (buffer == NULL)
         goto end;

     // Read file
     if (fread(buffer, 1, length, file) != length)
         goto end;

     // struct wasp_compiler compiler;
     // memset(&compiler, 0, sizeof(wasp_compiler));

     // // compiler.vm = vm;

     // // Initialize parser
     // compiler.code_start = (char *)buffer;
     // compiler.code_end = (char *)(buffer + length);
     // compiler.code_position = compiler.code_start;

     // printf("|---------------------------- NEW RUN ----------------------------|\n");

     // Parse tokens
     size_t token_count = 1;

     // wasp_lexer_next_unlocked(&compiler);
     // wasp_lexer_next_unlocked(&compiler);
     // while (wasp_lexer_next_unlocked(&compiler))
     // {
     //     token_count++;
     // }

     uint8_t *position = buffer;
     uint8_t *position_end = buffer + length;

     clock_t t1 = clock();

     wasp_token_type token;
     while ((token = wasp_next_token(&position, position_end)) & ~0x1)
     {
         // printf("token: %s\n", wasp_get_token_name(token));
         token_count++;
     }

     clock_t t2 = clock();
     double elapsed_time = (t2 - t1) / (double)CLOCKS_PER_SEC;

     printf("%lf sec\n", elapsed_time);
     printf("Parsed tokens: %zu\n", token_count);

 end:
     fclose(file);
     free(buffer);
     return result;*/
}