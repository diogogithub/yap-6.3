/*************************************************************************
 *									 *
 *	 YAP Prolog 							 *
 *									 *
 *	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
 *									 *
 * Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
 *									 *
 **************************************************************************
 *									 *
 * File:		parser.c *
 * Last rev:								 *
 * mods: *
 * comments:	Prolog's parser						 *
 *									 *
 *************************************************************************/
#ifdef SCCS
static char SccsId[] = "%W% %G%";
#endif

/**

@addtogroup YAPSyntax

describe the syntax for Prolog terms. In a second level we describe
the \a tokens from which Prolog \a terms are
built.

@defgroup Formal_Syntax Syntax of Terms
@ingroup YAPSyntax
@{

Below, we describe the syntax of YAP terms from the different
classes of tokens defined above. The formalism used will be <em>BNF</em>,
extended where necessary with attributes denoting integer precedence or
operator type.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                                                  term
---->     subterm(1200)   end_of_term_marker

 subterm(N) ---->     term(M)         [M <= N]

 term(N)    ---->     op(N, fx) subterm(N-1)
             |        op(N, fy) subterm(N)
             |        subterm(N-1) op(N, xfx) subterm(N-1)
             |        subterm(N-1) op(N, xfy) subterm(N)
             |        subterm(N) op(N, yfx) subterm(N-1)
             |        subterm(N-1) op(N, xf)
             |        subterm(N) op(N, yf)

 term(0)   ---->      atom '(' arguments ')'
             |        '(' subterm(1200)  ')'
             |        '{' subterm(1200)  '}'
             |        list
             |        string
             |        number
             |        atom
             |        variable

 arguments ---->      subterm(999)
             |        subterm(999) ',' arguments

 list      ---->      '[]'
             |        '[' list_expr ']'

 list_expr ---->      subterm(999)
             |        subterm(999) list_tail

 list_tail ---->      ',' list_expr
             |        ',..' subterm(999)
             |        '|' subterm(999)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Notes:

   + \a op(N,T) denotes an atom which has been previously declared with type
      \a T and base precedence \a N.

  + Since ',' is itself a pre-declared operator with type \a xfy and
       precedence 1000, is \a subterm starts with a '(', \a op must be
       followed by a space to avoid ambiguity with the case of a functor
       followed by arguments, e.g.:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ (a,b)        [the same as '+'(','(a,b)) of arity one]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      versus

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+(a,b)         [the same as '+'(a,b) of arity two]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  +
In the first rule for term(0) no blank space should exist between
\a atom and '('.

  +
Each term to be read by the YAP parser must end with a single
dot, followed by a blank (in the sense mentioned in the previous
paragraph). When a name consisting of a single dot could be taken for
the end of term marker, the ambiguity should be avoided by surrounding the
dot with single quotes.

@}

*/

/*
 * Description:
 *
 * parser:     produces a prolog term from an array of tokens
 *
 * parser usage: the parser takes its input from an array of token descriptions
 * addressed by the global variable 'tokptr' and produces a Term as result. A
 * macro 'NextToken' should be defined in 'yap.h' for advancing 'tokptr' from
 * one token to the next. In the distributed version this macro also updates
 * a variable named 'toktide' for keeping track of how far the parser went
 * before failling with a syntax error. The parser should be invoked with
 * 'tokptr' pointing to the first token. The last token should have type
 * 'eot_tok'. The parser return either a Term. Syntactic errors are signaled
 * by a return value 0. The parser builds new terms on the 'global stack' and
 * also uses an auxiliary stack pointed to by 'AuxSp'. In the distributed
 * version this auxiliary stack is assumed to grow downwards. This
 * assumption, however, is only relevant to routine 'ParseArgs', and to the
 * variable toktide. conclusion: set tokptr pointing to first token set AuxSp
 * Call Parse
 *
 * VSC: Working whithout known bugs in 87/4/6
 *
 * LD: -I or +I evaluated by parser 87/4/28
 *
 * LD: parser extended 87/4/28
 *
 */

#include "Yap.h"
#include "YapEval.h"
#include "YapHeap.h"
#include "YapText.h"
#include "Yatom.h"
#include "yapio.h"
/* stuff we want to use in standard YAP code */
#include "iopreds.h"
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef __STDC__XXX
#define Volatile volatile
#else
#define Volatile
#endif

/* weak backtraking mechanism based on long_jump */

typedef struct jmp_buff_struct {
  sigjmp_buf JmpBuff;
} JMPBUFF;

static void GNextToken(CACHE_TYPE1);
static void checkfor(Term, JMPBUFF *, encoding_t CACHE_TYPE);
static Term ParseArgs(Atom, Term, JMPBUFF *, Term, encoding_t, Term CACHE_TYPE);
static Term ParseList(JMPBUFF *, encoding_t, Term CACHE_TYPE);
static Term ParseTerm(int, JMPBUFF *, encoding_t, Term CACHE_TYPE);

extern Term Yap_tokRep(void *tokptr);
extern const char *Yap_tokText(void *tokptr);

static void syntax_msg(const char *msg, ...) {
  CACHE_REGS
  va_list ap;
  if (!LOCAL_ErrorMessage ||
      (LOCAL_Error_TYPE == SYNTAX_ERROR &&
              LOCAL_tokptr->TokPos < LOCAL_ActiveError->prologParserPos  )) {
    if (!LOCAL_ErrorMessage) {
      LOCAL_ErrorMessage = malloc(MAX_ERROR_MSG_SIZE + 1);
    }
    LOCAL_ActiveError->prologParserLine = LOCAL_tokptr->TokLine;
    LOCAL_ActiveError->prologParserPos = LOCAL_tokptr->TokPos;
    va_start(ap, msg);
    vsnprintf(LOCAL_ErrorMessage, MAX_ERROR_MSG_SIZE, msg, ap);
    va_end(ap);
  }
}

#define TRY(S, P)                                                              \
  {                                                                            \
    Volatile JMPBUFF *saveenv, newenv;                                         \
    Volatile TokEntry *saveT = LOCAL_tokptr;                                   \
    Volatile CELL *saveH = HR;                                                 \
    Volatile int savecurprio = curprio;                                        \
    saveenv = FailBuff;                                                        \
    if (!sigsetjmp(newenv.JmpBuff, 0)) {                                       \
      FailBuff = &newenv;                                                      \
      S;                                                                       \
      FailBuff = saveenv;                                                      \
      P;                                                                       \
    } else {                                                                   \
      FailBuff = saveenv;                                                      \
      HR = saveH;                                                              \
      curprio = savecurprio;                                                   \
      LOCAL_tokptr = saveT;                                                    \
    }                                                                          \
  }

#define TRY3(S, P, F)                                                          \
  {                                                                            \
    Volatile JMPBUFF *saveenv, newenv;                                         \
    Volatile TokEntry *saveT = LOCAL_tokptr;                                   \
    Volatile CELL *saveH = HR;                                                 \
    saveenv = FailBuff;                                                        \
    if (!sigsetjmp(newenv.JmpBuff, 0)) {                                       \
      FailBuff = &newenv;                                                      \
      S;                                                                       \
      FailBuff = saveenv;                                                      \
      P;                                                                       \
    } else {                                                                   \
      FailBuff = saveenv;                                                      \
      HR = saveH;                                                              \
      LOCAL_tokptr = saveT;                                                    \
      F                                                                        \
    }                                                                          \
  }

#define FAIL siglongjmp(FailBuff->JmpBuff, 1)

VarEntry *Yap_LookupVar(const char *var) /* lookup variable in variables table
                                          * */
{
  CACHE_REGS
  VarEntry *p;
  Atom vat = Yap_LookupAtom(var);

#if DEBUG
  if (GLOBAL_Option[4])
    fprintf(stderr, "[LookupVar %s]", var);
#endif
  if (var[0] != '_' || var[1] != '\0') {
    VarEntry **op = &LOCAL_VarTable;
    UInt hv;

    p = LOCAL_VarTable;
    hv = HashFunction((unsigned char *)var) % AtomHashTableSize;
    while (p != NULL) {
      CELL hpv = p->hv;
      if (hv == hpv) {
        Int scmp;
        if ((scmp = strcmp(var, RepAtom(p->VarRep)->StrOfAE)) == 0) {
          p->refs++;
          return (p);
        } else if (scmp < 0) {
          op = &(p->VarLeft);
          p = p->VarLeft;
        } else {
          op = &(p->VarRight);
          p = p->VarRight;
        }
      } else if (hv < hpv) {
        op = &(p->VarLeft);
        p = p->VarLeft;
      } else {
        op = &(p->VarRight);
        p = p->VarRight;
      }
    }
    p = (VarEntry *)Yap_AllocScannerMemory(sizeof(VarEntry));
    *op = p;
    p->VarLeft = p->VarRight = NULL;
    p->hv = hv;
    p->refs = 1L;
    p->VarRep = vat;
  } else {
    /* anon var */
    p = (VarEntry *)Yap_AllocScannerMemory(sizeof(VarEntry));
    p->VarLeft = LOCAL_AnonVarTable;
    LOCAL_AnonVarTable = p;
    p->VarRight = NULL;
    p->refs = 0L;
    p->hv = 1L;
    p->VarRep = vat;
  }
  p->VarAdr = TermNil;
  return (p);
}

static Term VarNames(VarEntry *p, Term l USES_REGS) {
  if (p != NULL) {
    if (strcmp(RepAtom(p->VarRep)->StrOfAE, "_") != 0) {
      Term t[2];
      Term o;

      t[0] = MkAtomTerm(p->VarRep);
      if (!IsVarTerm(p->VarAdr))
        p->VarAdr = MkVarTerm();
      t[1] = p->VarAdr;
      o = Yap_MkApplTerm(FunctorEq, 2, t);
      o = MkPairTerm(o, VarNames(p->VarRight,
                                 VarNames(p->VarLeft, l PASS_REGS) PASS_REGS));
      if (HR > ASP - 4096) {
        save_machine_regs();
        longjmp(LOCAL_IOBotch, 1);
      }
      return (o);
    } else {
      return VarNames(p->VarRight, VarNames(p->VarLeft, l PASS_REGS) PASS_REGS);
    }
  } else {
    return (l);
  }
}

Term Yap_VarNames(VarEntry *p, Term l) {
  CACHE_REGS
  return VarNames(p, l PASS_REGS);
}

static Term Singletons(VarEntry *p, Term l USES_REGS) {
  if (p != NULL) {
    if (RepAtom(p->VarRep)->StrOfAE[0] != '_' && p->refs == 1) {
      Term t[2];
      Term o;

      t[0] = MkAtomTerm(p->VarRep);
      t[1] = p->VarAdr;
      o = Yap_MkApplTerm(FunctorEq, 2, t);
      o = MkPairTerm(o,
                     Singletons(p->VarRight,
                                Singletons(p->VarLeft, l PASS_REGS) PASS_REGS));
      if (HR > ASP - 4096) {
        save_machine_regs();
        longjmp(LOCAL_IOBotch, 1);
      }
      return (o);
    } else {
      return Singletons(p->VarRight,
                        Singletons(p->VarLeft, l PASS_REGS) PASS_REGS);
    }
  } else {
    return (l);
  }
}

Term Yap_Singletons(VarEntry *p, Term l) {
  CACHE_REGS
  return Singletons(p, l PASS_REGS);
}

static Term Variables(VarEntry *p, Term l USES_REGS) {
  if (p != NULL) {
    Term o;
    o = MkPairTerm(
        p->VarAdr,
        Variables(p->VarRight, Variables(p->VarLeft, l PASS_REGS) PASS_REGS));
    if (HR > ASP - 4096) {
      save_machine_regs();
      siglongjmp(LOCAL_IOBotch, 1);
    }
    return (o);
  } else {
    return (l);
  }
}

Term Yap_Variables(VarEntry *p, Term l) {
  CACHE_REGS
  l = Variables(LOCAL_AnonVarTable, l PASS_REGS);
  return Variables(p, l PASS_REGS);
}

static int IsPrefixOp(Atom op, int *pptr, int *rpptr, Term cmod USES_REGS) {
  int p;

  OpEntry *opp = Yap_GetOpProp(op, PREFIX_OP, cmod PASS_REGS);
  if (!opp)
    return FALSE;
  if (opp->OpModule && opp->OpModule != cmod) {
    READ_UNLOCK(opp->OpRWLock);
    return FALSE;
  }
  if ((p = opp->Prefix) != 0) {
    READ_UNLOCK(opp->OpRWLock);
    *pptr = *rpptr = p & MaskPrio;
    if (p & DcrrpFlag)
      --*rpptr;
    return TRUE;
  } else {
    READ_UNLOCK(opp->OpRWLock);
    return FALSE;
  }
}

int Yap_IsPrefixOp(Atom op, int *pptr, int *rpptr) {
  CACHE_REGS
  return IsPrefixOp(op, pptr, rpptr, CurrentModule PASS_REGS);
}

static int IsInfixOp(Atom op, int *pptr, int *lpptr, int *rpptr,
                     Term cmod USES_REGS) {
  int p;

  OpEntry *opp = Yap_GetOpProp(op, INFIX_OP, cmod PASS_REGS);
  if (!opp)
    return false;
  if (opp->OpModule && opp->OpModule != cmod) {
    READ_UNLOCK(opp->OpRWLock);
    return false;
  }
  if ((p = opp->Infix) != 0) {
    READ_UNLOCK(opp->OpRWLock);
    *pptr = *rpptr = *lpptr = p & MaskPrio;
    if (p & DcrrpFlag)
      --*rpptr;
    if (p & DcrlpFlag)
      --*lpptr;
    return TRUE;
  } else {
    READ_UNLOCK(opp->OpRWLock);
    return FALSE;
  }
}

int Yap_IsInfixOp(Atom op, int *pptr, int *lpptr, int *rpptr) {
  CACHE_REGS
  return IsInfixOp(op, pptr, lpptr, rpptr, CurrentModule PASS_REGS);
}

static int IsPosfixOp(Atom op, int *pptr, int *lpptr, Term cmod USES_REGS) {
  int p;

  OpEntry *opp = Yap_GetOpProp(op, POSFIX_OP, cmod PASS_REGS);
  if (!opp)
    return FALSE;
  if (opp->OpModule && opp->OpModule != cmod) {
    READ_UNLOCK(opp->OpRWLock);
    return FALSE;
  }
  if ((p = opp->Posfix) != 0) {
    READ_UNLOCK(opp->OpRWLock);
    *pptr = *lpptr = p & MaskPrio;
    if (p & DcrlpFlag)
      --*lpptr;
    return (TRUE);
  } else {
    READ_UNLOCK(opp->OpRWLock);
    return (FALSE);
  }
}

int Yap_IsPosfixOp(Atom op, int *pptr, int *lpptr) {
  CACHE_REGS
  return IsPosfixOp(op, pptr, lpptr, CurrentModule PASS_REGS);
}

inline static void GNextToken(USES_REGS1) {
  if (LOCAL_tokptr->Tok == Ord(eot_tok))
    return;
  if (LOCAL_tokptr == LOCAL_toktide) {
    LOCAL_toktide = LOCAL_tokptr = LOCAL_tokptr->TokNext;
  } else
    LOCAL_tokptr = LOCAL_tokptr->TokNext;
}

inline static void checkfor(Term c, JMPBUFF *FailBuff,
                            encoding_t enc USES_REGS) {
  if (LOCAL_tokptr->Tok != Ord(Ponctuation_tok) || LOCAL_tokptr->TokInfo != c) {
    char s[1024];
    strncpy(s, Yap_tokText(LOCAL_tokptr), 1023);
    syntax_msg("line %d: expected to find "
               "\'%c....................................\', found %s",
               LOCAL_tokptr->TokLine, c, s);
    FAIL;
  }
  NextToken;
}

#ifdef O_QUASIQUOTATIONS

static int is_quasi_quotation_syntax(Term goal, Atom *pat, encoding_t enc,
                                     Term cmod) {
  CACHE_REGS
  Term m = cmod, t;
  Atom at;
  UInt arity;
  Functor f;

  t = Yap_StripModule(goal, &m);
  f = FunctorOfTerm(t);
  *pat = at = NameOfFunctor(f);
  arity = ArityOfFunctor(f);
  if (arity > 0)
    return TRUE;
  return FALSE;
}

static int get_quasi_quotation(term_t t, unsigned char **here,
                               unsigned char *ein) {
  unsigned char *in, *start = *here;

  for (in = start; in <= ein; in++) {
    if (in[0] == '}' && in[-1] == '|') {
      *here = in + 1; /* after } */
      in--;           /* Before | */

      if (LOCAL_quasi_quotations) /* option; must return strings */
      {
        PL_chars_t txt;
        int rc;

        txt.text.t = (char *)start;
        txt.length = in - start;
        txt.storage = PL_CHARS_HEAP;
        txt.encoding = ENC_UTF8;
        txt.canonical = FALSE;

        rc = PL_unify_text(t, 0, &txt, PL_CODE_LIST);
        PL_free_text(&txt);

        return rc;
      } else {
        return PL_unify_term(t, PL_FUNCTOR, FUNCTOR_dquasi_quotation3,
                             PL_POINTER, LOCAL, PL_INTPTR, (intptr_t)(start),
                             PL_INTPTR, (intptr_t)(in - start));
      }
    }
  }

  return false; // errorWarning("end_of_file_in_quasi_quotation", 0, _PL_rd);
}
#endif /*O_QUASIQUOTATIONS*/

static Term ParseArgs(Atom a, Term close, JMPBUFF *FailBuff, Term arg1,
                      encoding_t enc, Term cmod USES_REGS) {
  int nargs = 0;
  Term *p, t;
  Functor func;
#ifdef SFUNC
  SFEntry *pe = (SFEntry *)Yap_GetAProp(a, SFProperty);
#endif

  NextToken;
  p = (Term *)ParserAuxSp;
  if (arg1) {
    *p = arg1;
    nargs++;
    ParserAuxSp = (char *)(p + 1);
    if (LOCAL_tokptr->Tok == Ord(Ponctuation_tok) &&
        LOCAL_tokptr->TokInfo == close) {

      func = Yap_MkFunctor(a, 1);
      if (func == NULL) {
        syntax_msg("line %d: Heap Overflow", LOCAL_tokptr->TokLine);
        FAIL;
      }
      t = Yap_MkApplTerm(func, nargs, p);
      if (HR > ASP - 4096) {
        syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
        return TermNil;
      }
      NextToken;
      return t;
    }
  }
  while (1) {
    Term *tp = (Term *)ParserAuxSp;
    if (ParserAuxSp + 1 > LOCAL_TrailTop) {
      syntax_msg("line %d: Trail Overflow", LOCAL_tokptr->TokLine);
      FAIL;
    }
    *tp++ = Unsigned(ParseTerm(999, FailBuff, enc, cmod PASS_REGS));
    ParserAuxSp = (char *)tp;
    ++nargs;
    if (LOCAL_tokptr->Tok != Ord(Ponctuation_tok))
      break;
    if (LOCAL_tokptr->TokInfo != TermComma)
      break;
    NextToken;
  }
  ParserAuxSp = (char *)p;
  /*
   * Needed because the arguments for the functor are placed in reverse
   * order
   */
  if (HR > ASP - (nargs + 1)) {
    syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
    FAIL;
  }
  func = Yap_MkFunctor(a, nargs);
  if (func == NULL) {
    syntax_msg("line %d: Heap Overflow", LOCAL_tokptr->TokLine);
    FAIL;
  }
#ifdef SFUNC
  if (pe)
    t = MkSFTerm(Yap_MkFunctor(a, SFArity), nargs, p, pe->NilValue);
  else
    t = Yap_MkApplTerm(Yap_MkFunctor(a, nargs), nargs, p);
#else
  if (a == AtomDBref && nargs == 2)
    t = MkDBRefTerm((DBRef)IntegerOfTerm(p[0]));
  else
    t = Yap_MkApplTerm(func, nargs, p);
#endif
  if (HR > ASP - 4096) {
    syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
    return TermNil;
  }
  /* check for possible overflow against local stack */
  checkfor(close, FailBuff, enc PASS_REGS);
  return t;
}

static Term MakeAccessor(Term t, Functor f USES_REGS) {
  UInt arity = ArityOfFunctor(FunctorOfTerm(t));
  int i;
  Term tf[2], tl = TermNil;

  tf[1] = ArgOfTerm(1, t);
  for (i = arity; i > 1; i--) {
    tl = MkPairTerm(ArgOfTerm(i, t), tl);
  }
  tf[0] = tl;
  return Yap_MkApplTerm(f, 2, tf);
}

static Term ParseList(JMPBUFF *FailBuff, encoding_t enc, Term cmod USES_REGS) {
  Term o;
  CELL *to_store;
  o = AbsPair(HR);
loop:
  to_store = HR;
  HR += 2;
  to_store[0] = ParseTerm(999, FailBuff, enc, cmod PASS_REGS);
  if (LOCAL_tokptr->Tok == Ord(Ponctuation_tok)) {
    if (LOCAL_tokptr->TokInfo == TermComma) {
      NextToken;
      {
        /* check for possible overflow against local stack */
        if (HR > ASP - 4096) {
          to_store[1] = TermNil;
          syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
          FAIL;
        } else {
          to_store[1] = AbsPair(HR);
          goto loop;
        }
      }
    } else if (LOCAL_tokptr->TokInfo == TermVBar) {
      NextToken;
      to_store[1] = ParseTerm(999, FailBuff, enc, cmod PASS_REGS);
    } else {
      to_store[1] = MkAtomTerm(AtomNil);
    }
  } else {
    syntax_msg("line %d: looking for symbol ',','|' got symbol '%s'",
               LOCAL_tokptr->TokLine, Yap_tokText(LOCAL_tokptr));
    FAIL;
  }
  return (o);
}

static Term ParseTerm(int prio, JMPBUFF *FailBuff, encoding_t enc,
                      Term cmod USES_REGS) {
  /* parse term with priority prio */
  Volatile Term t;
  Volatile Functor func;
  Volatile VarEntry *varinfo;
  Volatile int curprio = 0, opprio, oplprio, oprprio;
  Volatile Atom opinfo;

  switch (LOCAL_tokptr->Tok) {
  case Name_tok:
    t = LOCAL_tokptr->TokInfo;
    NextToken;
    /* special rules apply for +1, -2.3, etc... */
    if (LOCAL_tokptr->Tok == Number_tok) {
      if (t == TermMinus) {
        t = LOCAL_tokptr->TokInfo;
        if (IsIntTerm(t))
          t = MkIntTerm(-IntOfTerm(t));
        else if (IsFloatTerm(t))
          t = MkFloatTerm(-FloatOfTerm(t));
#ifdef USE_GMP
        else if (IsBigIntTerm(t)) {
          t = Yap_gmp_neg_big(t);
        }
#endif
        else
          t = MkLongIntTerm(-LongIntOfTerm(t));
        NextToken;
        break;
      }
    }
    if ((LOCAL_tokptr->Tok != Ord(Ponctuation_tok) ||
         LOCAL_tokptr->TokInfo != Terml) &&
        IsPrefixOp(AtomOfTerm(t), &opprio, &oprprio, cmod PASS_REGS)) {
      if (LOCAL_tokptr->Tok == Name_tok) {
        Atom at = AtomOfTerm(LOCAL_tokptr->TokInfo);
#ifndef _MSC_VER
        if (t == TermPlus) {
          if (at == AtomInf) {
            t = MkFloatTerm(INFINITY);
            NextToken;
            break;
          } else if (at == AtomNan) {
            t = MkFloatTerm(NAN);
            NextToken;
            break;
          }
        } else if (t == TermMinus) {
          if (at == AtomInf) {
            t = MkFloatTerm(-INFINITY);
            NextToken;
            break;
          } else if (at == AtomNan) {
            t = MkFloatTerm(NAN);
            NextToken;
            break;
          }
        }
#endif
      }
      if (opprio <= prio) {
        /* try to parse as a prefix operator */
        TRY(
            /* build appl on the heap */
            func = Yap_MkFunctor(AtomOfTerm(t), 1); if (func == NULL) {
              syntax_msg("line %d: Heap Overflow", LOCAL_tokptr->TokLine);
              FAIL;
            } t = ParseTerm(oprprio, FailBuff, enc, cmod PASS_REGS);
            t = Yap_MkApplTerm(func, 1, &t);
            /* check for possible overflow against local stack */
            if (HR > ASP - 4096) {
              syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
              FAIL;
            } curprio = opprio;
            , break;)
      }
    }
    if (LOCAL_tokptr->Tok == Ord(Ponctuation_tok) &&
        LOCAL_tokptr->TokInfo == Terml)
      t = ParseArgs(AtomOfTerm(t), TermEndBracket, FailBuff, 0L, enc,
                    cmod PASS_REGS);
    break;

  case Number_tok:
    t = LOCAL_tokptr->TokInfo;
    NextToken;
    break;

  case String_tok: /* build list on the heap */
    t = LOCAL_tokptr->TokInfo;
    NextToken;
    break;

  case Var_tok:
    varinfo = (VarEntry *)(LOCAL_tokptr->TokInfo);
    if ((t = varinfo->VarAdr) == TermNil) {
      t = varinfo->VarAdr = MkVarTerm();
    }
    NextToken;
    break;

  case Error_tok:
    syntax_msg("line %d: found ill-formed \"%s\"", LOCAL_tokptr->TokLine,
               Yap_tokText(LOCAL_tokptr));
    FAIL;

  case Ponctuation_tok:

    switch (RepAtom(AtomOfTerm(LOCAL_tokptr->TokInfo))->StrOfAE[0]) {
    case '(':
    case 'l': /* non solo ( */
      NextToken;
      t = ParseTerm(GLOBAL_MaxPriority, FailBuff, enc, cmod PASS_REGS);
      checkfor(TermEndBracket, FailBuff, enc PASS_REGS);
      break;
    case '[':
      NextToken;
      if (LOCAL_tokptr->Tok == Ponctuation_tok &&
          LOCAL_tokptr->TokInfo == TermEndSquareBracket) {
        t = TermNil;
        NextToken;
        break;
      }
      t = ParseList(FailBuff, enc, cmod PASS_REGS);
      checkfor(TermEndSquareBracket, FailBuff, enc PASS_REGS);
      break;
    case '{':
      NextToken;
      if (LOCAL_tokptr->Tok == Ponctuation_tok &&
          (int)LOCAL_tokptr->TokInfo == TermEndCurlyBracket) {
        t = MkAtomTerm(AtomBraces);
        NextToken;
        break;
      }
      t = ParseTerm(GLOBAL_MaxPriority, FailBuff, enc, cmod PASS_REGS);
      t = Yap_MkApplTerm(FunctorBraces, 1, &t);
      /* check for possible overflow against local stack */
      if (HR > ASP - 4096) {
        syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
        FAIL;
      }
      checkfor(TermEndCurlyBracket, FailBuff, enc PASS_REGS);
      break;
    default:
      syntax_msg("line %d: unexpected ponctuation signal %s",
                 LOCAL_tokptr->TokLine, Yap_tokRep(LOCAL_tokptr));
      FAIL;
    }
    break;

#if QQ
  case QuasiQuotes_tok: {
    qq_t *qq = (qq_t *)(LOCAL_tokptr->TokInfo);
    term_t pv, positions = LOCAL_subtpos, to;
    Atom at;
    Term tn;
    CELL *tnp;

    // from SWI, enter the list
    /* prepare (if we are the first in term) */
    if (!LOCAL_varnames)
      LOCAL_varnames = PL_new_term_ref();
    if (!LOCAL_qq) {
      if (LOCAL_quasi_quotations) {
        LOCAL_qq = LOCAL_quasi_quotations;
      } else {
        if (!(LOCAL_qq = PL_new_term_ref()))
          return FALSE;
      }
      //  create positions term
      if (positions) {
        if (!(pv = PL_new_term_refs(3)) ||
            !PL_unify_term(positions, PL_FUNCTOR,
                           FUNCTOR_quasi_quotation_position5, PL_INTPTR,
                           qq->start.charno, PL_VARIABLE, PL_TERM,
                           pv + 0, // leave three open slots
                           PL_TERM, pv + 1, PL_TERM, pv + 2))
          return FALSE;
      } else
        pv = 0;
      /* push type */

      if (!(LOCAL_qq_tail = PL_copy_term_ref(LOCAL_qq)))
        return FALSE;
    }

    NextToken;
    t = ParseTerm(GLOBAL_MaxPriority, FailBuff, enc, cmod PASS_REGS);
    if (LOCAL_tokptr->Tok != QuasiQuotes_tok) {
      syntax_msg("expected to find quasi quotes, got \"%s\"", ,
                 Yap_tokText(LOCAL_tokptr));
      FAIL;
    }
    if (!(is_quasi_quotation_syntax(t, &at))) {
      syntax_msg("bad quasi quotation syntax, at \"%s\"",
                 Yap_tokText(LOCAL_tokptr));
      FAIL;
    }
    /* Arg 2: the content */
    tn = Yap_MkNewApplTerm(SWIFunctorToFunctor(FUNCTOR_quasi_quotation4), 4);
    tnp = RepAppl(tn) + 1;
    tnp[0] = MkAtomTerm(at);
    if (!get_quasi_quotation(Yap_InitSlot(ArgOfTerm(2, tn)), &qq->text,
                             qq->text + strlen((const char *)qq->text))) {
      syntax_msg("could not get quasi quotation, at \"%s\"",
                 Yap_tokText(LOCAL_tokptr));
      FAIL;
    }
    if (positions) {
      intptr_t qqend = qq->end.charno;

      // set_range_position(positions, -1, qqend PASS_LD);
      if (!PL_unify_term(Yap_InitSlot(ArgOfTerm(2, t)), PL_FUNCTOR,
                         FUNCTOR_minus2, PL_INTPTR,
                         qq->mid.charno + 2,    /* end of | token */
                         PL_INTPTR, qqend - 2)) /* end minus "|}" */
        syntax_msg("failed to unify quasi quotation, at \"%s\"",
                   Yap_tokText(LOCAL_tokptr));
      FAIL;
    }

    tnp[2] = Yap_GetFromSlot(LOCAL_varnames); /* Arg 3: the var dictionary */
    /* Arg 4: the result */
    t = ArgOfTerm(4, tn);
    if (!(to = PL_new_term_ref()) ||
        !PL_unify_list(LOCAL_qq_tail, to, LOCAL_qq_tail) ||
        !PL_unify(to, Yap_InitSlot(tn))) {
      syntax_msg("failed to unify quasi quotation, at \"%s\"",
                 Yap_tokRep(LOCAL_tokptr, enc));
      FAIL;
    }
  }
#endif
    NextToken;
    break;
  default:
    syntax_msg("line %d: expected operator, got \'%s\'", LOCAL_tokptr->TokLine,
               Yap_tokText(LOCAL_tokptr));
    FAIL;
  }

  /* main loop to parse infix and posfix operators starts here */
  while (true) {
    Atom name;
    if (LOCAL_tokptr->Tok == Ord(Name_tok) &&
        Yap_HasOp((name = AtomOfTerm(LOCAL_tokptr->TokInfo)))) {
      Atom save_opinfo = opinfo = name;
      if (IsInfixOp(save_opinfo, &opprio, &oplprio, &oprprio, cmod PASS_REGS) &&
          opprio <= prio && oplprio >= curprio) {
        /* try parsing as infix operator */
        Volatile int oldprio = curprio;
        TRY3(
            func = Yap_MkFunctor(save_opinfo, 2); if (func == NULL) {
              syntax_msg("line %d: Heap Overflow", LOCAL_tokptr->TokLine);
              FAIL;
            } NextToken;
            {
              Term args[2];
              args[0] = t;
              args[1] = ParseTerm(oprprio, FailBuff, enc, cmod PASS_REGS);
              t = Yap_MkApplTerm(func, 2, args);
              /* check for possible overflow against local stack */
              if (HR > ASP - 4096) {
                syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
                FAIL;
              }
            },
            curprio = opprio;
            opinfo = save_opinfo; continue;, opinfo = save_opinfo;
            curprio = oldprio;)
      }
      if (IsPosfixOp(opinfo, &opprio, &oplprio, cmod PASS_REGS) &&
          opprio <= prio && oplprio >= curprio) {
        /* parse as posfix operator */
        Functor func = Yap_MkFunctor(AtomOfTerm(LOCAL_tokptr->TokInfo), 1);
        if (func == NULL) {
          syntax_msg("line %d: Heap Overflow", LOCAL_tokptr->TokLine);
          FAIL;
        }
        t = Yap_MkApplTerm(func, 1, &t);
        /* check for possible overflow against local stack */
        if (HR > ASP - 4096) {
          syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
          FAIL;
        }
        curprio = opprio;
        NextToken;
        continue;
      }
      break;
    }
    if (LOCAL_tokptr->Tok == Ord(Ponctuation_tok)) {
      if (LOCAL_tokptr->TokInfo == TermComma && prio >= 1000 &&
          curprio <= 999) {
        Volatile Term args[2];
        NextToken;
        args[0] = t;
        args[1] = ParseTerm(1000, FailBuff, enc, cmod PASS_REGS);
        t = Yap_MkApplTerm(FunctorComma, 2, args);
        /* check for possible overflow against local stack */
        if (HR > ASP - 4096) {
          syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
          FAIL;
        }
        curprio = 1000;
        continue;
      } else if (LOCAL_tokptr->TokInfo == TermVBar &&
                 IsInfixOp(AtomVBar, &opprio, &oplprio, &oprprio,
                           cmod PASS_REGS) &&
                 opprio <= prio && oplprio >= curprio) {
        Volatile Term args[2];
        NextToken;
        args[0] = t;
        args[1] = ParseTerm(oprprio, FailBuff, enc, cmod PASS_REGS);
        t = Yap_MkApplTerm(FunctorVBar, 2, args);
        /* check for possible overflow against local stack */
        if (HR > ASP - 4096) {
          syntax_msg("line %d: Stack Overflow", LOCAL_tokptr->TokLine);
          FAIL;
        }
        curprio = opprio;
        continue;
      } else if (LOCAL_tokptr->TokInfo == TermBeginBracket &&
                 IsPosfixOp(AtomEmptyBrackets, &opprio, &oplprio,
                            cmod PASS_REGS) &&
                 opprio <= prio && oplprio >= curprio) {
        t = ParseArgs(AtomEmptyBrackets, TermEndBracket, FailBuff, t, enc,
                      cmod PASS_REGS);
        curprio = opprio;
        continue;
      } else if (LOCAL_tokptr->TokInfo == TermBeginSquareBracket &&
                 IsPosfixOp(AtomEmptySquareBrackets, &opprio, &oplprio,
                            cmod PASS_REGS) &&
                 opprio <= prio && oplprio >= curprio) {
        t = ParseArgs(AtomEmptySquareBrackets, TermEndSquareBracket, FailBuff,
                      t, enc, cmod PASS_REGS);
        t = MakeAccessor(t, FunctorEmptySquareBrackets PASS_REGS);
        curprio = opprio;
        continue;
      } else if (LOCAL_tokptr->TokInfo == TermBeginCurlyBracket &&
                 IsPosfixOp(AtomBraces, &opprio, &oplprio, cmod PASS_REGS) &&
                 opprio <= prio && oplprio >= curprio) {
        t = ParseArgs(AtomBraces, TermEndCurlyBracket, FailBuff, t, enc,
                      cmod PASS_REGS);
        t = MakeAccessor(t, FunctorBraces PASS_REGS);
        curprio = opprio;
        continue;
      }
    }
    if (LOCAL_tokptr->Tok <= Ord(String_tok)) {
      syntax_msg("line %d: expected operator, got \'%s\'", LOCAL_tokptr->TokLine,
                 Yap_tokText(LOCAL_tokptr));
      FAIL;
    }
    break;
  }
  return t;
}

Term Yap_Parse(UInt prio, encoding_t enc, Term cmod) {
  CACHE_REGS
  // ensure that if we throw an exception
  // t will be 0.
  Volatile Term t = 0;
  JMPBUFF FailBuff;
  yhandle_t sls = Yap_StartSlots();
  LOCAL_toktide = LOCAL_tokptr;

  if (!sigsetjmp(FailBuff.JmpBuff, 0)) {

    t = ParseTerm(prio, &FailBuff, enc, cmod PASS_REGS);
#if DEBUG
    if (GLOBAL_Option['p' - 'a' + 1]) {
      Yap_DebugPutc(stderr, '[');
      if (t == 0)
        Yap_DebugPlWrite(MkIntTerm(0));
      else
        Yap_DebugPlWrite(t);
      Yap_DebugPutc(stderr, ']');
      Yap_DebugPutc(stderr, '\n');
    }
#endif
    Yap_CloseSlots(sls);
  }
  if (LOCAL_tokptr != NULL && LOCAL_tokptr->Tok != Ord(eot_tok)) {
    LOCAL_Error_TYPE = SYNTAX_ERROR;
    if (LOCAL_tokptr->TokNext) {
      LOCAL_ErrorMessage = "bracket or operator expected.";
    } else {
      LOCAL_ErrorMessage = "term  must end with . or EOF.";
    }
    t = 0;
  }
  if (t != 0 && LOCAL_Error_TYPE == SYNTAX_ERROR) {
    LOCAL_Error_TYPE = YAP_NO_ERROR;
    LOCAL_ErrorMessage = NULL;
  }
  //    if (LOCAL_tokptr->Tok != Ord(eot_tok))
  //  return (0L);
  return t;
}

//! @}
