#ifndef PL_YAP_H
#define PL_YAP_H

#ifdef __YAP_PROLOG__

/* depends on tag schema, but 4 should always do */
#define LMASK_BITS	4		/* total # mask bits */

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#define SIZE_VOIDP SIZEOF_INT_P


#if SIZEOF_LONG_INT==4
#define INT64_FORMAT "%lld"
#else
#define INT64_FORMAT "%ld"
#endif
#define INTBITSIZE (sizeof(int)*8)

typedef YAP_Term	        Module;
typedef YAP_Term	       *Word;		/* Anonymous 4 byte object */
typedef YAP_Atom		Atom;
typedef YAP_Term		(*Func)();	/* foreign functions */

const char *Yap_GetCurrentPredName(void);
YAP_Int     Yap_GetCurrentPredArity(void);


extern atom_t codeToAtom(int chrcode);

#define valTermRef(t)    ((Word)YAP_AddressFromSlot(t))

#include "pl-codelist.h"

//move this to SWI

#define GP_CREATE	2		/* create (in this module) */

#ifndef HAVE_MBSCOLL
COMMON(int) mbscoll(const char *s1, const char *s2);
#endif


#ifndef HAVE_MBSCASECOLL
COMMON(int) mbscasecoll(const char *s1, const char *s2);
#endif

COMMON(atom_t) 		TemporaryFile(const char *id, int *fdp);
COMMON(char *) 		Getenv(const char *, char *buf, size_t buflen);

/*** memory allocation stuff: SWI wraps around malloc  */

#define allocHeap(X) YAP_AllocSpaceFromYap(X)

#define freeHeap(X,Size) YAP_FreeSpaceFromYap(X)

#define stopItimer()

COMMON(word) 		pl_print(term_t term);
COMMON(word) 		pl_write(term_t term);
COMMON(word) 		pl_write_canonical(term_t term);
COMMON(word) 		pl_write_term(term_t term, term_t options);
COMMON(word) 		pl_writeq(term_t term);

static inline int
get_procedure(term_t descr, predicate_t *proc, term_t he, int f) {
  YAP_Term t = YAP_GetFromSlot(descr);

  if (YAP_IsVarTerm(t)) return 0;
  if (YAP_IsAtomTerm(t)) 
    *proc = YAP_Predicate(YAP_AtomOfTerm(t),0,YAP_CurrentModule());
  else if (YAP_IsApplTerm(t)) {
    YAP_Functor f = YAP_FunctorOfTerm(t);
    *proc = YAP_Predicate(YAP_NameOfFunctor(f),YAP_ArityOfFunctor(f),YAP_CurrentModule());
  }
  return 1;
}

COMMON(intptr_t) 	lengthList(term_t list, int errors);
COMMON(int)		promoteToFloatNumber(Number n);
COMMON(char *) 		PrologPath(const char *ospath, char *plpath, size_t len);
COMMON(char *) 		ExpandOneFile(const char *spec, char *file);
COMMON(char *) 		AbsoluteFile(const char *spec, char *path);
COMMON(char *) 		BaseName(const char *f);
COMMON(bool) 		ChDir(const char *path);
COMMON(char *) 		OsPath(const char *plpath, char *ospath);
COMMON(bool) 		ChDir(const char *path);
COMMON(int) 		DeleteTemporaryFile(atom_t name);
COMMON(int) 		IsAbsolutePath(const char *spec);

/* TBD */

extern word globalString(size_t size, char *s);
extern word globalWString(size_t size, wchar_t *s);

static inline word
valHandle(term_t tt)
{
  return (word)YAP_GetFromSlot(tt);
}

YAP_Int YAP_PLArityOfSWIFunctor(functor_t f);
YAP_Atom YAP_AtomFromSWIAtom(atom_t at);
PL_blob_t*	YAP_find_blob_type(YAP_Atom at);


#define arityFunctor(f) YAP_PLArityOfSWIFunctor(f)

#define stringAtom(w)	YAP_AtomName((YAP_Atom)(w))
#define isInteger(A) (YAP_IsIntTerm((A)) && YAP_IsBigNumTerm((A)))
#define isString(A) FALSE
#define isAtom(A) YAP_IsAtomTerm((A))
#define isList(A) YAP_IsPairTerm((A))
#define isNil(A) ((A) == YAP_TermNil())
#define isReal(A) YAP_IsFloatTerm((A))
#define isFloat(A) YAP_IsFloatTerm((A))
#define isVar(A) YAP_IsVarTerm((A))
#define valReal(w) YAP_FloatOfTerm((w))
#define valFloat(w) YAP_FloatOfTerm((w))
#define AtomLength(w) YAP_AtomNameLength(w)
#define atomValue(atom) YAP_AtomFromSWIAtom(atom)
#define atomFromTerm(term) YAP_AtomOfTerm(term)
#define atomName(atom) ((char *)YAP_AtomName(atom))
#define nameOfAtom(atom) ((char *)YAP_AtomName(atom))
#define atomLength(atom) YAP_AtomNameLength(atom)
#define atomBlobType(at) YAP_find_blob_type(at)
#define argTermP(w,i) ((Word)((YAP_ArgsOfTerm(w)+(i))))
#define deRef(t)
#define canBind(t) FALSE  // VSC: to implement
#define MODULE_user YAP_ModuleUser()
#define _PL_predicate(A,B,C,D) PL_predicate(A,B,C)
#define predicateHasClauses(A) (YAP_NumberOfClausesForPredicate((YAP_PredEntryPtr)A) != 0)
#define lookupModule(A) ((module_t)PL_new_module(A))
#define charEscapeWriteOption(A) FALSE  // VSC: to implement
#define skip_list(A,B) YAP_SkipList(A,B)
#define wordToTermRef(A) YAP_InitSlot(*(A))
#define isTaggedInt(A) YAP_IsIntTerm(A)
#define valInt(A) YAP_IntOfTerm(A)

#define clearNumber(n)

inline static int
charCode(YAP_Term w)
{ if ( YAP_IsAtomTerm(w) )
    { 
      Atom a = atomValue(w);

      if ( YAP_AtomNameLength(a) == 1) {
	if (YAP_IsWideAtom(a)) {
	  return YAP_WideAtomName(a)[0];
	}
	return YAP_AtomName(a)[0];
      }
    }
  return -1;
}



#endif /* __YAP_PROLOG__ */

#if IN_PL_OS_C
static int
stripostfix(const char *s, const char *e)
{ size_t ls = strlen(s);
  size_t le = strlen(e);

  if ( ls >= le )
    return strcasecmp(&s[ls-le], e) == 0;

  return FALSE;
} 
#endif


#endif /* PL_YAP_H */
