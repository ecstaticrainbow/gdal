//
// Created by tom on 11/21/25.
//

#include "s101reader.h"



S101Reader::S101Reader(const char *pszFilename)
    : pszModuleName(CPLStrdup(pszFilename)), pszDSNM(nullptr),
      poModule(nullptr), nCMFX(10000000), nCMFY(10000000), nCMFZ(10), bFileIngested(false),
      bMissingWarningIssued(false), bAttrWarningIssued(false), nFDefnCount(0),
      papoFDefnList(nullptr), nNextDSIDIndex(0), poDSIDRecord(nullptr),
      poDSSIRecord(nullptr)
{
}

S101Reader::~S101Reader()
{
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

int S101Reader::Open(int bTestOpen)

{
    if (poModule != nullptr)
    {
        //Rewind();
        return TRUE;
    }

    poModule = new DDFModule();
    if (!poModule->Open(pszModuleName))
    {
        // notdef: test bTestOpen.
        delete poModule;
        poModule = nullptr;
        return FALSE;
    }

    // note that the following won't work for catalogs.
    if (poModule->FindFieldDefn("DSID") == nullptr)
    {
        if (!bTestOpen)
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "%s is an ISO8211 file, but not an S-101 data file.\n",
                     pszModuleName);
        }
        delete poModule;
        poModule = nullptr;
        return FALSE;
    }

    // Make sure the FSPT field is marked as repeating.
    DDFFieldDefn *poFSPT = poModule->FindFieldDefn("FSPT");
    if (poFSPT != nullptr && !poFSPT->IsRepeating())
    {
        CPLDebug("S101", "Forcing FSPT field to be repeating.");
        poFSPT->SetRepeatingFlag(TRUE);
    }

    // nNextFEIndex = 0;
    // nNextVIIndex = 0;
    // nNextVCIndex = 0;
    // nNextVEIndex = 0;
    // nNextVFIndex = 0;
    nNextDSIDIndex = 0;

    return TRUE;
}

bool S101Reader::Ingest()
{
    if (poModule == nullptr || bFileIngested)
        return true;

    /* -------------------------------------------------------------------- */
    /*      Read all the records in the module, and place them in           */
    /*      appropriate indexes.                                            */
    /* -------------------------------------------------------------------- */
    CPLErrorReset();
    DDFRecord *poRecord = nullptr;
    while ((poRecord = poModule->ReadRecord()) != nullptr)
    {
        DDFField *poKeyField = poRecord->GetField(0);
        if (poKeyField == nullptr)
            return false;
        DDFFieldDefn *poKeyFieldDefn = poKeyField->GetFieldDefn();
        if (poKeyFieldDefn == nullptr)
            continue;
        const char *pszName = poKeyFieldDefn->GetName();
        if (pszName == nullptr)
            continue;

        if (EQUAL(pszName, "VRID"))
        {
            // int bSuccess = FALSE;
            // const int nRCNM =
            //     poRecord->GetIntSubfield("VRID", 0, "RCNM", 0, &bSuccess);
            // if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
            //     break;
            // const int nRCID =
            //     poRecord->GetIntSubfield("VRID", 0, "RCID", 0, &bSuccess);
            // if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
            //     break;
            //
            // switch (nRCNM)
            // {
            //     case RCNM_VI:
            //         oVI_Index.AddRecord(nRCID, poRecord->Clone());
            //         break;
            //
            //     case RCNM_VC:
            //         oVC_Index.AddRecord(nRCID, poRecord->Clone());
            //         break;
            //
            //     case RCNM_VE:
            //         oVE_Index.AddRecord(nRCID, poRecord->Clone());
            //         break;
            //
            //     case RCNM_VF:
            //         oVF_Index.AddRecord(nRCID, poRecord->Clone());
            //         break;
            //
            //     default:
            //         CPLError(CE_Failure, CPLE_AppDefined,
            //                  "Unhandled value for RCNM ; %d", nRCNM);
            //         break;
            // }
        }

        else if (EQUAL(pszName, "FRID"))
        {
            // int bSuccess = FALSE;
            // int nRCID =
            //     poRecord->GetIntSubfield("FRID", 0, "RCID", 0, &bSuccess);
            // if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
            //     break;
            //
            // oFE_Index.AddRecord(nRCID, poRecord->Clone());
        }

        else if (EQUAL(pszName, "DSID"))
        {
            int bSuccess = FALSE;
            CPLFree(pszDSNM);
            pszDSNM = CPLStrdup(
                poRecord->GetStringSubfield("DSID", 0, "DSNM", 0, &bSuccess));
            if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
                break;

            nCMFX = std::max(
                1, poRecord->GetIntSubfield("DSSI", 0, "CMFX", 0, &bSuccess));
            if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
                break;
            nCMFY = std::max(
                1, poRecord->GetIntSubfield("DSSI", 0, "CMFY", 0, &bSuccess));
            if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
                break;

            nCMFZ = std::max(
               1, poRecord->GetIntSubfield("DSSI", 0, "CMFZ", 0, &bSuccess));
            if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
                break;

            // if (nOptionFlags & S57M_RETURN_DSID)
            // {
            //     if (poDSIDRecord != nullptr)
            //         delete poDSIDRecord;
            //
            //     poDSIDRecord = poRecord->Clone();
            // }
            poDSIDRecord = poRecord->Clone();
        }

        else
        {
            CPLDebug("S57", "Skipping %s record in S57Reader::Ingest().",
                     pszName);
        }
    }

    if (CPLGetLastErrorType() == CE_Failure)
        return false;

    bFileIngested = true;

    /* -------------------------------------------------------------------- */
    /*      If update support is enabled, read and apply them.              */
    /* -------------------------------------------------------------------- */
    // if (nOptionFlags & S57M_UPDATES)
    //     return FindAndApplyUpdates();

    return true;
}

/************************************************************************/
/*                              ReadDSID()                              */
/************************************************************************/

OGRFeature *S101Reader::ReadDSID()

{
    if (!bFileIngested && !Ingest())
        return nullptr;

    if (poDSIDRecord == nullptr && poDSSIRecord == nullptr)
        return nullptr;

    /* -------------------------------------------------------------------- */
    /*      Find the feature definition to use.                             */
    /* -------------------------------------------------------------------- */
    OGRFeatureDefn *poFDefn = nullptr;

    for (int i = 0; i < nFDefnCount; i++)
    {
        if (EQUAL(papoFDefnList[i]->GetName(), "DSID"))
        {
            poFDefn = papoFDefnList[i];
            break;
        }
    }

    if (poFDefn == nullptr)
    {
        // CPLAssert( false );
        return nullptr;
    }

    /* -------------------------------------------------------------------- */
    /*      Create feature.                                                 */
    /* -------------------------------------------------------------------- */
    OGRFeature *poFeature = new OGRFeature(poFDefn);

    /* -------------------------------------------------------------------- */
    /*      Apply DSID values.                                              */
    /* -------------------------------------------------------------------- */
    if (poDSIDRecord != nullptr)
    {
        poFeature->SetField("DSID_ENSP",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "ENSP", 0));
        poFeature->SetField("DSID_ENED",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "ENED", 0));
        poFeature->SetField("DSID_PRSP",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "PRSP", 0));
        poFeature->SetField("DSID_PRED",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "PRED", 0));
        poFeature->SetField("DSID_PROF",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "PROF", 0));
        poFeature->SetField("DSID_DSNM",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "DSMN", 0));
        poFeature->SetField("DSID_DSTL",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "DSTL", 0));
        poFeature->SetField("DSID_DSRD",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "DSRD", 0));
        poFeature->SetField("DSID_DSLG",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "DLSG", 0));
        poFeature->SetField("DSID_DSAB",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "DSAB", 0));
        poFeature->SetField("DSID_DSED",
                            poDSIDRecord->GetStringSubfield("DSID", 0, "DSED", 0));

        // TODO: DSTC

        /* --------------------------------------------------------------------
         */
        /*      Apply DSSI values. */
        /* --------------------------------------------------------------------
         */
        poFeature->SetField("DSSI_DCOX",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "DCOX", 0));
        poFeature->SetField("DSSI_DCOY",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "DCOY", 0));
        poFeature->SetField("DSSI_DCOZ",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "DCOZ", 0));
        poFeature->SetField("DSSI_CMFX",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "CMFX", 0));
        poFeature->SetField("DSSI_CMFY",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "CMFY", 0));
        poFeature->SetField("DSSI_CMFZ",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "CMFZ", 0));
        poFeature->SetField("DSSI_NOIR",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOIR", 0));
        poFeature->SetField("DSSI_NOPN",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOPN", 0));
        poFeature->SetField("DSSI_NOMN",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOMN", 0));
        poFeature->SetField("DSSI_NOCN",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOCN", 0));
        poFeature->SetField("DSSI_NOXN",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOXN", 0));
        poFeature->SetField("DSSI_NOSN",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOSN", 0));
        poFeature->SetField("DSSI_NOFR",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOFR", 0));
    }

    poFeature->SetFID(nNextDSIDIndex++);

    return poFeature;
}

void S101Reader::AddFeatureDefn(OGRFeatureDefn *poFDefn)
{
    nFDefnCount++;
    papoFDefnList = static_cast<OGRFeatureDefn **>(
        CPLRealloc(papoFDefnList, sizeof(OGRFeatureDefn *) * nFDefnCount));

    papoFDefnList[nFDefnCount - 1] = poFDefn;

    // if (poRegistrar != nullptr)
    // {
    //     if (poClassContentExplorer->SelectClass(poFDefn->GetName()))
    //     {
    //         const int nOBJL = poClassContentExplorer->GetOBJL();
    //         if (nOBJL >= 0)
    //         {
    //             if (nOBJL >= (int)apoFDefnByOBJL.size())
    //                 apoFDefnByOBJL.resize(nOBJL + 1);
    //             apoFDefnByOBJL[nOBJL] = poFDefn;
    //         }
    //     }
    // }
}