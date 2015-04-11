/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Kanwar Jit Singh
**********/

#include "ngspice/ngspice.h"
#include "ngspice/cktdefs.h"
#include "asrcdefs.h"
#include "ngspice/sperror.h"
#include "ngspice/suffix.h"


int
ASRCconvTest(GENmodel *inModel, CKTcircuit *ckt)
{
    ASRCmodel *model = (ASRCmodel *) inModel;
    ASRCinstance *here;
    int i, node_num, branch;
    double diff;
    double prev;
    double tol;
    double rhs;

    for (; model; model = model->ASRCnextModel) {
        for (here = model->ASRCinstances; here; here = here->ASRCnextInstance) {

            i = here->ASRCtree->numVars;
            if (asrc_nvals < i) {
                if (asrc_nvals) {
                    FREE(asrc_vals);
                    FREE(asrc_derivs);
                }
                asrc_nvals = i;
                asrc_vals = NEWN(double, i);
                asrc_derivs = NEWN(double, i);
            }

            for (i = 0; i < here->ASRCtree->numVars; i++)
                if (here->ASRCtree->varTypes[i] == IF_INSTANCE) {
                    branch = CKTfndBranch(ckt, here->ASRCtree->vars[i].uValue);
                    asrc_vals[i] = ckt->CKTrhsOld[branch];
                } else {
                    node_num = here->ASRCtree->vars[i].nValue->number;
                    asrc_vals[i] = ckt->CKTrhsOld[node_num];
                }

            if (here->ASRCtree->IFeval(here->ASRCtree, ckt->CKTgmin, &rhs,
                                       asrc_vals, asrc_derivs) != OK)
                return(E_BADPARM);

            prev = here->ASRCprev_value;
            diff = fabs(prev - rhs);

            if (here->ASRCtype == ASRC_VOLTAGE)
                tol = ckt->CKTreltol * MAX(fabs(rhs), fabs(prev))
                    + ckt->CKTvoltTol;
            else
                tol = ckt->CKTreltol * MAX(fabs(rhs), fabs(prev))
                    + ckt->CKTabstol;

            if (diff > tol) {
                ckt->CKTnoncon++;
                ckt->CKTtroubleElt = (GENinstance *) here;
                return(OK);
            }
        }
    }

    return(OK);
}
