//
// Created by tom on 11/21/25.
//

#include "cpl_multiproc.h"
#include "ogr_s101.h"

S101ClassRegistrar *OGRS101Driver::poRegistrar = nullptr;
static CPLMutex *hS101RegistrarMutex = nullptr;

OGRS101Driver::OGRS101Driver()
{
}

OGRS101Driver::~OGRS101Driver()
{
    if (poRegistrar != nullptr)
    {
        delete poRegistrar;
        poRegistrar = nullptr;
    }

    if (hS101RegistrarMutex != nullptr)
    {
        CPLDestroyMutex(hS101RegistrarMutex);
        hS101RegistrarMutex = nullptr;
    }
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

GDALDataset *OGRS101Driver::Open(GDALOpenInfo *poOpenInfo)
{
    auto *poDS = new OGRS101DataSource();

    if (!poDS->Open(poOpenInfo->pszFilename))
    {
        delete poDS;
        poDS = nullptr;
    }

    if (poDS && poOpenInfo->eAccess == GA_Update)
    {
        delete poDS;
        CPLError(CE_Failure, CPLE_OpenFailed,
                 "S57 Driver doesn't support update.");
        return nullptr;
    }

    return poDS;
}

/************************************************************************/
/*                                Identify()                                */
/************************************************************************/

static int OGRS101DriverIdentify(GDALOpenInfo *poOpenInfo)
{
    if (poOpenInfo->nHeaderBytes < 10)
        return false;
    const char *pachLeader = reinterpret_cast<char *>(poOpenInfo->pabyHeader);
    if ((pachLeader[5] != '1' && pachLeader[5] != '2' &&
         pachLeader[5] != '3') ||
        pachLeader[6] != 'L' || (pachLeader[8] != '1' && pachLeader[8] != ' '))
    {
        return false;
    }
    // Test for S-101 DSID field structure (to distinguish it from S-57)
    return strstr(pachLeader, "DSID") != nullptr &&
           (strstr(pachLeader,
                   "RCNM!RCID!ENSP!ENED!PRSP!PRED!PROF!DSNM!DSTL!DSRD!DSLG!DSAB!DSED") != nullptr ||
            // Below is for autotest/ogr/data/s57/fake_s57.000 fake dataset that has a shortened structure
            strstr(pachLeader, "RCNM!RCID!EXPP!xxxx") != nullptr);
}

/************************************************************************/
/*                                Create()                                */
/************************************************************************/

GDALDataset *OGRS101Driver::Create(const char *pszName, int /* nBands */,
                                  int /* nXSize */, int /* nYSize */,
                                  GDALDataType /* eDT */, char **papszOptions)
{
    return nullptr;
}

/************************************************************************/
/*                          GetS101Registrar()                           */
/************************************************************************/

S101ClassRegistrar *OGRS101Driver::GetS101Registrar()
{
    /* -------------------------------------------------------------------- */
    /*      Instantiate the class registrar if possible.                    */
    /* -------------------------------------------------------------------- */
    CPLMutexHolderD(&hS101RegistrarMutex);

    if (poRegistrar == nullptr)
    {
        poRegistrar = new S101ClassRegistrar();

        if (!poRegistrar->LoadInfo("/home/tom/gdal/101_Feature_Catalogue_2.0.0.xml")) // TODO: hardcorded path
        {
            delete poRegistrar;
            poRegistrar = nullptr;
        }
    }

    return poRegistrar;
}

/************************************************************************/
/*                                Register()                                */
/************************************************************************/

void RegisterOGRS101()
{
    if (GDALGetDriverByName("S101") != nullptr)
        return;

    GDALDriver *poDriver = new GDALDriver();

    poDriver->SetDescription("S101");
    poDriver->SetMetadataItem(GDAL_DCAP_VECTOR, "YES");
    poDriver->SetMetadataItem(GDAL_DMD_LONGNAME, "IHO S-101 (ENC)");
    poDriver->SetMetadataItem(GDAL_DMD_EXTENSION, "000");
    poDriver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "drivers/vector/s101.html");

    poDriver->SetMetadataItem(GDAL_DCAP_VIRTUALIO, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_MULTIPLE_VECTOR_LAYERS, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_Z_GEOMETRIES, "YES");

    poDriver->pfnOpen = OGRS101Driver::Open;
    poDriver->pfnIdentify = OGRS101DriverIdentify;
    poDriver->pfnCreate = OGRS101Driver::Create;

    GetGDALDriverManager()->RegisterDriver(poDriver);
}