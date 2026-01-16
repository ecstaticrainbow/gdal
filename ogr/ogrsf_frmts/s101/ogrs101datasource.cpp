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
    /*      Add the header layers if they are called for.                   */
    /* -------------------------------------------------------------------- */
    // if (GetOption(S57O_RETURN_DSID) == nullptr ||
    //     CPLTestBool(GetOption(S57O_RETURN_DSID)))
    {
        OGRFeatureDefn *poDefn = S101GenerateDSIDFeatureDefn();
        AddLayer(new OGRS101Layer(this, poDefn));
    }

    /* -------------------------------------------------------------------- */
    /*      Initialize a layer for each type of geometry.  Eventually       */
    /*      we will do this by object class.                                */
    /* -------------------------------------------------------------------- */
    // if (OGRS57Driver::GetS57Registrar() == nullptr)
    {
        OGRFeatureDefn *poDefn =
            S101GenerateGeomFeatureDefn(wkbPoint, poModule->GetOptionFlags());
        AddLayer(new OGRS101Layer(this, poDefn));

        poDefn = S101GenerateGeomFeatureDefn(wkbLineString,
                                            poModule->GetOptionFlags());
        AddLayer(new OGRS101Layer(this, poDefn));

        poDefn =
            S101GenerateGeomFeatureDefn(wkbPolygon, poModule->GetOptionFlags());
        AddLayer(new OGRS101Layer(this, poDefn));

        poDefn =
            S101GenerateGeomFeatureDefn(wkbNone, poModule->GetOptionFlags());
        AddLayer(new OGRS101Layer(this, poDefn));


        poDefn = S101GenerateObjectClassDefn(0, poModule->GetOptionFlags());
        AddLayer(new OGRS101Layer(this, poDefn));
    }





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

S101Reader *OGRS101DataSource::GetModule(int i) const
{
    if (i < 0 || i >= nModules)
        return nullptr;

    return papoModules[i];
}
