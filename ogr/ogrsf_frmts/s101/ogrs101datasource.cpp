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

    oField.Set("DSSI_DCOX", OFTInteger64, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_DCOY", OFTInteger64, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_DCOZ", OFTInteger64, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_CMFX", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_CMFY", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_CMFZ", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOIR", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOPN", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOMN", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOCN", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOXN", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOSN", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    oField.Set("DSSI_NOFR", OFTInteger, 0, 0);
    poFDefn->AddFieldDefn(&oField);

    return poFDefn;
}

