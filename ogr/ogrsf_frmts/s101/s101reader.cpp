//
// Created by tom on 11/21/25.
//

#include "s101reader.h"



S101Reader::S101Reader(const char *pszFilename)
    : pszModuleName(CPLStrdup(pszFilename)), pszDSNM(nullptr), poModule(nullptr),
      bFileIngested(false), bMissingWarningIssued(false),
      bAttrWarningIssued(false), nFDefnCount(0), papoFDefnList(nullptr),
      nNextDSIDIndex(0), poDSIDRecord(nullptr), poDSPMRecord(nullptr)
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

            const char *pszEDTN =
                poRecord->GetStringSubfield("DSID", 0, "EDTN", 0);
            if (pszEDTN)
                m_osEDTNUpdate = pszEDTN;

            const char *pszUPDN =
                poRecord->GetStringSubfield("DSID", 0, "UPDN", 0);
            if (pszUPDN)
                m_osUPDNUpdate = pszUPDN;

            const char *pszISDT =
                poRecord->GetStringSubfield("DSID", 0, "ISDT", 0);
            if (pszISDT)
                m_osISDTUpdate = pszISDT;

            // if (nOptionFlags & S57M_RETURN_DSID)
            // {
            //     if (poDSIDRecord != nullptr)
            //         delete poDSIDRecord;
            //
            //     poDSIDRecord = poRecord->Clone();
            // }
            poDSIDRecord = poRecord->Clone();
        }

        else if (EQUAL(pszName, "DSPM"))
        {
            // int bSuccess = FALSE;
            // nCOMF = std::max(
            //     1, poRecord->GetIntSubfield("DSPM", 0, "COMF", 0, &bSuccess));
            // if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
            //     break;
            // nSOMF = std::max(
            //     1, poRecord->GetIntSubfield("DSPM", 0, "SOMF", 0, &bSuccess));
            // if (!bSuccess && CPLGetLastErrorType() == CE_Failure)
            //     break;
            //
            // if (nOptionFlags & S57M_RETURN_DSID)
            // {
            //     if (poDSPMRecord != nullptr)
            //         delete poDSPMRecord;
            //
            //     poDSPMRecord = poRecord->Clone();
            // }
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

    if (poDSIDRecord == nullptr && poDSPMRecord == nullptr)
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
        poFeature->SetField(
            "DSID_DSNM", poDSIDRecord->GetStringSubfield("DSID", 0, "DSNM", 0));
        // if (!m_osEDTNUpdate.empty())
        //     poFeature->SetField("DSID_EDTN", m_osEDTNUpdate.c_str());
        // else
        //     poFeature->SetField("DSID_EDTN", poDSIDRecord->GetStringSubfield(
        //                                          "DSID", 0, "EDTN", 0));
        // if (!m_osUPDNUpdate.empty())
        //     poFeature->SetField("DSID_UPDN", m_osUPDNUpdate.c_str());
        // else
        //     poFeature->SetField("DSID_UPDN", poDSIDRecord->GetStringSubfield(
        //                                          "DSID", 0, "UPDN", 0));
        //
        // poFeature->SetField(
        //     "DSID_UADT", poDSIDRecord->GetStringSubfield("DSID", 0, "UADT", 0));
        // if (!m_osISDTUpdate.empty())
        //     poFeature->SetField("DSID_ISDT", m_osISDTUpdate.c_str());
        // else
        //     poFeature->SetField("DSID_ISDT", poDSIDRecord->GetStringSubfield(
        //                                          "DSID", 0, "ISDT", 0));
        poFeature->SetField(
            "DSID_STED", poDSIDRecord->GetFloatSubfield("DSID", 0, "STED", 0));
        poFeature->SetField("DSID_PRSP",
                            poDSIDRecord->GetIntSubfield("DSID", 0, "PRSP", 0));
        poFeature->SetField(
            "DSID_PSDN", poDSIDRecord->GetStringSubfield("DSID", 0, "PSDN", 0));
        poFeature->SetField(
            "DSID_PRED", poDSIDRecord->GetStringSubfield("DSID", 0, "PRED", 0));
        poFeature->SetField("DSID_PROF",
                            poDSIDRecord->GetIntSubfield("DSID", 0, "PROF", 0));
        poFeature->SetField("DSID_AGEN",
                            poDSIDRecord->GetIntSubfield("DSID", 0, "AGEN", 0));
        poFeature->SetField(
            "DSID_COMT", poDSIDRecord->GetStringSubfield("DSID", 0, "COMT", 0));

        /* --------------------------------------------------------------------
         */
        /*      Apply DSSI values. */
        /* --------------------------------------------------------------------
         */
        poFeature->SetField("DSSI_DSTR",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "DSTR", 0));
        poFeature->SetField("DSSI_AALL",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "AALL", 0));
        poFeature->SetField("DSSI_NALL",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NALL", 0));
        poFeature->SetField("DSSI_NOMR",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOMR", 0));
        poFeature->SetField("DSSI_NOCR",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOCR", 0));
        poFeature->SetField("DSSI_NOGR",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOGR", 0));
        poFeature->SetField("DSSI_NOLR",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOLR", 0));
        poFeature->SetField("DSSI_NOIN",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOIN", 0));
        poFeature->SetField("DSSI_NOCN",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOCN", 0));
        poFeature->SetField("DSSI_NOED",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOED", 0));
        poFeature->SetField("DSSI_NOFA",
                            poDSIDRecord->GetIntSubfield("DSSI", 0, "NOFA", 0));
    }

    /* -------------------------------------------------------------------- */
    /*      Apply DSPM record.                                              */
    /* -------------------------------------------------------------------- */
    if (poDSPMRecord != nullptr)
    {
        poFeature->SetField("DSPM_HDAT",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "HDAT", 0));
        poFeature->SetField("DSPM_VDAT",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "VDAT", 0));
        poFeature->SetField("DSPM_SDAT",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "SDAT", 0));
        poFeature->SetField("DSPM_CSCL",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "CSCL", 0));
        poFeature->SetField("DSPM_DUNI",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "DUNI", 0));
        poFeature->SetField("DSPM_HUNI",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "HUNI", 0));
        poFeature->SetField("DSPM_PUNI",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "PUNI", 0));
        poFeature->SetField("DSPM_COUN",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "COUN", 0));
        poFeature->SetField("DSPM_COMF",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "COMF", 0));
        poFeature->SetField("DSPM_SOMF",
                            poDSPMRecord->GetIntSubfield("DSPM", 0, "SOMF", 0));
        poFeature->SetField(
            "DSPM_COMT", poDSPMRecord->GetStringSubfield("DSPM", 0, "COMT", 0));
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