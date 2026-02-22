#include "file.h"

/* External declarations shared across lexer modules */
extern char keywords[32][10];     // List of C keywords
extern char operators[7];        // Supported operators
extern int semicolon_expected;   // Flag to detect missing semicolons
extern int line_count;           // Current line number for error reporting

/*----------------------------------------------------------
 * Checks whether the given string is a C keyword
 *----------------------------------------------------------*/
int is_keyword(char buffer[])
{
   for (int i = 0; i < 32; i++)
   {
      if (!strcmp(buffer, keywords[i]))
      {
         return 1;   // Match found → keyword
      }
   }
   return 0;          // Not a keyword
}

/*----------------------------------------------------------
 * Checks whether the string is a valid integer constant
 * (Only decimal digits allowed)
 *----------------------------------------------------------*/
int is_intconstant(char buffer[])
{
   int j = 0;
   while (buffer[j] != '\0')
   {
      if (!isdigit(buffer[j]))
         return 0;    // Non-digit found → invalid integer
      j++;
   }

   semicolon_expected = 1;  // Constants must be terminated
   return 1;
}

/*----------------------------------------------------------
 * Validates character constants: 'a'
 * Detects empty ('') and multi-character ('ab') errors
 *----------------------------------------------------------*/
int is_char_constant(FILE *fptr, char ch)
{
   char buff[10];
   int j = 0;

   buff[j++] = ch;          // Store opening quote

   ch = fgetc(fptr);        // Read character inside quotes
   if (ch == '\'')
   {
      // Empty character constant
      printf("\033[31mError : empty character constant -> on line %d \033[0m\n", line_count);
      exit(1);
   }

   buff[j++] = ch;          // Store actual character

   ch = fgetc(fptr);        // Read closing quote
   if (ch == '\'')
   {
      buff[j++] = ch;
      buff[j++] = '\0';
      printf("\033[36mCHAR CONSTANT :\033[0m %s\n", buff);
   }
   else
   {
      // More than one character inside quotes
      printf("\033[31mError : multi-character character constant -> on line %d \033[0m\n", line_count);
      exit(0);
   }

   semicolon_expected = 1;
   return 1;
}

/*----------------------------------------------------------
 * Checks floating-point constants
 * Rules enforced:
 *  - Exactly one decimal point
 *  - Optional 'f' suffix
 *----------------------------------------------------------*/
int is_floatConstant(char buffer[])
{
   int dec_flag = 0;
   int i = 0;

   /* Float must contain '.' */
   if (strchr(buffer, '.'))
   {
      while (buffer[i] != '\0')
      {
         if (buffer[i] == '.')
            dec_flag++;

         else if (!isdigit(buffer[i]))
         {
            /* Invalid suffix on integer part */
            if (dec_flag == 0)
            {
               printf("\033[31mError: invalid suffix \"%s\" on integer constant -> on line %d \033[0m\n",
                      buffer + i, line_count);
               exit(1);
            }
            /* Only 'f' allowed after decimal */
            else if (buffer[i] != 'f' && dec_flag == 1)
            {
               printf("\033[31mError: invalid suffix \"%c\" on floating constant -> on line %d \033[0m\n",
                      buffer[i], line_count);
               exit(1);
            }
         }
         i++;
      }

      if (dec_flag == 1)
      {
         semicolon_expected = 1;
         return 1;
      }
      else
      {
         printf("\033[31mError: too many decimal points in number \"%s\" -> on line %d \033[0m\n",
                buffer, line_count);
         exit(1);
      }
   }
   return 0;
}

/*----------------------------------------------------------
 * Checks binary constants: 0b10101 / 0B10101
 *----------------------------------------------------------*/
int is_binary(char *buffer)
{
   char temp[4] = {0};

   temp[0] = buffer[0];
   temp[1] = buffer[1];
   temp[2] = '\0';

   if (!strcmp(temp, "0b") || !strcmp(temp, "0B"))
   {
      int i = 2;
      while (buffer[i] != '\0')
      {
         if (buffer[i] != '0' && buffer[i] != '1')
         {
            printf("\033[31mError : invalid digit \"%c\" in binary constant -> on line %d \033[0m\n",
                   buffer[i], line_count);
            exit(1);
         }
         i++;
      }
      return 1;
   }
   return 0;
}

/*----------------------------------------------------------
 * Checks hexadecimal constants: 0xA3F / 0X1B
 *----------------------------------------------------------*/
int is_hexadecimal(char buffer[])
{
   char temp[4] = {0};

   temp[0] = buffer[0];
   temp[1] = buffer[1];
   temp[2] = '\0';

   if (!strcmp(temp, "0x") || !strcmp(temp, "0X"))
   {
      int i = 2;
      while (buffer[i] != '\0')
      {
         if (!isxdigit(buffer[i]))
         {
            printf("\033[31mError : invalid digit \"%c\" in hexadecimal constant -> on line %d \033[0m\n",
                   buffer[i], line_count);
            exit(1);
         }
         i++;
      }
      semicolon_expected = 1;
      return 1;
   }
   return 0;
}

/*----------------------------------------------------------
 * Checks octal constants: 0754
 *----------------------------------------------------------*/
int is_octal(char buffer[])
{
   int i = 0;

   if (buffer[i] == '0')
   {
      i = 1;
      while (buffer[i] != '\0')
      {
         if (buffer[i] < '0' || buffer[i] > '7')
         {
            printf("\033[31mError : invalid digit \"%c\" in octal constant -> on line %d \033[0m\n",
                   buffer[i], line_count);
            exit(1);
         }
         i++;
      }
      semicolon_expected = 1;
      return 1;
   }
   return 0;
}

/*----------------------------------------------------------
 * Checks valid identifiers
 * Rules:
 *  - First character: letter or '_'
 *  - Remaining: letters or '_'
 *----------------------------------------------------------*/
int is_identifier(char *buffer)
{
   int i = 0;

   if (isalpha(buffer[i]) || buffer[i] == '_')
   {
      i++;
      while (buffer[i] != '\0')
      {
         if (isalpha(buffer[i]) || buffer[i] == '_')
            i++;
         else
            return 0;
      }
   }
   else
      return 0;

   semicolon_expected = 1;
   return 1;
}

/*----------------------------------------------------------
 * Parses string literals: "hello world"
 * Detects missing closing quote
 *----------------------------------------------------------*/
int is_string_litral(FILE *fptr, char ch)
{
   char buffer[100];
   int i = 0;

   buffer[i++] = ch;    // Store opening quote

   while ((ch = fgetc(fptr)) != '"')
   {
      if (ch == EOF || ch == '\n')
      {
         printf("\033[31mError : missing terminating \" character -> on line %d \033[0m\n", line_count);
         exit(1);
      }
      buffer[i++] = ch;
   }

   buffer[i++] = '"';
   buffer[i++] = '\0';

   printf("\033[32mString Literal :\033[0m %s\n", buffer);
   semicolon_expected = 1;
   return 1;
}

/*----------------------------------------------------------
 * Operator detection
 * Supports single and double character operators
 *----------------------------------------------------------*/
int is_operator(FILE *fptr, char ch)
{
   char ch1;

   for (int i = 0; i < 7; i++)
   {
      if (ch == operators[i])
      {
         ch1 = fgetc(fptr);

         /* Check for two-character operators */
         for (int j = 0; j < 7; j++)
         {
            if (ch1 == operators[j])
            {
               printf("\033[35mOPERATOR :\033[0m %c%c\n", ch, ch1);
               return 1;
            }
         }

         /* Single-character operator */
         printf("\033[35mOPERATOR :\033[0m %c\n", ch);
         ungetc(ch1, fptr);   // Restore lookahead character
         return 1;
      }
   }

   semicolon_expected = 1;
   return 0;
}

/* Ternary operator check can be added here */