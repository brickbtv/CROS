This is a fork of ubasic with the aims of

- Making it smaller (removing lots of the more memory expensive coding)
- Bringing it up to ECMA55 and a bit beyond
- Making it faster (pretokenisation etc)
- Making it run on FUZIX boxen usefully

Currently

- Negative numbers are now supported
- Types for things are a bit clearer as they will matter
- Upper or lower case is permitted
- Errors in the original debugging code are fixed
- Variables may now be A-Z or A0-A9..Z0-Z9 as per ECMA55
- PRINT supports , for tab fields, ; for supressing newline
- PRINT constant string length is unlimited
- INPUT is added including support for a prompt
- FOR NEXT now supports STEP as per ECMA55
- PEEK() is now a function as in normal basic - X = PEEK(4)
- STOP replaces the rather odd "END"
- ABS() INT() and SGN() are implemented
- REM works as in normal BASIC
- DATA and RESTORE are supported but not yet READ !
- AND and OR keywords work (but not yet NOT)
- GOTO and GOSUB allow expressions "computed GOTO")
- GO SUB and GO TO are two tokens so can be spaced
- PRINT TAB() (SPC() is not ECMA55 nor is PRINT AT)
- String variables (A$-Z$ required)
- LEFT$(), RIGHT$(), MID$(), CHR$()
- VAL(),CODE()
- LEN()
- Proper print parsing (we don't allow PRINT ABC printing A then B then C.
  You must as in normal basic use ; or , .
- Faster RETURN and loops - we save the tokenizer pointer rather than mucking
  about playing hunt the line number
- Statements can be separated by :
- Removed the existing IF THEN ELSE in favour of a traditional IF THEN and
  : usage (IF THEN IF THEN ELSE ELSE ... gets horrible to parse and the old
  code messed it up badly)
- Arrays (1 or 2 dimensions required by ECMA55)
- Stdio is not used
- Logical expressions with AND and OR differently priorities to boolean & |
- Introduce "mod" to replace use of % - which we may need for other stuff later

In comparison with ECMA55, then apart from all the floaty stuff it's missing

- Implicit dimensioning of arrays
- The ^ operator (or ** equivalent)
- RND
- SQR
- DEF FN / FN (single or no variable required by ECMA55)
- READ
- Unquoted data strings

Space saving work needed

- Replace malloc() with a proper sbrk based basic allocator
- Only prealloc A-Z/A$-Z$ pointers
- Work out why factor and relation are so big - and shrink them
- Why are input_statement and dim_statement so big ?
- Maybe kill the line finding array
- Fast way to walk lines
- Pretokenize code to make smaller and faster
- Switch to compiling ?

Other useful stuff to add
- XOR
- USR()
- INKEY$
- CLS
- ON expr GO TO/SUB  ... {ELSE}
- ON ERROR
- ON TIMER/SIGNAL
- PAUSE
- Short tokens "P." etc
- DO WHILE
- DO UNTIL
- I/O streams PRINT# INPUT# OPEN# CLOSE# etc
- Command mode
- CLEAR
- NEW
- SAVE / LOAD / CHAIN
- Unix syscall bindings 8)
- Unify GOSUB/NEXT/etc stack so that RETURN wipes out next frames etc
