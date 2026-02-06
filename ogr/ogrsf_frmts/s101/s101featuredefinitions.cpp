//
// Created by tom on 11/21/25.
//

#include "ogr_s101.h"

/************************************************************************/
/*                     S101GenerateDSIDeatureDefn()                     */
/************************************************************************/

OGRFeatureDefn* S101GenerateDSIDFeatureDefn()

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

    oField.Set("DSID_ENED", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_PRSP", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_PRED", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_PROF", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_DSNM", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_DSTL", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_DSRD", OFTString, 8, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_DSLG", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_DSAB", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSID_DSED", OFTString, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    // TODO: DSTC
    // oField.Set("DSID_DSTC", OFTBinary, 32, 0);
    // poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      DSSI fields.                                                    */
    /* -------------------------------------------------------------------- */

    // oField.Set("DSSI_DCOX", OFTInteger64, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_DCOY", OFTInteger64, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_DCOZ", OFTInteger64, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_CMFX", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_CMFY", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_CMFZ", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_NOIR", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_NOPN", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_NOMN", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_NOCN", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_NOXN", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_NOSN", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);
    //
    // oField.Set("DSSI_NOFR", OFTInteger, 0, 0);
    // poFDefn->AddFieldDefn(&oField);

    return poFDefn;
}

/************************************************************************/
/*                     S101GenerateGeomFeatureDefn()                     */
/************************************************************************/

OGRFeatureDefn *S101GenerateGeomFeatureDefn(OGRwkbGeometryType eGType,
                                           int nOptionFlags)

{
    OGRFeatureDefn *poFDefn = nullptr;

    if (eGType == wkbPoint)
    {
        poFDefn = new OGRFeatureDefn("Point");
        poFDefn->SetGeomType(eGType);
    }
    else if (eGType == wkbLineString)
    {
        poFDefn = new OGRFeatureDefn("Line");
        poFDefn->SetGeomType(eGType);
    }
    else if (eGType == wkbPolygon)
    {
        poFDefn = new OGRFeatureDefn("Area");
        poFDefn->SetGeomType(eGType);
    }
    else if (eGType == wkbNone)
    {
        poFDefn = new OGRFeatureDefn("Meta");
        poFDefn->SetGeomType(eGType);
    }
    else if (eGType == wkbUnknown)
    {
        poFDefn = new OGRFeatureDefn("Generic");
        poFDefn->SetGeomType(eGType);
    }
    else
        return nullptr;

    poFDefn->Reference();
    S101GenerateStandardAttributes(poFDefn, nOptionFlags);

    return poFDefn;
}

/************************************************************************/
/*                     S57GenerateObjectClassDefn()                     */
/************************************************************************/

OGRFeatureDefn *
S101GenerateObjectClassDefn(S101ClassRegistrar *poCR,
                           S101ClassContentExplorer *poClassContentExplorer,
                           const char* featureCode, int nOptionFlags)

{
    if (!poClassContentExplorer->SelectClass(featureCode))
        return nullptr;

    /* -------------------------------------------------------------------- */
    /*      Create the feature definition based on the object class         */
    /*      acronym.                                                        */
    /* -------------------------------------------------------------------- */
    OGRFeatureDefn *poFDefn =
        new OGRFeatureDefn(poClassContentExplorer->GetAcronym());
    poFDefn->Reference();

    /* -------------------------------------------------------------------- */
    /*      Try and establish the geometry type.  If more than one          */
    /*      geometry type is allowed we just fall back to wkbUnknown.       */
    /* -------------------------------------------------------------------- */
    std::vector<std::string> papszGeomPrim = poClassContentExplorer->GetPrimitives();
    if (papszGeomPrim.empty())
    {
        poFDefn->SetGeomType(wkbNone);
    }
    else if (papszGeomPrim.size() > 1)
    {
        // leave as unknown geometry type.
    }
    else if (papszGeomPrim[0] == "point")
    {
        poFDefn->SetGeomType(wkbPoint);
    }
    else if (papszGeomPrim[0] == "pointSet")
    {
        poFDefn->SetGeomType(wkbMultiPoint);
    }
    else if (papszGeomPrim[0] == "surface")
    {
        poFDefn->SetGeomType(wkbPolygon);
    }
    else if (papszGeomPrim[0] == "curve")
    {
        // unfortunately this could be a multilinestring
        poFDefn->SetGeomType(wkbUnknown);
    }

    /* -------------------------------------------------------------------- */
    /*      Add the standard attributes.                                    */
    /* -------------------------------------------------------------------- */
    //S101GenerateStandardAttributes(poFDefn, nOptionFlags);

    /* -------------------------------------------------------------------- */
    /*      Add the attributes specific to this object class.               */
    /* -------------------------------------------------------------------- */

    // char **papszAttrList = poClassContentExplorer->GetAttributeList();
    //
    // for (int iAttr = 0;
    //      papszAttrList != nullptr && papszAttrList[iAttr] != nullptr; iAttr++)
    // {
    //     const int iAttrIndex = poCR->FindAttrByAcronym(papszAttrList[iAttr]);
    //
    //     if (iAttrIndex == -1)
    //     {
    //         CPLDebug("S57", "Can't find attribute %s from class %s:%s.",
    //                  papszAttrList[iAttr], poClassContentExplorer->GetAcronym(),
    //                  poClassContentExplorer->GetDescription());
    //         continue;
    //     }
    //
    //     OGRFieldDefn oField(papszAttrList[iAttr], OFTInteger);
    //
    //     switch (poCR->GetAttrType(iAttrIndex))
    //     {
    //         case SAT_ENUM:
    //         case SAT_INT:
    //             oField.SetType(OFTInteger);
    //             break;
    //
    //         case SAT_FLOAT:
    //             oField.SetType(OFTReal);
    //             break;
    //
    //         case SAT_CODE_STRING:
    //         case SAT_FREE_TEXT:
    //             oField.SetType(OFTString);
    //             break;
    //
    //         case SAT_LIST:
    //             if ((nOptionFlags & S57M_LIST_AS_STRING))
    //             {
    //                 // Legacy behavior
    //                 oField.SetType(OFTString);
    //             }
    //             else
    //             {
    //                 oField.SetType(OFTStringList);
    //             }
    //             break;
    //     }
    //
    //     poFDefn->AddFieldDefn(&oField);
    // }
    //
    // /* -------------------------------------------------------------------- */
    // /*      Do we need to add DEPTH attributes to soundings?                */
    // /* -------------------------------------------------------------------- */
    // const char *pszClassAcronym = poClassContentExplorer->GetAcronym();
    // if (pszClassAcronym != nullptr && EQUAL(pszClassAcronym, "SOUNDG") &&
    //     (nOptionFlags & S57M_ADD_SOUNDG_DEPTH))
    // {
    //     OGRFieldDefn oField("DEPTH", OFTReal);
    //     poFDefn->AddFieldDefn(&oField);
    // }

    return poFDefn;
}

/************************************************************************/
/*                   S57GenerateStandardAttributes()                    */
/*                                                                      */
/*      Attach standard feature attributes to a feature definition.     */
/************************************************************************/

void S101GenerateStandardAttributes(OGRFeatureDefn *poFDefn, int nOptionFlags)

{
    OGRFieldDefn oField("", OFTInteger);

    /* -------------------------------------------------------------------- */
    /*      RCID                                                            */
    /* -------------------------------------------------------------------- */
    oField.Set("RCID", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      PRIM                                                            */
    /* -------------------------------------------------------------------- */
    oField.Set("PRIM", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      GRUP                                                            */
    /* -------------------------------------------------------------------- */
    oField.Set("GRUP", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      OBJL                                                            */
    /* -------------------------------------------------------------------- */
    oField.Set("OBJL", OFTInteger, 5, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      RVER                                                            */
    /* -------------------------------------------------------------------- */
    oField.Set("RVER", OFTInteger, 3, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      AGEN                                                            */
    /* -------------------------------------------------------------------- */
    oField.Set("AGEN", OFTInteger, 5, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      FIDN                                                            */
    /* -------------------------------------------------------------------- */
    oField.Set("FIDN", OFTInteger, 10, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      FIDS                                                            */
    /* -------------------------------------------------------------------- */
    oField.Set("FIDS", OFTInteger, 5, 0);
    poFDefn->AddFieldDefn(&oField);

    /* -------------------------------------------------------------------- */
    /*      LNAM - only generated when LNAM strings are being used.         */
    /* -------------------------------------------------------------------- */
    // if (nOptionFlags & S57M_LNAM_REFS)
    // {
    //     oField.Set("LNAM", OFTString, 16, 0);
    //     poFDefn->AddFieldDefn(&oField);
    //
    //     oField.Set("LNAM_REFS", OFTStringList, 16, 0);
    //     poFDefn->AddFieldDefn(&oField);
    //
    //     oField.Set("FFPT_RIND", OFTIntegerList, 1, 0);
    //     poFDefn->AddFieldDefn(&oField);
    //
    //     // We should likely include FFPT_COMT here.
    // }

    /* -------------------------------------------------------------------- */
    /*      Values from FSPT field.                                         */
    /* -------------------------------------------------------------------- */
    // if (nOptionFlags & S57M_RETURN_LINKAGES)
    // {
    //     oField.Set("NAME_RCNM", OFTIntegerList, 3, 0);
    //     poFDefn->AddFieldDefn(&oField);
    //
    //     oField.Set("NAME_RCID", OFTIntegerList, 10, 0);
    //     poFDefn->AddFieldDefn(&oField);
    //
    //     oField.Set("ORNT", OFTIntegerList, 1, 0);
    //     poFDefn->AddFieldDefn(&oField);
    //
    //     oField.Set("USAG", OFTIntegerList, 1, 0);
    //     poFDefn->AddFieldDefn(&oField);
    //
    //     oField.Set("MASK", OFTIntegerList, 3, 0);
    //     poFDefn->AddFieldDefn(&oField);
    // }
}