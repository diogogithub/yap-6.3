#ifndef PACKAGES_CLPBN_HORUS_VARELIM_H
#define PACKAGES_CLPBN_HORUS_VARELIM_H

#include "unordered_map"

#include "GroundSolver.h"
#include "FactorGraph.h"
#include "Horus.h"


class VarElim : public GroundSolver
{
  public:
    VarElim (const FactorGraph& fg) : GroundSolver (fg) { }

   ~VarElim (void) { }

    Params solveQuery (VarIds);

    void printSolverFlags (void) const;

  private:
    void createFactorList (void);

    void absorveEvidence (void);

    Params processFactorList (const VarIds&);

    void eliminate (VarId);

    void printActiveFactors (void);

    Factors   factorList_;
    unsigned  largestFactorSize_;
    unsigned  totalFactorSize_;
    std::unordered_map<VarId, std::vector<size_t>> varMap_;

    DISALLOW_COPY_AND_ASSIGN (VarElim);
};

#endif // PACKAGES_CLPBN_HORUS_VARELIM_H

