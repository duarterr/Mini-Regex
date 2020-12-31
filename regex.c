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

/* -------------------------------------------------------------------------------------------------------------------- */
// Includes
/* -------------------------------------------------------------------------------------------------------------------- */

// Stdio
#include <stdio.h>

// Project specific defines and macros
#include <regex.h>

/* -------------------------------------------------------------------------------------------------------------------- */
// Prototypes - Private functions
/* -------------------------------------------------------------------------------------------------------------------- */

static int ismetachar(char c);
static int matchdigit(char c);
static int matchalpha(char c);
static int matchalphanum(char c);
static int matchwhitespace(char c);
static int matchrange(char c, const char* str);
static int matchmetachar(char c, const char* str);
static int matchcharclass(char c, const char* str);
static int matchstar(regex_t p, regex_t* pattern, const char* text);
static int matchplus(regex_t p, regex_t* pattern, const char* text);
static int matchquestion(regex_t p, regex_t* pattern, const char* text);
static int matchone(regex_t p, char c);
static int matchpattern(regex_t* pattern, const char* text);

/* -------------------------------------------------------------------------------------------------------------------- */
// Compile text pattern to a regex_t structure
/* -------------------------------------------------------------------------------------------------------------------- */

re_t Regex_Compile (const char* Pattern)
{
    // Compiled pattern
    static regex_t Pattern_Compiled[REGEX_MAX_OPERATORS];

    // Char buffer
    static unsigned char Char_Buffer[REGEX_CHAR_BUFFER_SIZE];

    // Index of char buffer
    int Index_Char_Buffer = 1;

    // Index of Pattern
    int Index_Pattern = 0;

    // Index of Pattern compiled
    int Index_Pattern_Compiled = 0;

    // Current char in Pattern
    char Current_Char;

    // Scan entire pattern
    while ((Pattern[Index_Pattern] != '\0') && ((Index_Pattern_Compiled+ 1) < REGEX_MAX_OPERATORS))
    {
        // Get current char in Pattern
        Current_Char = Pattern[Index_Pattern];

        // Process current char char
        switch (Current_Char)
        {
            // Meta-characters
            case '^': {Pattern_Compiled[Index_Pattern_Compiled].type = BEGIN;           } break;
            case '$': {Pattern_Compiled[Index_Pattern_Compiled].type = END;             } break;
            case '.': {Pattern_Compiled[Index_Pattern_Compiled].type = DOT;             } break;
            case '*': {Pattern_Compiled[Index_Pattern_Compiled].type = STAR;            } break;
            case '+': {Pattern_Compiled[Index_Pattern_Compiled].type = PLUS;            } break;
            case '?': {Pattern_Compiled[Index_Pattern_Compiled].type = QUESTIONMARK;    } break;
            //case '|': {Pattern_Compiled[Index_Pattern_Compiled].type = BRANCH;          } break; // Not working

            // Escaped character-classes (\s \w ...)
            case '\\':
            {
                // '\\' is not last char in pattern
                if (Pattern[Index_Pattern+ 1] != '\0')
                {
                    // Skip the escape-char '\\'
                    Index_Pattern++;

                    // Process the next
                    switch (Pattern[Index_Pattern])
                    {
                        // Meta-character
                        case 'd': {Pattern_Compiled[Index_Pattern_Compiled].type = DIGIT;            } break;
                        case 'D': {Pattern_Compiled[Index_Pattern_Compiled].type = NOT_DIGIT;        } break;
                        case 'w': {Pattern_Compiled[Index_Pattern_Compiled].type = ALPHA;            } break;
                        case 'W': {Pattern_Compiled[Index_Pattern_Compiled].type = NOT_ALPHA;        } break;
                        case 's': {Pattern_Compiled[Index_Pattern_Compiled].type = WHITESPACE;       } break;
                        case 'S': {Pattern_Compiled[Index_Pattern_Compiled].type = NOT_WHITESPACE;   } break;

                        // Escaped character - '.' or '$'
                        default:
                        {
                            Pattern_Compiled[Index_Pattern_Compiled].type = CHAR;
                            Pattern_Compiled[Index_Pattern_Compiled].ch = Pattern[Index_Pattern];
                        } break;
                    }
                }
            } break;

            // Character class
            case '[':
            {
                // Remember where the char-buffer starts
                int Index_Begin = Index_Char_Buffer;

                // Look-ahead to determine if negated
                if (Pattern[Index_Pattern+ 1] == '^')
                {
                    Pattern_Compiled[Index_Pattern_Compiled].type = INV_CHAR_CLASS;

                    // Increment Index_Pattern to avoid including '^' in the char-buffer
                    Index_Pattern++;
                }

                else
                    Pattern_Compiled[Index_Pattern_Compiled].type = CHAR_CLASS;

                // Copy characters inside [..] to buffer
                while ((Pattern[++Index_Pattern] != ']') && (Pattern[Index_Pattern] != '\0')) // Missing ]
                {
                    if (Pattern[Index_Pattern] == '\\')
                    {
                        // Exceeded internal buffer size
                        if (Index_Char_Buffer >= REGEX_CHAR_BUFFER_SIZE - 1)
                            return 0;

                        // Copy char to buffer
                        Char_Buffer[Index_Char_Buffer++] = Pattern[Index_Pattern++];
                    }

                    // Exceeded internal buffer size
                    else if (Index_Char_Buffer >= REGEX_CHAR_BUFFER_SIZE)
                        return 0;

                    // Copy char to buffer
                    Char_Buffer[Index_Char_Buffer++] = Pattern[Index_Pattern];
                }

                // Exceeded internal buffer size
                if (Index_Char_Buffer >= REGEX_CHAR_BUFFER_SIZE)
                    return 0;

                // Null-terminate string end
                Char_Buffer[Index_Char_Buffer++] = 0;
                Pattern_Compiled[Index_Pattern_Compiled].ccl = &Char_Buffer[Index_Begin];
            } break;

            // Other characters
            default:
            {
                // Save to compiled structure
                Pattern_Compiled[Index_Pattern_Compiled].type = CHAR;
                Pattern_Compiled[Index_Pattern_Compiled].ch = Current_Char;
            } break;
        }

        // Increment indexes
        Index_Pattern++;
        Index_Pattern_Compiled++;
    }

    // 'UNUSED' is a sentinel used to indicate end-of-pattern
    Pattern_Compiled[Index_Pattern_Compiled].type = UNUSED;

    // Return compiled pattern
    return (re_t) Pattern_Compiled;
}

/* -------------------------------------------------------------------------------------------------------------------- */
// Find matches of the text pattern inside text (will compile automatically first)
/* -------------------------------------------------------------------------------------------------------------------- */

int Regex_Match (const char* Text, const char* Pattern)
{
    // Compile pattern and find matches
    return Regex_Matchp (Text, Regex_Compile (Pattern));
}

/* -------------------------------------------------------------------------------------------------------------------- */
// Find matches of the compiled regex_t pattern inside text
/* -------------------------------------------------------------------------------------------------------------------- */

int Regex_Matchp (const char* Text, re_t Pattern)
{
    // Compiled pattern is valid
    if (Pattern != 0)
    {
        // Pattern has BEGIN operator
        if (Pattern[0].type == BEGIN)
            return ((matchpattern(&Pattern[1], Text)) ? 0 : -1);

        // Pattern doesn't have BEGIN operator
        else
        {
            // Index of the first match
            int Index = -1;

            // Scan all text
            do
            {
                // Increment index
                Index++;

                // Check for match
                if (matchpattern(Pattern, Text))
                {
                    if (Text[0] == '\0')
                        return -1;

                    return Index;
                }
            }
            while (*Text++ != '\0');
        }
    }

    // Compiled pattern is invalid
    return -1;
}

/* -------------------------------------------------------------------------------------------------------------------- */
// Private functions
/* -------------------------------------------------------------------------------------------------------------------- */

// Check if char is metachar
static int ismetachar(char c)
{
    return ((c == 's') || (c == 'S') || (c == 'w') || (c == 'W') || (c == 'd') || (c == 'D'));
}

// Match numeric
static int matchdigit (char c)
{
    return ((c >= '0') && (c <= '9'));
}

// Match character
static int matchalpha(char c)
{
    return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}

// Match alphanumeric
static int matchalphanum(char c)
{
    return ((c == '_') || matchalpha(c) || matchdigit(c));
}

// Match whitespace and special characters
static int matchwhitespace (char c)
{
    return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\f') || (c == '\v'));
}

// Match ranges
static int matchrange(char c, const char* str)
{
    return ((c != '-') && (str[0] != '\0') && (str[0] != '-') &&
            (str[1] == '-') && (str[1] != '\0') &&
            (str[2] != '\0') && ((c >= str[0]) && (c <= str[2])));
}

// Match metachar
static int matchmetachar(char c, const char* str)
{
    switch (str[0])
    {
        case 'd': return  matchdigit(c);
        case 'D': return !matchdigit(c);
        case 'w': return  matchalphanum(c);
        case 'W': return !matchalphanum(c);
        case 's': return  matchwhitespace(c);
        case 'S': return !matchwhitespace(c);
        default:  return (c == str[0]);
    }
}

// Match characters-class
static int matchcharclass(char c, const char* str)
{
    do
    {
        if (matchrange(c, str))
        {
            return 1;
        }
        else if (str[0] == '\\')
        {
            /* Escape-char: increment str-ptr and match on next char */
            str += 1;
            if (matchmetachar(c, str))
            {
                return 1;
            }
            else if ((c == str[0]) && !ismetachar(c))
            {
                return 1;
            }
        }
        else if (c == str[0])
        {
            if (c == '-')
            {
                return ((str[-1] == '\0') || (str[1] == '\0'));
            }
            else
            {
                return 1;
            }
        }
    }
    while (*str++ != '\0');

    return 0;
}

// Match 0 or more
static int matchstar(regex_t p, regex_t* pattern, const char* text)
{
    do
    {
        if (matchpattern(pattern, text))
            return 1;
    }
    while ((text[0] != '\0') && matchone(p, *text++));

    return 0;
}

// Match 1 or more
static int matchplus(regex_t p, regex_t* pattern, const char* text)
{
    while ((text[0] != '\0') && matchone(p, *text++))
    {
        if (matchpattern(pattern, text))
            return 1;
    }
    return 0;
}

// Match zero or one
static int matchquestion(regex_t p, regex_t* pattern, const char* text)
{
    if (p.type == UNUSED)
        return 1;
    if (matchpattern(pattern, text))
        return 1;
    if (*text && matchone(p, *text++))
        return matchpattern(pattern, text);
    return 0;
}

// Match one char of pattern
static int matchone(regex_t p, char c)
{
    switch (p.type)
    {
    case DOT:            return 1;
    case CHAR_CLASS:     return  matchcharclass(c, (const char*)p.ccl);
    case INV_CHAR_CLASS: return !matchcharclass(c, (const char*)p.ccl);
    case DIGIT:          return  matchdigit(c);
    case NOT_DIGIT:      return !matchdigit(c);
    case ALPHA:          return  matchalphanum(c);
    case NOT_ALPHA:      return !matchalphanum(c);
    case WHITESPACE:     return  matchwhitespace(c);
    case NOT_WHITESPACE: return !matchwhitespace(c);
    default:             return  (p.ch == c);
    }
}

// Match pattern
static int matchpattern(regex_t* pattern, const char* text)
{
    do
    {
        if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK))
            return matchquestion(pattern[0], &pattern[2], text);

        else if (pattern[1].type == STAR)
            return matchstar(pattern[0], &pattern[2], text);

        else if (pattern[1].type == PLUS)
            return matchplus(pattern[0], &pattern[2], text);

        else if ((pattern[0].type == END) && pattern[1].type == UNUSED)
            return (text[0] == '\0');

        // Branching is not working properly
        //else if (pattern[1].type == BRANCH)
        //  return (matchpattern(pattern, text) || matchpattern(&pattern[2], text));
    }
    while ((text[0] != '\0') && matchone(*pattern++, *text++));

    return 0;
}

/* -------------------------------------------------------------------------------------------------------------------- */
// End of file
/* -------------------------------------------------------------------------------------------------------------------- */
