#ifndef STACK_TRACE_H_
#define STACK_TRACE_H_

#include <iostream>

extern "C" void stack_trace();

#define normally(x) if (x); else  {\
  fprintf(stderr,"\n\n** Aborting at %s/%s (%d): Expression (%s) evaluated to false\n",__FILE__, __FUNCTION__, __LINE__,#x); \
  stack_trace(); \
  throw (__LINE__); \
}

#define never(x) {\
  fprintf(stderr,"\n\n** Aborting at %s/%s (%d): %s\n",__FILE__, __FUNCTION__, __LINE__,#x); \
  stack_trace(); \
  throw (__LINE__); \
}

// #define D(...) LOCATION FOR_EACH(DUMP,__VA_ARGS__) << std::endl



#endif /* STACK_TRACE_H_ */
