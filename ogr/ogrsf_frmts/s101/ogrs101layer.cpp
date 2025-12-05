//
// Created by tom on 11/21/25.
//

#include "ogr_s101.h"

#include "s101reader.h"
#include "miramon_common/mm_gdal_functions.h"

OGRS101Layer::OGRS101Layer(OGRS101DataSource *poDSIn, OGRFeatureDefn *poDefnIn,
                           const char *pszNameIn)
{
    poDS = poDSIn;
    poFeatureDefn = poDefnIn;
    SetDescription(poFeatureDefn->GetName());
    fileName = pszNameIn;
}

OGRS101Layer::~OGRS101Layer()
{
    if (poFeatureDefn)
        poFeatureDefn->Release();
}

/************************************************************************/
/*                            ResetReading()                            */
/************************************************************************/

void OGRS101Layer::ResetReading()
{
    iNext = 0;
}

int OGRS101Layer::TestCapability(const char *) const
{
    return TRUE;
}

OGRFeature *OGRS101Layer::GetNextFeature()
{
    // TODO: Return parsed features
    if (iNext > 0)
        return nullptr;

    OGRFeature *poFeature = nullptr;

    S101Reader *poReader = poDS->GetModule(0);
    if (poReader != nullptr)
    {
        poFeature = poReader->ReadDSID();
    }

    /* -------------------------------------------------------------------- */
    /*      If we didn't get a feature we need to move onto the next file.  */
    /* -------------------------------------------------------------------- */
    if (poFeature == nullptr)
    {

    }
    else
    {
        m_nFeaturesRead++;
        if (poFeature->GetGeometryRef() != nullptr)
            poFeature->GetGeometryRef()->assignSpatialReference(
                GetSpatialRef());
    }

    return poFeature;
}