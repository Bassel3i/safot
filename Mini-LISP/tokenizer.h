#ifndef TOKENIZER_H
#define TOKENIZER_H
#include "mini-lisp.h"
namespace Tokenizer {
enum { undefined = 128, $};
const char tokens[] = "()[].\'";
extern void initialize(char *buffer);
// Retrieve the next token: if non-positive, this is an atom (including NIL)
// else it is a character as per the table above
extern H next(void); 
extern H get(void); 
extern H unget(void); 
}
extern bool exists(const char, String s); 

#endif
