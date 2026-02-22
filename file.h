// // #ifndef File
// // define File
// typedef struct file
// {
//     FILE *Fptr;
// }file;
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef struct node
{
    int top ;
    char ch ;
    struct node *link ;
}stack_t ;


// Use this macros to check prev buffer (if int is keyword and next any variable is there and it should't start with digit so to check this we need the prev value of buffer)
#define VARIBLE       1
#define CONSTANT      2
#define STRING_LITRAL 3
#define KEYWORD       4
 
int check_tokens(FILE *fptr1);
FILE * validate_file(char *filename);
int  is_keyword(char * ch);
int is_floatConstant(char buffer[]) ;
int is_char_constant(FILE *fptr , char ch ) ;
int is_intconstant(char buffer[]);
void check_type(char *str) ;
int is_identifier(char *str);
int is_string_litral(FILE *fptr , char  ch) ;
int is_operator(FILE *fptr , char ch) ;
int is_binary(char *buffer) ;
int is_hexadecimal(char buffer[]) ;
int is_octal(char str[]);

// int push(stack_t *top ,char ch) ;
// int pop(stack_t *top);
int check_parenthesis(FILE *fptr);

int push(stack_t **top ,char ch) ;
int pop(stack_t **top);

