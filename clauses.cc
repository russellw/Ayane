#include "main.h"

const char* ruleNames[] = {
#define _(x) #x,
#include "rules.h"
};
