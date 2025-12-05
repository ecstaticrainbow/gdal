//
// Created by tom on 11/21/25.
//

#include "ogr_s101.h"


/************************************************************************/
/*                         Hard-coded S-101 catalogue                  */
/************************************************************************/

static const struct
{
    const char* code;
    const char* layerName;
} gaS101Catalogue[] =
{
    {"BOYSPP", "BuoySpecialPurpose"},
    {"BOYSAW", "SafeWaterBuoy"},
    {"SLCONS", "SlopeConstruction"},
    {"LNDARE", "LandArea"},
    // Add as many as you need…
};

static const int nS101FeatureTypes =
    sizeof(gaS101Catalogue) / sizeof(gaS101Catalogue[0]);

int OGRS101DataSource::Open(const char *pszName)
{
    // TODO: think added for testing purposes
    pszFilename = pszName;
    BuildLayers();

    S101Reader *poModule = new S101Reader(pszFilename);

    /* -------------------------------------------------------------------- */
    /*      Try opening.                                                    */
    /*                                                                      */
    /*      Eventually this should check for catalogs, and if found         */
    /*      instantiate a whole series of modules.                          */
    /* -------------------------------------------------------------------- */
    if (!poModule->Open(TRUE))
    {
        delete poModule;

        return FALSE;
    }

    nModules = 1;
    papoModules = static_cast<S101Reader **>(CPLMalloc(sizeof(void *)));
    papoModules[0] = poModule;


    /* -------------------------------------------------------------------- */
    /*      Attach the layer definitions to each of the readers.            */
    /* -------------------------------------------------------------------- */
    for (int iModule = 0; iModule < nModules; iModule++)
    {
        for (int iLayer = 0; iLayer < nLayers; iLayer++)
        {
            OGRLayer *poLayer = papoLayers[iLayer];
            papoModules[iModule]->AddFeatureDefn(poLayer->GetLayerDefn());
        }
    }



    return true;
}

void OGRS101DataSource::AddLayer(OGRS101Layer *poNewLayer)
{
    papoLayers = static_cast<OGRS101Layer **>(
        CPLRealloc(papoLayers, sizeof(void *) * ++nLayers));

    papoLayers[nLayers - 1] = poNewLayer;
}

S101Reader *OGRS101DataSource::GetModule(int i)
{
    if (i < 0 || i >= nModules)
        return nullptr;

    return papoModules[i];
}

void OGRS101DataSource::BuildLayers()
{
    {
        // TODO: Test stuff
        /* -------------------------------------------------------------------- */
        /*      Add the header layers if they are called for.                   */
        /* -------------------------------------------------------------------- */
        OGRFeatureDefn *poDefn = S101GenerateDSIDFeatureDefn();
        AddLayer(new OGRS101Layer(this, poDefn, pszFilename));

        // for (int i = 0; i < nS101FeatureTypes; i++)
        // {
        //     auto* poLayer = new OGRS101Layer(gaS101Catalogue[i].layerName);
        //     AddLayer(poLayer);
        // }
    }
}

/************************************************************************/
/*                     S101GenerateDSIDeatureDefn()                     */
/************************************************************************/

OGRFeatureDefn* OGRS101DataSource::S101GenerateDSIDFeatureDefn()

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

