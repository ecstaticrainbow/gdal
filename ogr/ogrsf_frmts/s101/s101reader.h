//
// Created by tom on 11/21/25.
//

#ifndef S101READER_H
#define S101READER_H

#include <string>
#include <vector>
#include "ogr_feature.h"
#include "iso8211.h"
#include "s57/s57.h"

#include <unordered_map>

/************************************************************************/
/*                              S57Reader                               */
/************************************************************************/

struct S101FeatureTypeRow
{
    int nFTNC = -1;
    std::string osName;
};


class CPL_DLL S101Reader
{

    char *pszModuleName;
    char *pszDSNM;

    DDFModule *poModule;

    int nCMFX; /* X Coordinate multiplier */
    int nCMFY; /* Y Coordinate multiplier */
    int nCMFZ; /* Z Vertical multiplier */

    bool bFileIngested;

    bool bMissingWarningIssued;
    bool bAttrWarningIssued;

    int nFDefnCount;
    OGRFeatureDefn **papoFDefnList;

    DDFRecordIndex oFE_Index;


    int nNextDSIDIndex;
    DDFRecord *poDSIDRecord;
    DDFRecord *poDSSIRecord;
    std::string m_osEDTNUpdate;
    std::string m_osUPDNUpdate;
    std::string m_osISDTUpdate;

    int nOptionFlags;

  public:
    explicit S101Reader(const char *);
    ~S101Reader();

    int GetOptionFlags()
    {
        return nOptionFlags;
    }

    int Open(int bTestOpen);
    bool Ingest();

    DDFModule *GetModule()
    {
        return poModule;
    }

    OGRFeature *ReadDSID();
    void AddFeatureDefn(OGRFeatureDefn *);

    bool CollectClassList(std::vector<int> &anClassCount);
    bool BuildFeatureTypeMap(std::unordered_map<int, S101FeatureTypeRow> &m_oFTNCToType);

    OGRFeature *ReadNextFeature(OGRFeatureDefn * = nullptr);
    OGRFeature *ReadFeature(int nFID, OGRFeatureDefn * = nullptr);
};


#endif //S101READER_H
