#include "eval.h"
#include "a-list.h"
#include "basics.h"

#define BUGGY 0

#if !BUGGY
#undef D
#undef M
#undef M2
#undef M3
#define D(...) 0
#define M(...) 0
#define M2(...) 0
#define M3(...) 0
#else
#include "dump.h"
#include "io.h"
#endif

S NLAMBDA("nlambda"), LAMBDA("lambda");

const S ATOMIC_FUNCTIONS = list(
  CAR,   CDR,  CONS,
  ATOM,  NULL, EQ,  COND, 
  ERROR, SET,  EVAL, QUOTE,
  DEFUN, NDEFUN, NLAMBDA, LAMBDA
);

inline bool atomic(S name) { return exists(name, ATOMIC_FUNCTIONS); }

/** Assertions like */ 
S bug(S s) { return s.error(T); }


#define FUN(Return, Name, ArgumentType) \
  Return Name(ArgumentType _) { \
    const auto $$ = Name; \
    M3("[",_,"]"); \
    Return __ =  

#define IS(X)    \
    X; \
    M2("-->", __); \
    return __; \
  } 

S defun(S name, S parameters, S body) {
    return set(name, list(LAMBDA, parameters, body));
}

S ndefun(S name, S parameters, S body) {
    return set(name, list(NLAMBDA, parameters, body));
}

FUN(S, evaluate_list, S) IS(
  _.null() ? NIL : _.car().eval().cons($$(_.cdr()))
) 

FUN(S, evaluate_cond, S)  IS( 
    _.null() ?  NIL:
    _.car().atom() ? _.car().error(COND):
      _.car().car().eval().t() ? _.car().cdr().car().eval():
        $$(_.cdr()) ;
)

S only(S s) {
    s.pair() || die(s);
    s.cdr().cdr().t() && s.error(REDUNDANT).t();
    S a = evaluate_list(s.cdr());
    a.null() && s.error(MISSING).t();
    a.atom() && s.error(INVALID).t();
    return a.car();
}

void checkNumberOfArgs(S s) {
    S f = s.car();
    // 1 Args:
    if (f.eq(QUOTE) || f.eq(CAR) || f.eq(CDR) ||
        f.eq(ATOM) || f.eq(NULL) || f.eq(EVAL)) {
        s.more2() && s.error(REDUNDANT).t();
        s.less2() && s.error(MISSING).t();
    }

    // 2 Args:
    if (f.eq(CONS) || f.eq(SET) || f.eq(EQ) || f.eq(LAMBDA) || f.eq(NLAMBDA)) {
        s.more3() && s.error(REDUNDANT).t();
        s.less3() && s.error(MISSING).t();
    }

    // 3 Args:
    if (f.eq(NDEFUN) || f.eq(DEFUN)) {
        s.more4() && s.error(REDUNDANT).t();
        s.less4() && s.error(MISSING).t();
    }
}

S evaluate_atomic_function(S s) { M(s);
    checkNumberOfArgs(s);
    S f = s.car();
    S res = NIL;
    // Atomic functions:
    if (f.eq(CAR)) {
        res = only(s).car();
    } else if (f.eq(CONS)) {
        res = s.$2$().eval().cons(s.$3$().eval());
    } else if (f.eq(SET)) {
        res = set(s.$2$().eval(), s.$3$().eval());
    } else if (f.eq(EQ)) {
        res = s.$2$().eval().eq(s.$3$().eval()) ? T : NIL;
    } else if (f.eq(COND)) {
        res = evaluate_cond(s.cdr());
    } else if (f.eq(CDR)) {
        res = only(s).cdr();
    } else if (f.eq(ATOM)) {
        res = only(s).atom() ? T : NIL;
    } else if (f.eq(EVAL)) {
        res = only(s).eval();
    } else if (f.eq(ERROR)) {
        res = s.error(s.cdr());
    }
    // Built-in functions:
    else if (f.eq(NULL)) {
        res = only(s).null() ? T : NIL;
    } else if (f.eq(QUOTE)) {
        res = s.cdr().car();
    } else if (f.eq(NLAMBDA)) {
        res = list(NLAMBDA, s.$2$(), s.$3$());
    } else if (f.eq(LAMBDA)) {
        res = list(LAMBDA, s.$2$(), s.$3$());
    } else if (f.eq(NDEFUN)) {
        ndefun(s.$2$(), s.$3$(), s.$4$());
        res = s.$2$();
    } else if (f.eq(DEFUN)) {
        defun(s.$2$(), s.$3$(), s.$4$());
        res = s.$2$();
    } else {
        return bug(s);
    }
    return res;
}

S apply(S f, S args) {
  f.n3() || f.cons(args).error(INVALID).t();
  const auto saved_alist = alist;
  const auto actuals = f.$1$().eq(NLAMBDA)? args : f.$1$().eq(LAMBDA) ? evaluate_list(args) : f.cons(args).error(INVALID);
  alist = bind(f.$2$(), actuals, alist);
  const auto result = f.$3$().eval();
  alist = saved_alist;
  return result;
}

S apply_defined_function(S f, S args) {
  apply(f,args);
}


FUN(S, eval, S) IS(
    _.atom() ? lookup(_):
    atomic(_.car()) ? evaluate_atomic_function(_):
      apply_defined_function($$(_.car()), _.cdr())
)

