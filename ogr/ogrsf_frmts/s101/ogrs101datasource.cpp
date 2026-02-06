//
// Created by tom on 11/21/25.
//

#include "ogr_s101.h"


/************************************************************************/
/*                          OGRS57DataSource()                          */
/************************************************************************/

OGRS101DataSource::OGRS101DataSource(char **papszOpenOptionsIn)
    : nLayers(0), papoLayers(nullptr), poSpatialRef(new OGRSpatialReference()), nModules(0), papoModules(nullptr),
      poClassContentExplorer(nullptr)
{
    poSpatialRef->SetWellKnownGeogCS("WGS84");
    poSpatialRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

    /* -------------------------------------------------------------------- */
    /*      Allow initialization of options from the environment.           */
    /* -------------------------------------------------------------------- */
    const char *pszOptString = CPLGetConfigOption("OGR_S57_OPTIONS", nullptr);

    if (pszOptString != nullptr)
    {
        // papszOptions =
        //     CSLTokenizeStringComplex(pszOptString, ",", FALSE, FALSE);
        //
        // if (papszOptions && *papszOptions)
        // {
        //     CPLDebug("S57", "The following S57 options are being set:");
        //     char **papszCurOption = papszOptions;
        //     while (*papszCurOption)
        //         CPLDebug("S57", "    %s", *papszCurOption++);
        // }
    }

    /* -------------------------------------------------------------------- */
    /*      And from open options.                                          */
    /* -------------------------------------------------------------------- */
    for (char **papszIter = papszOpenOptionsIn; papszIter && *papszIter;
         ++papszIter)
    {
        char *pszKey = nullptr;
        const char *pszValue = CPLParseNameValue(*papszIter, &pszKey);
        if (pszKey && pszValue)
        {
            //papszOptions = CSLSetNameValue(papszOptions, pszKey, pszValue);
        }
        CPLFree(pszKey);
    }
}

/************************************************************************/
/*                         ~OGRS57DataSource()                          */
/************************************************************************/

OGRS101DataSource::~OGRS101DataSource()

{
    for (int i = 0; i < nLayers; i++)
        delete papoLayers[i];

    CPLFree(papoLayers);

    for (int i = 0; i < nModules; i++)
        delete papoModules[i];
    CPLFree(papoModules);

    // CSLDestroy(papszOptions);
    //
    poSpatialRef->Release();
    //
    // if (poWriter != nullptr)
    // {
    //     poWriter->Close();
    //     delete poWriter;
    // }
    delete poClassContentExplorer;
}

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

    bool bSuccess = true;

    nModules = 1;
    papoModules = static_cast<S101Reader **>(CPLMalloc(sizeof(void *)));
    papoModules[0] = poModule;

    /* -------------------------------------------------------------------- */
    /*      Add the header layers if they are called for.                   */
    /* -------------------------------------------------------------------- */
    // if (GetOption(S57O_RETURN_DSID) == nullptr ||
    //     CPLTestBool(GetOption(S57O_RETURN_DSID)))
    {
        // TODO: this layer breaks everything
        // OGRFeatureDefn *poDefn = S101GenerateDSIDFeatureDefn();
        // AddLayer(new OGRS101Layer(this, poDefn));
    }

    /* -------------------------------------------------------------------- */
    /*      Initialize a layer for each type of geometry.  Eventually       */
    /*      we will do this by object class.                                */
    /* -------------------------------------------------------------------- */
    if (OGRS101Driver::GetS101Registrar() == nullptr)
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
    }

    /* -------------------------------------------------------------------- */
    /*      Initialize a feature definition for each class that actually    */
    /*      occurs in the dataset.                                          */
    /* -------------------------------------------------------------------- */
    else
    {
        poClassContentExplorer =
            new S101ClassContentExplorer(OGRS101Driver::GetS101Registrar());

        for (int iModule = 0; iModule < nModules; iModule++)
            papoModules[iModule]->SetClassBased(OGRS101Driver::GetS101Registrar(),
                                                poClassContentExplorer);

        std::vector<int> anClassCount;
        std::unordered_map<int, S101FeatureTypeRow> m_oFTNCToType;

        for (int iModule = 0; iModule < nModules; iModule++)
        {
            bSuccess &= CPL_TO_BOOL(
                papoModules[iModule]->CollectClassList(anClassCount));

            bSuccess &= CPL_TO_BOOL(
                papoModules[iModule]->BuildFeatureTypeMap(m_oFTNCToType));
        }



        bool bGeneric = false;

        for (unsigned int iClass = 0; iClass < anClassCount.size(); iClass++)
        {
            if (anClassCount[iClass] > 0)
            {
                auto blah = m_oFTNCToType[iClass];

                OGRFeatureDefn *poDefn = S101GenerateObjectClassDefn(
                    OGRS101Driver::GetS101Registrar(), poClassContentExplorer,
                    blah.osName, poModule->GetOptionFlags());

                if (poDefn != nullptr)
                    AddLayer(
                        new OGRS101Layer(this, poDefn, anClassCount[iClass]));
                else
                {
                    bGeneric = true;
                    CPLDebug("S57", "Unable to find definition for OBJL=%d\n",
                             iClass);
                }
            }
        }

        // if (bGeneric)
        // {
        //     OGRFeatureDefn *poDefn = S57GenerateGeomFeatureDefn(
        //         wkbUnknown, poModule->GetOptionFlags());
        //     AddLayer(new OGRS101Layer(this, poDefn));
        // }
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

/************************************************************************/
/*                              GetLayer()                              */
/************************************************************************/

const OGRLayer *OGRS101DataSource::GetLayer(int iLayer) const

{
    if (iLayer < 0 || iLayer >= nLayers)
        return nullptr;

    return papoLayers[iLayer];
}

/************************************************************************/
/*                              AddLayer()                              */
/************************************************************************/

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
