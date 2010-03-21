/******************************************************************************\
*                                                                              *
*    SimpleCUDD library (www.cs.kuleuven.be/~theo/tools/simplecudd.html)       *
*  SimpleCUDD was developed at Katholieke Universiteit Leuven(www.kuleuven.be) *
*                                                                              *
*  Copyright T. Mantadelis and Katholieke Universiteit Leuven 2008             *
*                                                                              *
*  Author: Theofrastos Mantadelis                                              *
*  File: simplecudd.h                                                          *
*                                                                              *
********************************************************************************
*                                                                              *
*        The "Artistic License"                                                *
*                                                                              *
*         Preamble                                                             *
*                                                                              *
* The intent of this document is to state the conditions under which a         *
* Package may be copied, such that the Copyright Holder maintains some         *
* semblance of artistic control over the development of the package,           *
* while giving the users of the package the right to use and distribute        *
* the Package in a more-or-less customary fashion, plus the right to make      *
* reasonable modifications.                                                    *
*                                                                              *
* Definitions:                                                                 *
*                                                                              *
*   "Package" refers to the collection of files distributed by the             *
*   Copyright Holder, and derivatives of that collection of files              *
*   created through textual modification.                                      *
*                                                                              *
*   "Standard Version" refers to such a Package if it has not been             *
*   modified, or has been modified in accordance with the wishes               *
*   of the Copyright Holder as specified below.                                *
*                                                                              *
*   "Copyright Holder" is whoever is named in the copyright or                 *
*   copyrights for the package.                                                *
*                                                                              *
*   "You" is you, if you're thinking about copying or distributing             *
*   this Package.                                                              *
*                                                                              *
*   "Reasonable copying fee" is whatever you can justify on the                *
*   basis of media cost, duplication charges, time of people involved,         *
*   and so on.  (You will not be required to justify it to the                 *
*   Copyright Holder, but only to the computing community at large             *
*   as a market that must bear the fee.)                                       *
*                                                                              *
*   "Freely Available" means that no fee is charged for the item               *
*   itself, though there may be fees involved in handling the item.            *
*   It also means that recipients of the item may redistribute it              *
*   under the same conditions they received it.                                *
*                                                                              *
* 1. You may make and give away verbatim copies of the source form of the      *
* Standard Version of this Package without restriction, provided that you      *
* duplicate all of the original copyright notices and associated disclaimers.  *
*                                                                              *
* 2. You may apply bug fixes, portability fixes and other modifications        *
* derived from the Public Domain or from the Copyright Holder.  A Package      *
* modified in such a way shall still be considered the Standard Version.       *
*                                                                              *
* 3. You may otherwise modify your copy of this Package in any way, provided   *
* that you insert a prominent notice in each changed file stating how and      *
* when you changed that file, and provided that you do at least ONE of the     *
* following:                                                                   *
*                                                                              *
*    a) place your modifications in the Public Domain or otherwise make them   *
*    Freely Available, such as by posting said modifications to Usenet or      *
*    an equivalent medium, or placing the modifications on a major archive     *
*    site such as uunet.uu.net, or by allowing the Copyright Holder to include *
*    your modifications in the Standard Version of the Package.                *
*                                                                              *
*    b) use the modified Package only within your corporation or organization. *
*                                                                              *
*    c) rename any non-standard executables so the names do not conflict       *
*    with standard executables, which must also be provided, and provide       *
*    a separate manual page for each non-standard executable that clearly      *
*    documents how it differs from the Standard Version.                       *
*                                                                              *
*    d) make other distribution arrangements with the Copyright Holder.        *
*                                                                              *
* 4. You may distribute the programs of this Package in object code or         *
* executable form, provided that you do at least ONE of the following:         *
*                                                                              *
*    a) distribute a Standard Version of the executables and library files,    *
*    together with instructions (in the manual page or equivalent) on where    *
*    to get the Standard Version.                                              *
*                                                                              *
*    b) accompany the distribution with the machine-readable source of         *
*    the Package with your modifications.                                      *
*                                                                              *
*    c) give non-standard executables non-standard names, and clearly          *
*    document the differences in manual pages (or equivalent), together        *
*    with instructions on where to get the Standard Version.                   *
*                                                                              *
*    d) make other distribution arrangements with the Copyright Holder.        *
*                                                                              *
* 5. You may charge a reasonable copying fee for any distribution of this      *
* Package.  You may charge any fee you choose for support of this              *
* Package.  You may not charge a fee for this Package itself.  However,        *
* you may distribute this Package in aggregate with other (possibly            *
* commercial) programs as part of a larger (possibly commercial) software      *
* distribution provided that you do not advertise this Package as a            *
* product of your own.  You may embed this Package's interpreter within        *
* an executable of yours (by linking); this shall be construed as a mere       *
* form of aggregation, provided that the complete Standard Version of the      *
* interpreter is so embedded.                                                  *
*                                                                              *
* 6. The scripts and library files supplied as input to or produced as         *
* output from the programs of this Package do not automatically fall           *
* under the copyright of this Package, but belong to whoever generated         *
* them, and may be sold commercially, and may be aggregated with this          *
* Package.  If such scripts or library files are aggregated with this          *
* Package via the so-called "undump" or "unexec" methods of producing a        *
* binary executable image, then distribution of such an image shall            *
* neither be construed as a distribution of this Package nor shall it          *
* fall under the restrictions of Paragraphs 3 and 4, provided that you do      *
* not represent such an executable image as a Standard Version of this         *
* Package.                                                                     *
*                                                                              *
* 7. C subroutines (or comparably compiled subroutines in other                *
* languages) supplied by you and linked into this Package in order to          *
* emulate subroutines and variables of the language defined by this            *
* Package shall not be considered part of this Package, but are the            *
* equivalent of input as in Paragraph 6, provided these subroutines do         *
* not change the language in any way that would cause it to fail the           *
* regression tests for the language.                                           *
*                                                                              *
* 8. Aggregation of this Package with a commercial distribution is always      *
* permitted provided that the use of this Package is embedded; that is,        *
* when no overt attempt is made to make this Package's interfaces visible      *
* to the end user of the commercial distribution.  Such use shall not be       *
* construed as a distribution of this Package.                                 *
*                                                                              *
* 9. The name of the Copyright Holder may not be used to endorse or promote    *
* products derived from this software without specific prior written           *
* permission.                                                                  *
*                                                                              *
* 10. THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR              *
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED               *
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.          *
*                                                                              *
*         The End                                                              *
*                                                                              *
\******************************************************************************/
/* modified by Fabrizio Riguzzi in 2009 for dealing with multivalued variables
instead of variables or their negation, the script can contain equations of the 
form
variable=value
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "util.h"
#include "cudd.h"
#include "cuddInt.h"
#include "general.h"

#define IsHigh(manager, node)  HIGH(manager) == node
#define IsLow(manager, node)   LOW(manager) == node
#define HIGH(manager)          Cudd_ReadOne(manager)
#define LOW(manager)           Cudd_Not(Cudd_ReadOne(manager))
#define NOT(node)              Cudd_Not(node)
#define GetIndex(node)         Cudd_NodeReadIndex(node)
#define GetMVar(manager, index, value, varmap) equality(manager,index,value,varmap)//Cudd_bddIthVar(manager, index)
#define GetVar(manager, index) Cudd_bddIthVar(manager, index)
#define NewVar(manager)        Cudd_bddNewVar(manager)
#define KillBDD(manager)       Cudd_Quit(manager)
#define GetVarCount(manager)   Cudd_ReadSize(manager)
#define DEBUGON                _debug = 1
#define DEBUGOFF               _debug = 0
#define RAPIDLOADON            _RapidLoad = 1
#define RAPIDLOADOFF           _RapidLoad = 0
#define SETMAXBUFSIZE(size)    _maxbufsize = size
#define BDDFILE_ERROR          -1
#define BDDFILE_OTHER          0
#define BDDFILE_SCRIPT         1
#define BDDFILE_NODEDUMP       2

extern int _RapidLoad;
extern int _debug;
extern int _maxbufsize;

typedef struct _bddfileheader {
  FILE *inputfile;
  int version;
  int varcnt;
  int bvarcnt;
  int varstart;
  int intercnt;
  int filetype;
} bddfileheader;

typedef struct
  {
int nVal,nBit,init;
double * probabilities;
int  * booleanVars;
} variable;

typedef struct _namedvars {
  int varcnt;
  int varstart;
  char **vars;
  int *loaded;
  double *dvalue;
  int *ivalue;
  void **dynvalue;
  variable * mvars;
  int * bVar2mVar;
} namedvars;


typedef struct _hisnode {
  DdNode *key;
  double dvalue;
  int ivalue;
  void *dynvalue;
} hisnode;

typedef struct _hisqueue {
  int cnt;
  hisnode *thenode;
} hisqueue;

typedef struct _nodeline {
  char *varname;
  char *truevar;
  char *falsevar;
  int nodenum;
  int truenode;
  int falsenode;
} nodeline;

/* Initialization */

DdManager* simpleBDDinit(int varcnt);

/* BDD Generation */

DdNode* D_BDDAnd(DdManager *manager, DdNode *bdd1, DdNode *bdd2);
DdNode* D_BDDNand(DdManager *manager, DdNode *bdd1, DdNode *bdd2);
DdNode* D_BDDOr(DdManager *manager, DdNode *bdd1, DdNode *bdd2);
DdNode* D_BDDNor(DdManager *manager, DdNode *bdd1, DdNode *bdd2);
DdNode* D_BDDXor(DdManager *manager, DdNode *bdd1, DdNode *bdd2);
DdNode* D_BDDXnor(DdManager *manager, DdNode *bdd1, DdNode *bdd2);

DdNode* FileGenerateBDD(DdManager *manager, namedvars varmap, bddfileheader fileheader);
DdNode* OnlineGenerateBDD(DdManager *manager, namedvars *varmap);
DdNode* LineParser(DdManager *manager, namedvars varmap, DdNode **inter, int maxinter, char *function, int iline);
DdNode* OnlineLineParser(DdManager *manager, namedvars *varmap, DdNode **inter, int maxinter, char *function, int iline);
DdNode* BDD_Operator(DdManager *manager, DdNode *bdd1, DdNode *bdd2, char Operator, int inegoper);
int getInterBDD(char *function);
char* getFileName(const char *function);
int GetParam(char *inputline, int iParam);
int LoadVariableData(namedvars varmap, char *filename);
int LoadMultiVariableData(DdManager * mgr,namedvars varmap, char *filename);

/* Named variables */

namedvars InitNamedVars(int varcnt, int varstart);
namedvars InitNamedMultiVars(int varcnt, int varstart, int bvarcnt);
void EnlargeNamedVars(namedvars *varmap, int newvarcnt);
int AddNamedVarAt(namedvars varmap, const char *varname, int index);
int AddNamedVar(namedvars varmap, const char *varname);
int AddNamedMultiVar(DdManager *mgr,namedvars varmap, const char *varnamei, int *value);
void SetNamedVarValuesAt(namedvars varmap, int index, double dvalue, int ivalue, void *dynvalue);
int SetNamedVarValues(namedvars varmap, const char *varname, double dvalue, int ivalue, void *dynvalue);
int GetNamedVarIndex(const namedvars varmap, const char *varname);
int RepairVarcnt(namedvars *varmap);
char* GetNodeVarName(DdManager *manager, namedvars varmap, DdNode *node);
char* GetNodeVarNameDisp(DdManager *manager, namedvars varmap, DdNode *node);
int all_loaded(namedvars varmap, int disp);

/* Traversal */

DdNode* HighNodeOf(DdManager *manager, DdNode *node);
DdNode* LowNodeOf(DdManager *manager, DdNode *node);

/* Traversal - History */

hisqueue* InitHistory(int varcnt);
void ReInitHistory(hisqueue *HisQueue, int varcnt);
void AddNode(hisqueue *HisQueue, int varstart, DdNode *node, double dvalue, int ivalue, void *dynvalue);
hisnode* GetNode(hisqueue *HisQueue, int varstart, DdNode *node);
int GetNodeIndex(hisqueue *HisQueue, int varstart, DdNode *node);
void onlinetraverse(DdManager *manager, namedvars varmap, hisqueue *HisQueue, DdNode *bdd);

/* Save-load */

bddfileheader ReadFileHeader(char *filename);
int CheckFileVersion(const char *version);

DdNode * LoadNodeDump(DdManager *manager, namedvars varmap, FILE *inputfile);
DdNode * LoadNodeRec(DdManager *manager, namedvars varmap, hisqueue *Nodes, FILE *inputfile, nodeline current);
DdNode * GetIfExists(DdManager *manager, namedvars varmap, hisqueue *Nodes, char *varname, int nodenum);

int SaveNodeDump(DdManager *manager, namedvars varmap, DdNode *bdd, char *filename);
void SaveExpand(DdManager *manager, namedvars varmap, hisqueue *Nodes, DdNode *Current, FILE *outputfile);
void ExpandNodes(hisqueue *Nodes, int index, int nodenum);

/* Export */

int simpleBDDtoDot(DdManager *manager, DdNode *bdd, char *filename);
int simpleNamedBDDtoDot(DdManager *manager, namedvars varmap, DdNode *bdd, char *filename);

DdNode * equality(DdManager *mgr,int varIndex,int value,namedvars varmap);
hisnode* GetNodei1(int *bVar2mVar,hisqueue *HisQueue, int varstart, DdNode *node);
void AddNode1(int *bVar2mVar, hisqueue *HisQueue, int varstart, DdNode *node, double dvalue, int ivalue, void *dynvalue);
hisnode* GetNode1(int *bVar2mVar,hisqueue *HisQueue, int varstart, DdNode *node);
