//
// Created by tom on 11/21/25.
//

#ifndef OGR_S101_H
#define OGR_S101_H

#include "ogrsf_frmts.h"
#include "s101reader.h"

class OGRS101DataSource;

/************************************************************************/
/*                           OGRS101Layer                               */
/************************************************************************/

class OGRS101Layer : public OGRLayer
{
    OGRS101DataSource *poDS;

    OGRFeatureDefn* poFeatureDefn = nullptr;
    OGRSpatialReference* poSRS = nullptr;
    size_t iNext = 0;

    //TODO
    const char *fileName;

public:
    OGRS101Layer(OGRS101DataSource *poDSIn, OGRFeatureDefn *poDefnIn, const char *pszName);

    ~OGRS101Layer() override;

    OGRFeatureDefn* GetLayerDefn() const override { return poFeatureDefn; }

    void ResetReading() override;
    int TestCapability(const char *) const override;

    OGRFeature* GetNextFeature() override;
};

class OGRS101DataSource final : public GDALDataset
{

public:

    int nModules;
    S101Reader **papoModules;

    OGRS101Layer **papoLayers;
    int nLayers;
    // TODO
    const char *pszFilename;
    OGRS101DataSource() = default;
    ~OGRS101DataSource() override
    {
        for (int i = 0; i < nLayers; i++)
            delete papoLayers[i];
    }

    int Open(const char *pszName);
    void AddLayer(OGRS101Layer *);
    static OGRFeatureDefn* S101GenerateDSIDFeatureDefn();
    int GetLayerCount() const override
    { return nLayers; }

    OGRLayer *GetLayer(int iLayer) const override
    {
        if (iLayer < 0 || iLayer >= nLayers)
            return nullptr;
        return papoLayers[iLayer];
    }

    S101Reader *GetModule(int);

    void BuildLayers();
};

#endif //OGR_S101_H
