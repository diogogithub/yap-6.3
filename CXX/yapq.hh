
/**
 *   @file yapq.hh
 *
 *   @defgroup yap-cplus-query-handling Query Handling in the YAP interface.
 *   @brief Engine and Query Management
 *
 *   @ingroup yap-cplus-interface
 *
 * @{
 *
 * These classes wrap  engine and query. An engine is an environment where we can rum
 * Prolog, that is, where we can run queries.
 *
 * Also, supports callbacks and engine configuration.
 *
 */

#ifndef YAPQ_HH
#define YAPQ_HH 1

class X_API YAPPredicate;

/**
   Queries and engines
*/

#if __ANDROID__

#endif

/**
 * @brief Queries
 *
 * interface to a YAP Query;
 * uses an SWI-like status info internally.
 */
class X_API YAPQuery : public YAPPredicate
{
  bool q_open;
  int q_state;
  yhandle_t q_g, q_handles;
  struct yami *q_p, *q_cp;
  sigjmp_buf q_env;
  int q_flags;
  YAP_dogoalinfo q_h;
  YAPQuery *oq;
  YAPPairTerm names;
  YAPTerm goal;
  // temporaries
  Term tnames, tgoal ;

  inline void setNext() {  // oq = LOCAL_execution;
  //  LOCAL_execution = this;
  q_open = true;
  q_state = 0;
  q_flags = true; // PL_Q_PASS_EXCEPTION;

  q_p = P;
  q_cp = CP;
  // make sure this is safe
  q_handles = LOCAL_CurSlot;
};


  void openQuery( Term *ts);

PredEntry *rewriteUndefQuery();

public:
YAPQuery() {
  goal = TermTrue;
  openQuery( nullptr);
};
  /// main constructor, uses a predicate and an array of terms
  ///
  /// It is given a YAPPredicate _p_ , and an array of terms that must have at
  /// least
  /// the same arity as the functor.
  YAPQuery(YAPPredicate p, YAPTerm t[]);
  /// full constructor,
  ///
  ///
  /// It is given a functor, module, and an array of terms that must have at
  /// least
  /// the same arity as the functor.
  YAPQuery(YAPFunctor f, YAPTerm mod, YAPTerm t[]);
  /// functor/term constructor,
  ///
  /// It is given a functor, and an array of terms that must have at least
  /// the same arity as the functor. Works within the current module.
  //YAPQuery(YAPFunctor f, YAPTerm t[]);
  /// string constructor without varnames
  ///
  /// It is given a string, calls the parser and obtains a Prolog term that
  /// should be a callable
  /// goal.
  inline YAPQuery(const char *s) : YAPPredicate(s, tgoal, tnames)
  {
    CELL *qt = nullptr;
      __android_log_print(ANDROID_LOG_INFO, "YAPDroid", "got game %d",
                          LOCAL_CurSlot);
      if (!ap)
          return;
      __android_log_print(ANDROID_LOG_INFO, "YAPDroid", "%s", names.text());
      goal = YAPTerm(tgoal);
      if (IsPairTerm(tgoal)) {
        qt = RepPair(tgoal);
        tgoal = Yap_MkApplTerm(Yap_MkFunctor(Yap_LookupAtom("consult"), 1),1,qt);
      } else if (IsApplTerm(tgoal)) {
        Functor f = FunctorOfTerm(tgoal);
        if (!IsExtensionFunctor(f)) {
          qt = RepAppl(tgoal)+1;
        }
      }
      names = YAPPairTerm(tnames);
      openQuery(qt);
  };
  // inline YAPQuery() : YAPPredicate(s, tgoal, tnames)
  // {
  //     __android_log_print(ANDROID_LOG_INFO, "YAPDroid", "got game %ld",
  //                         LOCAL_CurSlot);
  //     if (!ap)
  //         return;
  //     __android_log_print(ANDROID_LOG_INFO, "YAPDroid", "%s", vnames.text());
  //     goal = YAPTerm(tgoal);
  //     names = YAPPairTerm(tnames);
  //     openQuery(tgoal);
  // };
  /// string constructor with just an atom
  ///
  /// It i;
  ///};
  YAPQuery(YAPTerm t);
  /// set flags for query execution, currently only for exception handling
  void setFlag(int flag) { q_flags |= flag; }
  /// reset flags for query execution, currently only for exception handling
void resetFlag(int flag) { q_flags &= ~flag; }
/// first query
///
/// actually implemented by calling the next();
inline bool first() { return next(); }
/// ask for the next solution of the current query
/// same call for every solution
bool next();
/// does this query have open choice-points?
/// or is it deterministic?
bool deterministic();
/// represent the top-goal
const char *text();
/// remove alternatives in the current search space, and finish the current
/// query
/// finish the current query: undo all bindings.
void close();
/// query variables.
void cut();
Term namedVars() {return  names.term(); };
YAPPairTerm namedVarTerms() {return  names; };
/// query variables, but copied out
std::vector<Term> namedVarsVector() {
  return names.listToArray(); };
/// convert a ref to a binding.
YAPTerm getTerm(yhandle_t t);
/// simple YAP Query;
/// just calls YAP and reports success or failure, Useful when we just
/// want things done, eg YAPCommand("load_files(library(lists), )")
inline bool command()
{
  bool rc = next();
  close();
  return rc;
};
};

// Java support

/// This class implements a callback Prolog-side. It will be inherited by the
/// Java or Python
/// class that actually implements the callback.
class X_API YAPCallback
{
public:
  virtual ~YAPCallback() {}
  virtual void run() { LOG("callback"); }
  virtual void run(char *s) {}
};

void YAP_init_args::YAP_init_args()
{
      Yap_InitDefaults(this, NULL, 0, NULL);
} ;

/// @brief Setup all arguments to a new engine
struct X_API YAPEngineArgs: YAP_init_args {
public:

    YAPEngineArgs(): yap_boot_params() {
   #if YAP_PYTHON
      Embedded = true;
    python_in_python = Py_IsInitialized();
#endif
    };

  inline void setEmbedded( bool fl )
  {
    Embedded = fl;
  };

  inline bool getEmbedded(  )
  {
    return Embedded;
  };

  inline void setStackSize( bool fl )
  {
    StackSize = fl;
  };

  inline bool getStackSize(  )
  {
    return StackSize;
  };

  inline void setTrailSize( bool fl )
  {
    TrailSize = fl;
  };

  inline bool getTrailSize(  )
  {
    return TrailSize;
  };

  inline bool getMStackSize(  )
  {
    return StackSize;
  };

  inline void setMaxTrailSize( bool fl )
  {
    MaxTrailSize = fl;
  };

  inline bool getMaxTrailSize(  )
  {
    return MaxTrailSize;
  };

  inline void setYapLibDir( const char * fl )
  {
    YapLibDir = (const char *)malloc(strlen(fl)+1);
    strcpy((char *)YapLibDir, fl);
  };

  inline const char * getYapLibDir(  )
  {
    return YapLibDir;
  };

    inline void setYapShareDir( const char * fl )
  {
    YapShareDir = (const char *)malloc(strlen(fl)+1);
    strcpy((char *)YapShareDir, fl);
  };

  inline const char * getYapShareDir(  )
  {
    return YapShareDir;
  };

  inline void setSavedState( const char * fl )
  {
    SavedState = (const char *)malloc(strlen(fl)+1);
    strcpy((char *)SavedState, fl);
  };

  inline const char * getSavedState(  )
  {
    return SavedState;
  };

  inline void setYapPrologBootFile( const char * fl )
  {
    YapPrologBootFile = (const char *)malloc(strlen(fl)+1);
    strcpy((char *)YapPrologBootFile, fl);
};

  inline const char * getYapPrologBootFile(  )
  {
    return YapPrologBootFile;
  };

  inline void setYapPrologGoal( const char * fl )
  {
    YapPrologGoal = fl;
  };

  inline const char * getYapPrologGoal(  )
  {
    return YapPrologGoal;
  };

  inline void setYapPrologTopLevelGoal( const char * fl )
  {
    YapPrologTopLevelGoal = fl;
  };

  inline const char * getYapPrologTopLevelGoal(  )
  {
    return YapPrologTopLevelGoal;
  };

  inline void setHaltAfterConsult( bool fl )
  {
    HaltAfterConsult = fl;
  };

  inline bool getHaltAfterConsult(  )
  {
    return HaltAfterConsult;
  };

  inline void setFastBoot( bool fl )
  {
    FastBoot = fl;
  };

    inline bool getFastBoot(  )
    {
        return FastBoot;
    };

#if __ANDROID__
    //> export ResoourceManager
    inline void setAssetManager( AAssetManager *mgr )
    {
        assetManager = mgr;
    };
#endif

  inline void setArgc( int  fl )
  {
    Argc = fl;
  };

  inline int getArgc(  )
  {
    return Argc;
  };

  inline void setArgv( char ** fl )
  {
    Argv = fl;
  };

  inline char ** getArgv(  )
  {
    return Argv;
  };

};


/**
 * @brief YAP Engine: takes care of the execution environment
 where we can go executing goals.
 *
 *
 */
  class YAPEngine
{
private:
  YAPEngineArgs *engine_args;
  YAPCallback *_callback;
  YAPError yerror;
  void doInit(YAP_file_type_t BootMode);
  YAP_dogoalinfo q;
  PredEntry *rewriteUndefEngineQuery(PredEntry *ap, Term t, Term tmod);

      public :
      /// construct a new engine; may use a variable number of arguments
      YAPEngine(YAPEngineArgs *cargs)
  {
    engine_args = cargs;
    //doInit(cargs->boot_file_type);
    doInit(YAP_QLY);
  }; /// construct a new engine, including aaccess to callbacks
  /// construct a new engine using argc/argv list of arguments
  YAPEngine(int argc, char *argv[],
	    YAPCallback *callback = (YAPCallback *)NULL);
  /// kill engine
  ~YAPEngine() { delYAPCallback(); };
  /// remove current callback
  void delYAPCallback() { _callback = 0; };
  /// set a new callback
  void setYAPCallback(YAPCallback *cb)
  {
    delYAPCallback();
    _callback = cb;
  };
  /// execute the callback.
  ////void run() { if (_callback) _callback.run(); }
  /// execute the callback with a text argument.
  void run(char *s)
  {
    if (_callback)
      _callback->run(s);
  }
  /// stop yap
  void close() { Yap_exit(0); }

  /// execute the callback with a text argument.
  bool hasError() { return LOCAL_Error_TYPE != YAP_NO_ERROR; }
  /// build a query on the engine
  YAPQuery *query(const char *s) { return new YAPQuery(s); };
  /// build a query from a term
  YAPQuery *query(YAPTerm t) { return new YAPQuery(t); };
  /// build a query from a Prolog term (internal)
  YAPQuery *qt(Term t) { return new YAPQuery(YAPTerm(t)); };
  /// current module for the engine
  YAPModule currentModule() { return YAPModule(); }
  /// given a handle, fetch a term from the engine
  inline YAPTerm getTerm(yhandle_t h) { return YAPTerm(h); }
  /// current directory for the engine
  bool call(YAPPredicate ap, YAPTerm ts[]);
  /// current directory for the engine
  bool goalt(YAPTerm Yt) { return Yt.term(); };
  /// current directory for the engine
  bool mgoal(Term t, Term tmod);
  /// current directory for the engine

  bool goal(Term t)
  {
    return mgoal(t, CurrentModule);
  }
  /// reset Prolog state
  void reSet();
  /// assune that there are no stack pointers, just release memory
  // for last execution
  void release();

  const char *currentDir()
  {
    char dir[1024];
    std::string s = Yap_getcwd(dir, 1024 - 1);
    return s.c_str();
  };
  /// report YAP version as a string
  const char *version()
  {
    std::string s = Yap_version();
    return s.c_str();
  };
  //> call a deterninistic predicate: the user will construct aterm of
  //> arity N-1. YAP adds an extra variable which will have the
  //> output.
  YAPTerm funCall(YAPTerm t) { return YAPTerm(fun(t.term())); };
  Term fun(Term t);
  Term fun(YAPTerm t) { return fun(t.term()); };
  //> set a StringFlag, usually a path
  //>
  bool setStringFlag(std::string arg, std::string path)
  {
    return setYapFlag(MkAtomTerm(Yap_LookupAtom(arg.data())), MkAtomTerm(Yap_LookupAtom(path.data())));
  };

  Term  top_level( std::string s);
  Term next_answer(YAPQuery * &Q);

  };

#endif /* YAPQ_HH */

    /// @}
