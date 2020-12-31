
# Mini Regex
Mini regex module for microcontrollers - Based on the [Tiny Regex C](https://github.com/kokke/tiny-regex-c) by kokke

Edited by Renan R. Duarte (24/01/2019). Code heavily modified to suit my application

## Supports:
- '.' -> Dot, matches any character
- '^' -> Start anchor, matches beginning of string
- '$' -> End anchor, matches end of string
- '*' -> Asterisk, match zero or more (greedy)
- '+' -> Plus, match one or more (greedy)
- '?' -> Question, match zero or one (non-greedy)
- '[abc]' -> Character class, match if one of {'a', 'b', 'c'}
- '[^abc]' -> Inverted class, match if NOT one of {'a', 'b', 'c'} - NOTE: feature is currently broken!
- '[a-zA-Z]' -> Character ranges, the character set of the ranges { a-z | A-Z }
- '\s' -> Whitespace, \t \f \r \n \v and spaces
- '\S' -> Non-whitespace
- '\w' -> Alphanumeric, [a-zA-Z0-9_]
- '\W' -> Non-alphanumeric
- '\d' -> Digits, [0-9]
- '\D' -> Non-digits
