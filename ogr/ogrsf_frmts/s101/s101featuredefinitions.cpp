//
// Created by tom on 11/21/25.
//

#include "ogr_s101.h"

/************************************************************************/
/*                     S57GenerateGeomFeatureDefn()                     */
/************************************************************************/

OGRFeatureDefn *S101GenerateDSIDFeatureDefn()

{
    OGRFeatureDefn *poFDefn = new OGRFeatureDefn("DSID");

    poFDefn->SetGeomType(wkbNone);
    poFDefn->Reference();

    /* -------------------------------------------------------------------- */
    /*      DSID fields.                                                    */
    /* -------------------------------------------------------------------- */
    OGRFieldDefn oField("", OFTInteger);

    oField.Set("DSID_ENSP", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_INTU", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_DSNM", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_EDTN", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_UPDN", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_UADT", OFTString, 8, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_ISDT", OFTString, 8, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_STED", OFTReal, 11, 6);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_PRSP", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_PSDN", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_PRED", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_PROF", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_AGEN", OFTInteger, 5, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_COMT", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      DSSI fields.                                                    */
    /* -------------------------------------------------------------------- */

    oField.Set("DSSI_DSTR", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_AALL", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NALL", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOMR", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOCR", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOGR", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOLR", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOIN", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOCN", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOED", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOFA", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      DSPM fields.                                                    */
    /* -------------------------------------------------------------------- */

    oField.Set("DSPM_HDAT", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_VDAT", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_SDAT", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_CSCL", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_DUNI", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_HUNI", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_PUNI", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_COUN", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_COMF", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_SOMF", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSPM_COMT", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    return poFDefn;
}