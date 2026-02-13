//
// Created by tom on 11/21/25.
//

#ifndef S101READER_H
#define S101READER_H

#include <string>
#include <vector>
#include "ogr_feature.h"
#include "iso8211.h"
#include "s101classregistrar.h"

#include <unordered_map>

/************************************************************************/
/*                            DDFRecordIndex                            */
/*                                                                      */
/*      Maintain an index of DDF records based on an integer key.       */
/************************************************************************/

typedef struct
{
    int nKey;
    DDFRecord *poRecord;
    void *pClientData;
} DDFIndexedRecord;

class CPL_DLL DDFRecordIndex
{
    bool bSorted;

    int nRecordCount;
    int nRecordMax;

    int nLastObjlPos;  // Added for FindRecordByObjl().
    int nLastObjl;     // Added for FindRecordByObjl().

    DDFIndexedRecord *pasRecords;

    void Sort();

public:
    DDFRecordIndex();
    ~DDFRecordIndex();

    void AddRecord(int nKey, DDFRecord *);
    bool RemoveRecord(int nKey);

    DDFRecord *FindRecord(int nKey);

    DDFRecord *FindRecordByObjl(int nObjl);  // Added for FindRecordByObjl().

    void Clear();

    int GetCount()
    {
        return nRecordCount;
    }

    DDFRecord *GetByIndex(int i);
    void *GetClientInfoByIndex(int i);
    void SetClientInfoByIndex(int i, void *pClientInfo);
};



/************************************************************************/
/*                              S57Reader                               */
/************************************************************************/

struct S101FeatureTypeRow
{
    int nFTNC = -1;
    CPLString osName;
};


class CPL_DLL S101Reader
{
    S101ClassRegistrar *poRegistrar;
    S101ClassContentExplorer *poClassContentExplorer;

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

    std::unordered_map<int, S101FeatureTypeRow> m_oFTNCToType;
    std::map<std::string, OGRFeatureDefn *> apoFDefnByCode;

    int nNextFEIndex;
    DDFRecordIndex oFE_Index;


    int nNextDSIDIndex;
    DDFRecord *poDSIDRecord;
    DDFRecord *poDSSIRecord;
    std::string m_osEDTNUpdate;
    std::string m_osUPDNUpdate;
    std::string m_osISDTUpdate;

    int nOptionFlags;

    OGRFeatureDefn *FindFDefn(DDFRecord *);

  public:
    explicit S101Reader(const char *);
    ~S101Reader();

    void SetClassBased(S101ClassRegistrar *, S101ClassContentExplorer *);

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
    bool BuildFeatureTypeMap();
    S101FeatureTypeRow GetFeatureTypeByNFTC(int nNFTC)
    {
        return m_oFTNCToType[nNFTC];
    }

    OGRFeature *ReadNextFeature(OGRFeatureDefn * = nullptr);
    OGRFeature *ReadFeature(int nFID, OGRFeatureDefn * = nullptr);
};


#endif //S101READER_H
