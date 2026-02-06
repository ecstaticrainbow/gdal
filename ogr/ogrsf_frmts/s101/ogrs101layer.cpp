//
// Created by tom on 11/21/25.
//

#include "ogr_s101.h"

#include "s101reader.h"
#include "miramon_common/mm_gdal_functions.h"

OGRS101Layer::OGRS101Layer(OGRS101DataSource *poDSIn, OGRFeatureDefn *poDefnIn)
    :nCurrentModule(-1)
{
    poDS = poDSIn;
    poFeatureDefn = poDefnIn;
    SetDescription(poFeatureDefn->GetName());
    if (poFeatureDefn->GetGeomFieldCount() > 0)
        poFeatureDefn->GetGeomFieldDefn(0)->SetSpatialRef(
            poDS->DSGetSpatialRef());
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
    OGRFeature *poFeature = nullptr;

    /* -------------------------------------------------------------------- */
    /*      Read features till we find one that satisfies our current       */
    /*      spatial criteria.                                               */
    /* -------------------------------------------------------------------- */
    while (true)
    {
        poFeature = GetNextUnfilteredFeature();
        if (poFeature == nullptr)
            break;

        if ((m_poFilterGeom == nullptr ||
             FilterGeometry(poFeature->GetGeometryRef())) &&
            (m_poAttrQuery == nullptr || m_poAttrQuery->Evaluate(poFeature)))
            break;

        delete poFeature;
    }

    return poFeature;
}

/************************************************************************/
/*                      GetNextUnfilteredFeature()                      */
/************************************************************************/

OGRFeature *OGRS101Layer::GetNextUnfilteredFeature()

{
    /* -------------------------------------------------------------------- */
    /*      Are we out of modules to request features from?                 */
    /* -------------------------------------------------------------------- */
    if (nCurrentModule >= poDS->GetModuleCount())
        return nullptr;

    /* -------------------------------------------------------------------- */
    /*      Set the current position on the current module and fetch a      */
    /*      feature.                                                        */
    /* -------------------------------------------------------------------- */
    S101Reader *poReader = poDS->GetModule(nCurrentModule);
    OGRFeature *poFeature = nullptr;

    if (poReader != nullptr)
    {
        //poReader->SetNextFEIndex(nNextFEIndex, nRCNM);
        poFeature = poReader->ReadNextFeature(poFeatureDefn);
        //nNextFEIndex = poReader->GetNextFEIndex(nRCNM);
    }

    /* -------------------------------------------------------------------- */
    /*      If we didn't get a feature we need to move onto the next file.  */
    /* -------------------------------------------------------------------- */
    if (poFeature == nullptr)
    {
        nCurrentModule++;
        poReader = poDS->GetModule(nCurrentModule);

        if (poReader != nullptr && poReader->GetModule() == nullptr)
        {
            if (!poReader->Open(FALSE))
                return nullptr;
        }

        return GetNextUnfilteredFeature();
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

/************************************************************************/
/*                             GetFeature()                             */
/************************************************************************/

OGRFeature *OGRS101Layer::GetFeature(GIntBig nFeatureId)

{
    S101Reader *poReader = poDS->GetModule(0);  // not multi-reader aware

    if (poReader != nullptr && nFeatureId <= INT_MAX)
    {
        OGRFeature *poFeature =
            poReader->ReadFeature(static_cast<int>(nFeatureId), poFeatureDefn);

        if (poFeature != nullptr && poFeature->GetGeometryRef() != nullptr)
            poFeature->GetGeometryRef()->assignSpatialReference(
                GetSpatialRef());
        return poFeature;
    }

    return nullptr;
}
