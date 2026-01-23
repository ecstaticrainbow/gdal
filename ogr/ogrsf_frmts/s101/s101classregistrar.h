//
// Created by tom on 1/23/26.
//

#ifndef GDAL_S101CLASSREGISTRAR_H
#define GDAL_S101CLASSREGISTRAR_H

struct S101Multiplicity
{
    int  lower = 0;
    int  upper = 1;       // ignored if infinite=true
    bool infinite = false;
};

struct S101AttributeBinding
{
    std::string attrRef;                 // e.g. "colour"
    bool sequential = false;
    S101Multiplicity mult;
    std::vector<std::string> permittedValues;
    std::string visibility;              // e.g. "privateVisibility" (optional)
};

struct S101FeatureTypeDefn
{
    bool isAbstract = false;

    std::string name;
    std::string definition;
    std::string code;    // <-- FTCD key: e.g. "SiloTank"
    std::string alias;   // e.g. "SILTNK"

    std::vector<S101AttributeBinding> attributes;
    std::vector<std::string> permittedPrimitives; // "point", "surface", ...
};


/************************************************************************/
/*                          S101ClassRegistrar                           */
/************************************************************************/

class S101ClassContentExplorer;

class CPL_DLL S101AttrInfo
{
public:
    CPLString osName;
    CPLString osAcronym;
    char chType;
    char chClass;
};

class CPL_DLL S101ClassRegistrar
{
    friend class S101ClassContentExplorer;

    // Class information:
    int nClasses;
    CPLStringList apszClassesInfo;

    // Attribute Information:
    int nAttrCount;
    std::vector<S57AttrInfo *> aoAttrInfos;
    std::vector<int> anAttrIndex;  // sorted by acronym.

    bool m_bLoaded = false;
    std::string m_osLoadedPath;
    std::unordered_map<std::string, S101FeatureTypeDefn> m_oFeatureByCode;


    static bool FindFile(const char *pszTarget, const char *pszDirectory,
                         bool bReportErr, VSILFILE **fp);

    const char *ReadLine(VSILFILE *fp);
    char **papszNextLine;

public:
    S101ClassRegistrar();
    ~S101ClassRegistrar();

    bool LoadInfo(const char *pszFeatureCatalogueXML);

    // attribute table methods.
    // int         GetMaxAttrIndex() { return nAttrMax; }
    const S57AttrInfo *GetAttrInfo(int i);

    const char *GetAttrName(int i)
    {
        return GetAttrInfo(i) == nullptr ? nullptr
                                         : aoAttrInfos[i]->osName.c_str();
    }

    const char *GetAttrAcronym(int i)
    {
        return GetAttrInfo(i) == nullptr ? nullptr
                                         : aoAttrInfos[i]->osAcronym.c_str();
    }

    char GetAttrType(int i)
    {
        return GetAttrInfo(i) == nullptr ? '\0' : aoAttrInfos[i]->chType;
    }

#define SAT_ENUM 'E'
#define SAT_LIST 'L'
#define SAT_FLOAT 'F'
#define SAT_INT 'I'
#define SAT_CODE_STRING 'A'
#define SAT_FREE_TEXT 'S'

    char GetAttrClass(int i)
    {
        return GetAttrInfo(i) == nullptr ? '\0' : aoAttrInfos[i]->chClass;
    }

    int FindAttrByAcronym(const char *);

    std::string LocalName(const char* pszName);

    const CPLXMLNode* ChildByLocal(const CPLXMLNode* n, const char* local);
    const CPLXMLNode* NextByLocal(const CPLXMLNode* n, const char* local);
    std::string TextOf(const CPLXMLNode* n);
    bool AttrTrue(const CPLXMLNode* n, const char* attrName, bool defaultVal=false);
    std::string AttrStr(const CPLXMLNode* n, const char* attrName);
    S101AttributeBinding ParseAttributeBinding(const CPLXMLNode* ab);
    void ParseFeatureTypeNode(const CPLXMLNode* ftNode, S101FeatureTypeDefn& ft);
    // Finds the first node anywhere in the tree with local-name == target.
    const CPLXMLNode* FindFirstByLocalDFS(const CPLXMLNode* root, const char* target);
};

#endif  //GDAL_S101CLASSREGISTRAR_H
