/* -------------------------------------------------------------------------------------------------------------------- */
// Mini regex module - From https://github.com/kokke/tiny-regex-c
/* -------------------------------------------------------------------------------------------------------------------- */
//
// Author:      kokke
// Date:        25/10/18
//
// Edited:      Renan R. Duarte (24/01/2019)
//              Library heavily modified to suit this application
//
// Supports:
//
// '.'          Dot, matches any character
// '^'          Start anchor, matches beginning of string
// '$'          End anchor, matches end of string
// '*'          Asterisk, match zero or more (greedy)
// '+'          Plus, match one or more (greedy)
// '?'          Question, match zero or one (non-greedy)
// '[abc]'      Character class, match if one of {'a', 'b', 'c'}
// '[^abc]'     Inverted class, match if NOT one of {'a', 'b', 'c'} - NOTE: feature is currently broken!
// '[a-zA-Z]'   Character ranges, the character set of the ranges { a-z | A-Z }
// '\s'         Whitespace, \t \f \r \n \v and spaces
// '\S'         Non-whitespace
// '\w'         Alphanumeric, [a-zA-Z0-9_]
// '\W'         Non-alphanumeric
// '\d'         Digits, [0-9]
// '\D'         Non-digits
//
/* -------------------------------------------------------------------------------------------------------------------- */

#ifndef REGEX_H_
#define REGEX_H_

/* -------------------------------------------------------------------------------------------------------------------- */
// Defines
/* -------------------------------------------------------------------------------------------------------------------- */

#define REGEX_MAX_OPERATORS         30  // Max number of regex operators in expression
#define REGEX_CHAR_BUFFER_SIZE      40  // Max length of character-class buffer

/* -------------------------------------------------------------------------------------------------------------------- */
// Data structures
/* -------------------------------------------------------------------------------------------------------------------- */

// Regex compiled pattern
typedef struct regex_t
{
  unsigned char  type;  // Operator
  union
  {
    unsigned char  ch;  // Character
    unsigned char* ccl; // Pointer to character
  };
} regex_t;

// Pointer to get abstract datatype
typedef struct regex_t* re_t;

// Available regex operators
enum
{
    UNUSED, DOT, BEGIN, END, QUESTIONMARK, STAR, PLUS,
    CHAR,CHAR_CLASS, INV_CHAR_CLASS, DIGIT, NOT_DIGIT,
    ALPHA, NOT_ALPHA, WHITESPACE, NOT_WHITESPACE /*, BRANCH */
};

/* -------------------------------------------------------------------------------------------------------------------- */
// Prototypes - Public functions
/* -------------------------------------------------------------------------------------------------------------------- */

// Compile text pattern to a regex_t structure
re_t Regex_Compile (const char* Pattern);

// Find matches of the text pattern inside text (will compile automatically first)
int Regex_Match (const char* Text, const char* Pattern);

// Find matches of the compiled regex_t pattern inside text
int Regex_Matchp (const char* Text, re_t Pattern);

#endif

/* -------------------------------------------------------------------------------------------------------------------- */
// End of file
/* -------------------------------------------------------------------------------------------------------------------- */
