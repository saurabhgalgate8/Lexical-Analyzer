/* ============================================================
 * Project Name : C Language Lexical Analyzer
 * ============================================================
 *
 * - Implements a lexical analyzer (lexer) for the C language.
 * - Reads the input .c source file character by character.
 * - Uses fgetc() and ungetc() for controlled lookahead handling.
 *
 * - Identifies and classifies the following token types:
 *   - Keywords
 *   - Identifiers
 *   - Integer constants
 *   - Floating-point constants
 *   - Binary constants (0b / 0B)
 *   - Octal constants (leading 0)
 *   - Hexadecimal constants (0x / 0X)
 *   - Character constants
 *   - String literals
 *   - Operators (single and double character)
 *   - Delimiters
 *
 * - Skips preprocessor directives such as:
 *   - #include
 *   - #define
 *
 * - Validates syntax-related constructs:
 *   - Parentheses (), brackets [], and braces {} using stack
 *   - Semicolon detection after statements
 *
 * - Performs lexical error detection with line number tracking:
 *   - Missing or mismatched parentheses
 *   - Missing semicolons
 *   - Invalid numeric constants
 *   - Unterminated string literals
 *   - Empty or multi-character character constants
 *   - Invalid identifiers and operators
 *
 * - Displays tokenized output with color-coded formatting.
 * - Maintains line count for accurate error reporting.
 * - Uses modular design with separate functions for each token type.
 *
 * - Demonstrates core compiler design concepts:
 *   - Lexical analysis
 *   - Token classification
 *   - Lookahead handling
 *   - Stack-based validation
 *
 * - Serves as a foundation for further compiler phases
 *   such as syntax analysis and semantic analysis.
 *
 * ============================================================
 */

#include "header.h"

/* List of all C language keywords */
char keywords[32][10] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

/* Supported operators */
char operators[12] = {'+', '-', '*', '/', '=', '%'};

/* Flags and counters */
int semicolon_expected = 0;   // Used to track missing semicolons
int line_count = 1;           // Keeps track of line numbers for error reporting

/* Stack pointer for parenthesis checking */
stack_t *top = NULL;

int main(int argc, char *argv[])
{
    int ret;
    FILE *fptr;

    /* Check whether input file is provided */
    if (argc < 2)
    {
        printf("\033[31mError : Input file is not present\033[0m\n");
        printf("Usage : ");
        printf("\"a.out\" \"filename\"\n");
        return 0;
    }

    /* Validate and open file */
    fptr = validate_file(argv[1]);
    if (fptr != NULL)
    {
        /* First, validate parenthesis using stack */
        ret = check_parenthesis(fptr);
        if (ret == 0)
        {
            printf("Error : Parenthesis Mis-match\n");
            return 0;
        }

        /* Perform lexical analysis */
        check_tokens(fptr);
    }

    return 0;
}

/* Global flags */
int iskeyWords_variable_flag = 0;
int ret = 0;

/* Main tokenization function */
int check_tokens(FILE *fptr)
{
    char buffer[100];      // Stores token temporarily
    int i = 0;             // Index for buffer
    char ch;
    int semicolon_expected = 0;

    while ((ch = fgetc(fptr)) != EOF)
    {
        /* Skip preprocessor directives */
        if (ch == '#')
        {
            while ((ch = fgetc(fptr)) != '\n');
            line_count++;
        }

        /* Handle newline and semicolon validation */
        else if (ch == '\n')
        {
            line_count++;
            if (semicolon_expected)
            {
                char ch1;
                if ((ch1 = fgetc(fptr)) != '{')
                {
                    printf("\033[031mError : Semicolon expected on line -> %d\033[0m\n", line_count);
                    exit(1);
                }
                else
                {
                    ungetc(ch1, fptr);
                    semicolon_expected = 0;
                }
            }
        }

        /* Collect alphanumeric tokens (identifiers/constants) */
        else if (isalnum(ch) || ch == '_' || ch == '.')
        {
            buffer[i++] = ch;
        }

        /* Token boundary encountered */
        else
        {
            if (i > 0)
            {
                buffer[i] = '\0';     // Terminate token
                check_type(buffer);   // Identify token type
                i = 0;
            }

            /* Character constant */
            if (ch == '\'')
            {
                is_char_constant(fptr, ch);
                semicolon_expected = 1;
            }

            /* String literal */
            else if (ch == '"')
            {
                is_string_litral(fptr, ch);
            }

            /* Operator */
            else if (is_operator(fptr, ch))
            {
                semicolon_expected = 1;
            }

            /* Delimiters */
            else if (!isspace(ch))
            {
                printf("\033[35m%-22s\033[0m %c\n", "Special Character:", ch);

                /* ')' must be followed by semicolon or block */
                if (ch == ')')
                    semicolon_expected = 1;
                else
                    semicolon_expected = 0;
            }

            /* Semicolon clears expectation */
            if (ch == ';')
            {
                semicolon_expected = 0;
            }
        }
    }
    return 0;
}

/* Determines the type of token */
void check_type(char *str)
{
    if (is_keyword(str))
        printf("\033[34m%-22s\033[0m %s\n", "KEYWORD :", str);

    else if (is_identifier(str))
        printf("\033[32m%-22s\033[0m %s\n", "IDENTIFIER :", str);

    else if (is_intconstant(str))
        printf("\033[33m%-22s\033[0m %s\n", "INT CONSTANT :", str);

    else if (is_floatConstant(str))
        printf("%-22s %s\n", "FLOAT CONSTANT :", str);

    else if (is_binary(str))
        printf("%-22s %s\n", "BINARY CONSTANT :", str);

    else if (is_hexadecimal(str))
        printf("%-22s %s\n", "HEXADECIMAL CONSTANT :", str);

    else if (is_octal(str))
        printf("%-22s %s\n", "OCTAL CONSTANT :", str);

    else
        printf("\033[31m%-22s\033[0m %s\n", "INVALID TOKEN :", str);
}

/* Validate file extension and open file */
FILE *validate_file(char *filename)
{
    FILE *fptr;
    char *substr;

    printf("filename : %s\n", filename);

    /* Check for .c extension */
    if ((substr = strstr(filename, ".c")))
    {
        if ((substr - filename ) != (strlen(filename)- 2))
        {
            printf("Error : File is not a .c file\n");
            return 0 ;
        }
    }

    fptr = fopen(filename, "r");
    if (fptr == NULL)
    {
        printf("Error : Unable to open the file\n");
        return NULL;
    }

    printf("File opened Successfully !\n");
    return fptr;
}

/* Parenthesis matching using stack */
int check_parenthesis(FILE *fptr)
{
    char ch;

    while ((ch = fgetc(fptr)) != EOF)
    {
        if (ch == '(' || ch == '[' || ch == '{')
        {
            push(&top, ch);
        }
        else if (ch == ')' || ch == ']' || ch == '}')
        {
            if (top == NULL)
            {
                return 0;   // Unmatched closing bracket
            }
            pop(&top);
        }
    }

    if (top != NULL)
    {
        printf("Error : Parenthesis are not matching\n");
    }
    else
    {
        rewind(fptr);   // Reset file pointer after validation
    }

    return 1;
}

/* Push element into stack */
int push(stack_t **top, char ch)
{
    stack_t *new = malloc(sizeof(stack_t));
    if (new == NULL)
    {
        printf("malloc fails\n");
        return 0;
    }

    new->ch = ch;
    new->link = *top;
    *top = new;

    return 1;
}

/* Pop element from stack */
int pop(stack_t **top)
{
    stack_t *temp = *top;
    *top = (*top)->link;
    free(temp);
    return 1;
}
