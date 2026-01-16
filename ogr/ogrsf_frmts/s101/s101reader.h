//
// Created by tom on 11/21/25.
//

#ifndef S101READER_H
#define S101READER_H

#include <string>
#include <vector>
#include "ogr_feature.h"
#include "iso8211.h"

/************************************************************************/
/*                              S57Reader                               */
/************************************************************************/

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
    OGRFeature *ReadNextFeature(OGRFeatureDefn * = nullptr);
    OGRFeature *ReadFeature(int nFID, OGRFeatureDefn * = nullptr);
};


#endif //S101READER_H
