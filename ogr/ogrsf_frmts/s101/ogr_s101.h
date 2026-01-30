//
// Created by tom on 11/21/25.
//

#ifndef OGR_S101_H
#define OGR_S101_H

#include "ogrsf_frmts.h"
#include "s101reader.h"
#include "s101classregistrar.h"

class OGRS101DataSource;

/************************************************************************/
/*                           OGRS101Layer                               */
/************************************************************************/

class OGRS101Layer : public OGRLayer
{
    OGRS101DataSource *poDS;

    OGRFeatureDefn* poFeatureDefn = nullptr;
    int nCurrentModule;

    OGRSpatialReference* poSRS = nullptr;
    size_t iNext = 0;

public:
    OGRS101Layer(OGRS101DataSource *poDSIn, OGRFeatureDefn *poDefnIn);

    ~OGRS101Layer() override;

    OGRFeatureDefn* GetLayerDefn() const override { return poFeatureDefn; }

    void ResetReading() override;
    int TestCapability(const char *) const override;

    OGRFeature *GetNextFeature() override;
    OGRFeature *GetNextUnfilteredFeature();
    OGRFeature *GetFeature(GIntBig nFeatureId) override;
};

/************************************************************************/
/*                           OGRS101DataSource                          */
/************************************************************************/

class OGRS101DataSource final : public GDALDataset
{

public:

    int nModules;
    S101Reader **papoModules;

    S101ClassContentExplorer *poClassContentExplorer;

    OGRS101Layer **papoLayers;
    int nLayers;
    // TODO
    const char *pszFilename;
    explicit OGRS101DataSource(char **papszOpenOptions = nullptr);
    ~OGRS101DataSource() override;

    int Open(const char *pszName);
    void AddLayer(OGRS101Layer *);
    int GetLayerCount() const override
    {
        return nLayers;
    }

    int GetModuleCount()
    {
        return nModules;
    }

    using GDALDataset::GetLayer;
    const OGRLayer *GetLayer(int) const override;

    S101Reader *GetModule(int) const;
};

/************************************************************************/
/*                            OGRS57Driver                              */
/************************************************************************/

class OGRS101Driver final : public GDALDriver
{
    static S101ClassRegistrar *poRegistrar;

public:
    OGRS101Driver();
    ~OGRS101Driver() override;

    static GDALDataset *Open(GDALOpenInfo *poOpenInfo);
    static GDALDataset *Create(const char *pszName, int nBands, int nXSize,
                               int nYSize, GDALDataType eDT,
                               char **papszOptions);
    static S101ClassRegistrar *GetS101Registrar();
};


/* -------------------------------------------------------------------- */
/*      Functions to create OGRFeatureDefns.                            */
/* -------------------------------------------------------------------- */
void CPL_DLL S101GenerateStandardAttributes(OGRFeatureDefn *, int);
OGRFeatureDefn CPL_DLL *S101GenerateGeomFeatureDefn(OGRwkbGeometryType, int);
OGRFeatureDefn CPL_DLL *
S101GenerateObjectClassDefn(S101ClassRegistrar *,
                           S101ClassContentExplorer *poClassContentExplorer,
                           const char*,
                           int);
OGRFeatureDefn CPL_DLL *S101GenerateVectorPrimitiveFeatureDefn(int, int);
OGRFeatureDefn CPL_DLL *S101GenerateDSIDFeatureDefn(void);

#endif //OGR_S101_H
