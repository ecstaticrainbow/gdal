//
// Created by tom on 1/23/26.
//

#include "ogr_s101.h"


bool S101ClassRegistrar::FindFile(const char *pszTarget,
                                  const char *pszDirectory, bool bReportErr,
                                  VSILFILE **fp)
{
}

const char *S101ClassRegistrar::ReadLine(VSILFILE *fp)
{
}

S101ClassRegistrar::S101ClassRegistrar()
    : nClasses(0), nAttrCount(0), papszNextLine(nullptr)
{
}

S101ClassRegistrar::~S101ClassRegistrar()
{
}

bool S101ClassRegistrar::LoadInfo(const char *pszFeatureCatalogueXML)
{
    if( pszFeatureCatalogueXML == nullptr || pszFeatureCatalogueXML[0] == '\0' )
        return false;

    // cache: if already loaded same file, nothing to do
    if( m_bLoaded && m_osLoadedPath == pszFeatureCatalogueXML )
        return true;

    m_oFeatureByCode.clear();
    m_bLoaded = false;
    m_osLoadedPath.clear();

    // if (pszDirectory == nullptr)
    //     pszDirectory = CPLGetConfigOption("S101_XML", nullptr);

    CPLXMLNode* poRoot = CPLParseXMLFile("/home/tom/gdal/101_Feature_Catalogue_2.0.0.xml");
    if( poRoot == nullptr )
    {
        CPLError(CE_Failure, CPLE_OpenFailed,
                 "Failed to parse Feature Catalogue XML: %s", pszFeatureCatalogueXML);
        return false;
    }

    // We don’t assume where feature types live in the document; we just find all
    // S100_FC_FeatureType nodes anywhere (namespace-agnostic).
    //
    // Note: This is O(n) in node count; fine for FC files.
    const CPLXMLNode* firstFT = FindFirstByLocalDFS(poRoot, "S100_FC_FeatureType");
    if( firstFT == nullptr )
    {
        CPLDestroyXMLNode(poRoot);
        CPLError(CE_Failure, CPLE_AppDefined,
                 "No S100_FC_FeatureType elements found in: %s", pszFeatureCatalogueXML);
        return false;
    }

    // Walk siblings at the same level if possible; if not, just DFS from each found.
    // Simpler: do a full DFS and parse every matching node.
    //
    // We'll implement a tiny explicit stack DFS to avoid recursion depth concerns.
    std::vector<const CPLXMLNode*> stack;
    stack.push_back(poRoot);

    while( !stack.empty() )
    {
        const CPLXMLNode* n = stack.back();
        stack.pop_back();

        if( n->eType == CXT_Element && LocalName(n->pszValue) == "S100_FC_FeatureType" )
        {
            S101FeatureTypeDefn ft;
            ParseFeatureTypeNode(n, ft);

            // Key by <S100FC:code> (eg "SiloTank") – this is what you’ll join with FTCD
            if( !ft.code.empty() )
            {
                // If duplicates exist, last wins (or change to “first wins” + warning)
                m_oFeatureByCode[ft.code] = std::move(ft);
            }
        }
        else if( n->eType == CXT_Element && LocalName(n->pszValue) == "S100_FC_SimpleAttribute" )
        {
            S101SimpleAttributeDefn ad = ParseSimpleAttributeNode(n);
            if( !ad.code.empty() )
                m_oSimpleAttrsByCode[ad.code] = std::move(ad);
        }
        else if( n->eType == CXT_Element && LocalName(n->pszValue) == "S100_FC_ComplexAttribute" )
        {
            S101ComplexAttributeDefn ca = ParseComplexAttributeNode(n);
            if( !ca.code.empty() )
                m_oComplexAttrsByCode[ca.code] = std::move(ca);
        }

        // DFS: push next then child (order not important)
        if( n->psNext )  stack.push_back(n->psNext);
        if( n->psChild ) stack.push_back(n->psChild);
    }

    CPLDestroyXMLNode(poRoot);

    if( m_oFeatureByCode.empty() )
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Parsed Feature Catalogue but found zero usable feature codes in: %s",
                 pszFeatureCatalogueXML);
        return false;
    }

    m_bLoaded = true;
    m_osLoadedPath = pszFeatureCatalogueXML;

    nClasses = m_oFeatureByCode.size();
    CPLDebug("S101", "Loaded %zu feature types from %s",
             m_oFeatureByCode.size(), pszFeatureCatalogueXML);

    return true;
}

int S101ClassRegistrar::FindAttrByAcronym(const char *)
{
}

std::string S101ClassRegistrar::LocalName(const char* pszName)
{
    if( pszName == nullptr ) return {};
    const char* p = strchr(pszName, ':');
    return p ? std::string(p + 1) : std::string(pszName);
}

const CPLXMLNode *S101ClassRegistrar::ChildByLocal(const CPLXMLNode *n,
                                                   const char *local)
{
    for( const CPLXMLNode* c = n ? n->psChild : nullptr; c; c = c->psNext )
    {
        if( c->eType == CXT_Element && LocalName(c->pszValue) == local )
            return c;
    }
    return nullptr;
}

const CPLXMLNode *S101ClassRegistrar::NextByLocal(const CPLXMLNode *n,
                                                  const char *local)
{
    for( const CPLXMLNode* s = n ? n->psNext : nullptr; s; s = s->psNext )
    {
        if( s->eType == CXT_Element && LocalName(s->pszValue) == local )
            return s;
    }
    return nullptr;
}

std::string S101ClassRegistrar::TextOf(const CPLXMLNode *n)
{
    const CPLXMLNode* t = n ? n->psChild : nullptr;
    return (t && t->eType == CXT_Text && t->pszValue) ? std::string(t->pszValue) : std::string();
}

bool S101ClassRegistrar::AttrTrue(const CPLXMLNode *n, const char *attrName,
                                  bool defaultVal)
{
    const char* v = CPLGetXMLValue(n, attrName, defaultVal ? "true" : "false");
    return v && (EQUAL(v, "true") || EQUAL(v, "1"));
}

std::string S101ClassRegistrar::AttrStr(const CPLXMLNode *n,
                                        const char *attrName)
{
    const char* v = CPLGetXMLValue(n, attrName, nullptr);
    return v ? std::string(v) : std::string();
}

S101AttributeBinding S101ClassRegistrar::ParseAttributeBinding(const CPLXMLNode *ab)
{
    S101AttributeBinding out;
    out.sequential = AttrTrue(ab, "sequential", false);

    // multiplicity
    if( const CPLXMLNode* mult = ChildByLocal(ab, "multiplicity") )
    {
        if( const CPLXMLNode* low = ChildByLocal(mult, "lower") )
            out.mult.lower = atoi(TextOf(low).c_str());

        if( const CPLXMLNode* up = ChildByLocal(mult, "upper") )
        {
            // attributes may be named "infinite" and "xsi:nil"
            out.mult.infinite = AttrTrue(up, "infinite", false);

            const std::string nilAttr = AttrStr(up, "xsi:nil");
            if( !nilAttr.empty() && (nilAttr == "true" || nilAttr == "1") )
            {
                // In your sample, nil=true pairs with infinite=true for unbounded.
                // If nil=true occurs alone, treating it as unbounded is usually safest.
                out.mult.infinite = true;
            }

            if( !out.mult.infinite )
            {
                const std::string sUpper = TextOf(up);
                if( !sUpper.empty() )
                    out.mult.upper = atoi(sUpper.c_str());
            }
        }
    }

    // permittedValues/value*
    if( const CPLXMLNode* pv = ChildByLocal(ab, "permittedValues") )
    {
        for( const CPLXMLNode* v = ChildByLocal(pv, "value"); v; v = NextByLocal(v, "value") )
        {
            const std::string s = TextOf(v);
            if( !s.empty() )
                out.permittedValues.push_back(s);
        }
    }

    // attribute ref="..."
    if( const CPLXMLNode* a = ChildByLocal(ab, "attribute") )
        out.attrRef = AttrStr(a, "ref");

    // attributeVisibility (optional)
    if( const CPLXMLNode* vis = ChildByLocal(ab, "attributeVisibility") )
        out.visibility = TextOf(vis);

    const auto it = m_oSimpleAttrsByCode.find(out.attrRef);
    if( it != m_oSimpleAttrsByCode.end() )
    {
        out.simpleDef = &it->second;
    }
    else
    {
        auto cit = m_oComplexAttrsByCode.find(out.attrRef);
        if( cit != m_oComplexAttrsByCode.end() )
            out.complexDef = &cit->second;
    }

    return out;
}

S101SimpleAttributeDefn S101ClassRegistrar::ParseSimpleAttributeNode(const CPLXMLNode *n)
{
    S101SimpleAttributeDefn out;

    if( const CPLXMLNode* c = ChildByLocal(n, "code") )
        out.code = TextOf(c);

    if( const CPLXMLNode* nm = ChildByLocal(n, "name") )
        out.name = TextOf(nm);

    if( const CPLXMLNode* d = ChildByLocal(n, "definition") )
        out.definition = TextOf(d);

    if( const CPLXMLNode* vt = ChildByLocal(n, "valueType") )
        out.valueType = TextOf(vt);

    if (out.code == "colour")
    {
        auto x = 1;
    }

    // enumeration / listedValues
    if( const CPLXMLNode* lvs = ChildByLocal(n, "listedValues") )
    {
        for( const CPLXMLNode* lv = ChildByLocal(lvs, "listedValue");
             lv;
             lv = NextByLocal(lv, "listedValue") )
        {
            if( const CPLXMLNode* c = ChildByLocal(lv, "code") )
            {
                const std::string code = TextOf(c);
                if( !code.empty() )
                    out.enumeration.push_back(code);
            }
        }
    }

    return out;
}

S101ComplexAttributeDefn S101ClassRegistrar::ParseComplexAttributeNode(const CPLXMLNode *n)
{
    S101ComplexAttributeDefn out;

    if( const CPLXMLNode* c = ChildByLocal(n, "code") )
        out.code = TextOf(c);

    if( const CPLXMLNode* nm = ChildByLocal(n, "name") )
        out.name = TextOf(nm);

    if( const CPLXMLNode* d = ChildByLocal(n, "definition") )
        out.definition = TextOf(d);

    // attributeBinding* (sub-attributes)
    for( const CPLXMLNode* ab = ChildByLocal(n, "subAttributeBinding");
         ab;
         ab = NextByLocal(ab, "subAttributeBinding") )
    {
        S101AttributeBinding b = ParseAttributeBinding(ab);
        if( !b.attrRef.empty() )
            out.subAttributes.push_back(std::move(b));
    }

    return out;
}

void S101ClassRegistrar::ParseFeatureTypeNode(const CPLXMLNode *ftNode,
                                              S101FeatureTypeDefn &ft)
{
    ft.isAbstract = AttrTrue(ftNode, "isAbstract", false);

    if( const CPLXMLNode* n = ChildByLocal(ftNode, "name") )
        ft.name = TextOf(n);

    if( const CPLXMLNode* d = ChildByLocal(ftNode, "definition") )
        ft.definition = TextOf(d);

    if( const CPLXMLNode* c = ChildByLocal(ftNode, "code") )
        ft.code = TextOf(c);

    if( const CPLXMLNode* a = ChildByLocal(ftNode, "alias") )
        ft.alias = TextOf(a);

    // attributeBinding*
    for( const CPLXMLNode* ab = ChildByLocal(ftNode, "attributeBinding");
         ab;
         ab = NextByLocal(ab, "attributeBinding") )
    {
        S101AttributeBinding b = ParseAttributeBinding(ab);
        if( !b.attrRef.empty() )
            ft.attributes.push_back(std::move(b));
    }

    // permittedPrimitives*
    for( const CPLXMLNode* pp = ChildByLocal(ftNode, "permittedPrimitives");
         pp;
         pp = NextByLocal(pp, "permittedPrimitives") )
    {
        const std::string prim = TextOf(pp);
        if( !prim.empty() )
            ft.permittedPrimitives.push_back(prim);
    }

    // TODO:  If you later care about informationBinding / featureBinding:
    // parse those here in the same style.
}

const CPLXMLNode* S101ClassRegistrar::FindFirstByLocalDFS(const CPLXMLNode *root,
                                        const char *target)
{
    {
        if( !root ) return nullptr;

        if( root->eType == CXT_Element && LocalName(root->pszValue) == target )
            return root;

        if( const CPLXMLNode* inChild = FindFirstByLocalDFS(root->psChild, target) )
            return inChild;

        return FindFirstByLocalDFS(root->psNext, target);
    }
}

/************************************************************************/
/*                        S57ClassContentExplorer()                     */
/************************************************************************/

S101ClassContentExplorer::S101ClassContentExplorer(
    S101ClassRegistrar *poRegistrarIn)
    : poRegistrar(poRegistrarIn), papapszClassesFields(nullptr),
        papszCurrentFields(nullptr), papszTempResult(nullptr)
{
}

/************************************************************************/
/*                        ~S57ClassContentExplorer()                    */
/************************************************************************/

S101ClassContentExplorer::~S101ClassContentExplorer()
{
    CSLDestroy(papszTempResult);

    if (papapszClassesFields != nullptr)
    {
        for (int i = 0; i < poRegistrar->nClasses; i++)
            CSLDestroy(papapszClassesFields[i]);
        CPLFree(papapszClassesFields);
    }
}

/************************************************************************/
/*                            SelectClass()                             */
/************************************************************************/

bool S101ClassContentExplorer::SelectClass(const char *pszFeatureCode)

{
    auto it = poRegistrar->m_oFeatureByCode.find(pszFeatureCode);
    if (it != poRegistrar->m_oFeatureByCode.end())
    {
        S101FeatureTypeDefn def = it->second;
        pCurrentFeatureDef = def;
        return true;
    }

    return false;
}

/************************************************************************/
/*                              GetOBJL()                               */
/************************************************************************/

int S101ClassContentExplorer::GetOBJL()

{
    // if (iCurrentClass >= 0)
    //     return atoi(poRegistrar->apszClassesInfo[iCurrentClass]);

    return -1;
}

/************************************************************************/
/*                           GetDescription()                           */
/************************************************************************/

const char *S101ClassContentExplorer::GetDescription() const

{
    // if (iCurrentClass >= 0 && papszCurrentFields[0] != nullptr)
    //     return papszCurrentFields[1];

    return nullptr;
}

/************************************************************************/
/*                             GetAcronym()                             */
/************************************************************************/

const char *S101ClassContentExplorer::GetAcronym() const

{
    return pCurrentFeatureDef.code.c_str();
    // if (iCurrentClass >= 0 && papszCurrentFields[0] != nullptr &&
    //     papszCurrentFields[1] != nullptr)
    //     return papszCurrentFields[2];

    return nullptr;
}

/************************************************************************/
/*                          GetAttributeList()                          */
/*                                                                      */
/*      The passed string can be "a", "b", "c" or NULL for all.  The    */
/*      returned list remained owned by this object, not the caller.    */
/************************************************************************/

std::vector<S101AttributeBinding> S101ClassContentExplorer::GetAttributeList()

{
    return pCurrentFeatureDef.attributes;
}

/************************************************************************/
/*                            GetClassCode()                            */
/************************************************************************/

char S101ClassContentExplorer::GetClassCode() const

{
    // if (iCurrentClass >= 0 && papszCurrentFields[0] != nullptr &&
    //     papszCurrentFields[1] != nullptr && papszCurrentFields[2] != nullptr &&
    //     papszCurrentFields[3] != nullptr && papszCurrentFields[4] != nullptr &&
    //     papszCurrentFields[5] != nullptr && papszCurrentFields[6] != nullptr)
    //     return papszCurrentFields[6][0];

    return '\0';
}

/************************************************************************/
/*                           GetPrimitives()                            */
/************************************************************************/

std::vector<std::string> S101ClassContentExplorer::GetPrimitives() const

{
    return pCurrentFeatureDef.permittedPrimitives;
    // if (iCurrentClass >= 0 && CSLCount(papszCurrentFields) > 7)
    // {
    //     CSLDestroy(papszTempResult);
    //     papszTempResult =
    //         CSLTokenizeStringComplex(papszCurrentFields[7], ";", TRUE, FALSE);
    //     return papszTempResult;
    // }
}

