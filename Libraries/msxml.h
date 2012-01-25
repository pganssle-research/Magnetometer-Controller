#ifndef _MSXML2_H
#define _MSXML2_H

#if defined(INCLUDE_AFTER_WINDOWS_H) && !defined(_INC_WINDOWS)
#error  This header must be included before utility.h and formatio.h
#error  because it includes cviauto.h which includes Windows SDK headers.
#endif /* INCLUDE_AFTER_WINDOWS_H */

#include <cviauto.h>

#ifdef __cplusplus
    extern "C" {
#endif
/* NICDBLD_BEGIN> Type Library Specific Types */

enum MSXML2Enum_tagDOMNodeType
{
	MSXML2Const_NODE_INVALID = 0,
	MSXML2Const_NODE_ELEMENT = 1,
	MSXML2Const_NODE_ATTRIBUTE = 2,
	MSXML2Const_NODE_TEXT = 3,
	MSXML2Const_NODE_CDATA_SECTION = 4,
	MSXML2Const_NODE_ENTITY_REFERENCE = 5,
	MSXML2Const_NODE_ENTITY = 6,
	MSXML2Const_NODE_PROCESSING_INSTRUCTION = 7,
	MSXML2Const_NODE_COMMENT = 8,
	MSXML2Const_NODE_DOCUMENT = 9,
	MSXML2Const_NODE_DOCUMENT_TYPE = 10,
	MSXML2Const_NODE_DOCUMENT_FRAGMENT = 11,
	MSXML2Const_NODE_NOTATION = 12,
	_MSXML2_tagDOMNodeTypeForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SERVERXMLHTTP_OPTION
{
	MSXML2Const_SXH_OPTION_URL = -1,
	MSXML2Const_SXH_OPTION_URL_CODEPAGE = 0,
	MSXML2Const_SXH_OPTION_ESCAPE_PERCENT_IN_URL = 1,
	MSXML2Const_SXH_OPTION_IGNORE_SERVER_SSL_CERT_ERROR_FLAGS = 2,
	MSXML2Const_SXH_OPTION_SELECT_CLIENT_SSL_CERT = 3,
	_MSXML2__SERVERXMLHTTP_OPTIONForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SXH_PROXY_SETTING
{
	MSXML2Const_SXH_PROXY_SET_DEFAULT = 0,
	MSXML2Const_SXH_PROXY_SET_PRECONFIG = 0,
	MSXML2Const_SXH_PROXY_SET_DIRECT = 1,
	MSXML2Const_SXH_PROXY_SET_PROXY = 2,
	_MSXML2__SXH_PROXY_SETTINGForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SOMITEMTYPE
{
	MSXML2Const_SOMITEM_SCHEMA = 4096,
	MSXML2Const_SOMITEM_ATTRIBUTE = 4097,
	MSXML2Const_SOMITEM_ATTRIBUTEGROUP = 4098,
	MSXML2Const_SOMITEM_NOTATION = 4099,
	MSXML2Const_SOMITEM_ANNOTATION = 4100,
	MSXML2Const_SOMITEM_IDENTITYCONSTRAINT = 4352,
	MSXML2Const_SOMITEM_KEY = 4353,
	MSXML2Const_SOMITEM_KEYREF = 4354,
	MSXML2Const_SOMITEM_UNIQUE = 4355,
	MSXML2Const_SOMITEM_ANYTYPE = 8192,
	MSXML2Const_SOMITEM_DATATYPE = 8448,
	MSXML2Const_SOMITEM_DATATYPE_ANYTYPE = 8449,
	MSXML2Const_SOMITEM_DATATYPE_ANYURI = 8450,
	MSXML2Const_SOMITEM_DATATYPE_BASE64BINARY = 8451,
	MSXML2Const_SOMITEM_DATATYPE_BOOLEAN = 8452,
	MSXML2Const_SOMITEM_DATATYPE_BYTE = 8453,
	MSXML2Const_SOMITEM_DATATYPE_DATE = 8454,
	MSXML2Const_SOMITEM_DATATYPE_DATETIME = 8455,
	MSXML2Const_SOMITEM_DATATYPE_DAY = 8456,
	MSXML2Const_SOMITEM_DATATYPE_DECIMAL = 8457,
	MSXML2Const_SOMITEM_DATATYPE_DOUBLE = 8458,
	MSXML2Const_SOMITEM_DATATYPE_DURATION = 8459,
	MSXML2Const_SOMITEM_DATATYPE_ENTITIES = 8460,
	MSXML2Const_SOMITEM_DATATYPE_ENTITY = 8461,
	MSXML2Const_SOMITEM_DATATYPE_FLOAT = 8462,
	MSXML2Const_SOMITEM_DATATYPE_HEXBINARY = 8463,
	MSXML2Const_SOMITEM_DATATYPE_ID = 8464,
	MSXML2Const_SOMITEM_DATATYPE_IDREF = 8465,
	MSXML2Const_SOMITEM_DATATYPE_IDREFS = 8466,
	MSXML2Const_SOMITEM_DATATYPE_INT = 8467,
	MSXML2Const_SOMITEM_DATATYPE_INTEGER = 8468,
	MSXML2Const_SOMITEM_DATATYPE_LANGUAGE = 8469,
	MSXML2Const_SOMITEM_DATATYPE_LONG = 8470,
	MSXML2Const_SOMITEM_DATATYPE_MONTH = 8471,
	MSXML2Const_SOMITEM_DATATYPE_MONTHDAY = 8472,
	MSXML2Const_SOMITEM_DATATYPE_NAME = 8473,
	MSXML2Const_SOMITEM_DATATYPE_NCNAME = 8474,
	MSXML2Const_SOMITEM_DATATYPE_NEGATIVEINTEGER = 8475,
	MSXML2Const_SOMITEM_DATATYPE_NMTOKEN = 8476,
	MSXML2Const_SOMITEM_DATATYPE_NMTOKENS = 8477,
	MSXML2Const_SOMITEM_DATATYPE_NONNEGATIVEINTEGER = 8478,
	MSXML2Const_SOMITEM_DATATYPE_NONPOSITIVEINTEGER = 8479,
	MSXML2Const_SOMITEM_DATATYPE_NORMALIZEDSTRING = 8480,
	MSXML2Const_SOMITEM_DATATYPE_NOTATION = 8481,
	MSXML2Const_SOMITEM_DATATYPE_POSITIVEINTEGER = 8482,
	MSXML2Const_SOMITEM_DATATYPE_QNAME = 8483,
	MSXML2Const_SOMITEM_DATATYPE_SHORT = 8484,
	MSXML2Const_SOMITEM_DATATYPE_STRING = 8485,
	MSXML2Const_SOMITEM_DATATYPE_TIME = 8486,
	MSXML2Const_SOMITEM_DATATYPE_TOKEN = 8487,
	MSXML2Const_SOMITEM_DATATYPE_UNSIGNEDBYTE = 8488,
	MSXML2Const_SOMITEM_DATATYPE_UNSIGNEDINT = 8489,
	MSXML2Const_SOMITEM_DATATYPE_UNSIGNEDLONG = 8490,
	MSXML2Const_SOMITEM_DATATYPE_UNSIGNEDSHORT = 8491,
	MSXML2Const_SOMITEM_DATATYPE_YEAR = 8492,
	MSXML2Const_SOMITEM_DATATYPE_YEARMONTH = 8493,
	MSXML2Const_SOMITEM_DATATYPE_ANYSIMPLETYPE = 8703,
	MSXML2Const_SOMITEM_SIMPLETYPE = 8704,
	MSXML2Const_SOMITEM_COMPLEXTYPE = 9216,
	MSXML2Const_SOMITEM_PARTICLE = 16384,
	MSXML2Const_SOMITEM_ANY = 16385,
	MSXML2Const_SOMITEM_ANYATTRIBUTE = 16386,
	MSXML2Const_SOMITEM_ELEMENT = 16387,
	MSXML2Const_SOMITEM_GROUP = 16640,
	MSXML2Const_SOMITEM_ALL = 16641,
	MSXML2Const_SOMITEM_CHOICE = 16642,
	MSXML2Const_SOMITEM_SEQUENCE = 16643,
	MSXML2Const_SOMITEM_EMPTYPARTICLE = 16644,
	MSXML2Const_SOMITEM_NULL = 2048,
	MSXML2Const_SOMITEM_NULL_TYPE = 10240,
	MSXML2Const_SOMITEM_NULL_ANY = 18433,
	MSXML2Const_SOMITEM_NULL_ANYATTRIBUTE = 18434,
	MSXML2Const_SOMITEM_NULL_ELEMENT = 18435,
	_MSXML2__SOMITEMTYPEForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SCHEMADERIVATIONMETHOD
{
	MSXML2Const_SCHEMADERIVATIONMETHOD_EMPTY = 0,
	MSXML2Const_SCHEMADERIVATIONMETHOD_SUBSTITUTION = 1,
	MSXML2Const_SCHEMADERIVATIONMETHOD_EXTENSION = 2,
	MSXML2Const_SCHEMADERIVATIONMETHOD_RESTRICTION = 4,
	MSXML2Const_SCHEMADERIVATIONMETHOD_LIST = 8,
	MSXML2Const_SCHEMADERIVATIONMETHOD_UNION = 16,
	MSXML2Const_SCHEMADERIVATIONMETHOD_ALL = 255,
	MSXML2Const_SCHEMADERIVATIONMETHOD_NONE = 256,
	_MSXML2__SCHEMADERIVATIONMETHODForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SCHEMATYPEVARIETY
{
	MSXML2Const_SCHEMATYPEVARIETY_NONE = -1,
	MSXML2Const_SCHEMATYPEVARIETY_ATOMIC = 0,
	MSXML2Const_SCHEMATYPEVARIETY_LIST = 1,
	MSXML2Const_SCHEMATYPEVARIETY_UNION = 2,
	_MSXML2__SCHEMATYPEVARIETYForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SCHEMAWHITESPACE
{
	MSXML2Const_SCHEMAWHITESPACE_NONE = -1,
	MSXML2Const_SCHEMAWHITESPACE_PRESERVE = 0,
	MSXML2Const_SCHEMAWHITESPACE_REPLACE = 1,
	MSXML2Const_SCHEMAWHITESPACE_COLLAPSE = 2,
	_MSXML2__SCHEMAWHITESPACEForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SCHEMACONTENTTYPE
{
	MSXML2Const_SCHEMACONTENTTYPE_EMPTY = 0,
	MSXML2Const_SCHEMACONTENTTYPE_TEXTONLY = 1,
	MSXML2Const_SCHEMACONTENTTYPE_ELEMENTONLY = 2,
	MSXML2Const_SCHEMACONTENTTYPE_MIXED = 3,
	_MSXML2__SCHEMACONTENTTYPEForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SCHEMAPROCESSCONTENTS
{
	MSXML2Const_SCHEMAPROCESSCONTENTS_NONE = 0,
	MSXML2Const_SCHEMAPROCESSCONTENTS_SKIP = 1,
	MSXML2Const_SCHEMAPROCESSCONTENTS_LAX = 2,
	MSXML2Const_SCHEMAPROCESSCONTENTS_STRICT = 3,
	_MSXML2__SCHEMAPROCESSCONTENTSForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SCHEMAUSE
{
	MSXML2Const_SCHEMAUSE_OPTIONAL = 0,
	MSXML2Const_SCHEMAUSE_PROHIBITED = 1,
	MSXML2Const_SCHEMAUSE_REQUIRED = 2,
	_MSXML2__SCHEMAUSEForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum_tagXMLEMEM_TYPE
{
	MSXML2Const_XMLELEMTYPE_ELEMENT = 0,
	MSXML2Const_XMLELEMTYPE_TEXT = 1,
	MSXML2Const_XMLELEMTYPE_COMMENT = 2,
	MSXML2Const_XMLELEMTYPE_DOCUMENT = 3,
	MSXML2Const_XMLELEMTYPE_DTD = 4,
	MSXML2Const_XMLELEMTYPE_PI = 5,
	MSXML2Const_XMLELEMTYPE_OTHER = 6,
	_MSXML2_tagXMLEMEM_TYPEForceSizeToFourBytes = 0xFFFFFFFF
};
enum MSXML2Enum__SXH_SERVER_CERT_OPTION
{
	MSXML2Const_SXH_SERVER_CERT_IGNORE_UNKNOWN_CA = 256,
	MSXML2Const_SXH_SERVER_CERT_IGNORE_WRONG_USAGE = 512,
	MSXML2Const_SXH_SERVER_CERT_IGNORE_CERT_CN_INVALID = 4096,
	MSXML2Const_SXH_SERVER_CERT_IGNORE_CERT_DATE_INVALID = 8192,
	MSXML2Const_SXH_SERVER_CERT_IGNORE_ALL_SERVER_ERRORS = 13056,
	_MSXML2__SXH_SERVER_CERT_OPTIONForceSizeToFourBytes = 0xFFFFFFFF
};
typedef long MSXML2Type_DOMNodeType;
typedef CAObjHandle MSXML2Obj_IXMLDOMNode;
typedef CAObjHandle MSXML2Obj_IXMLDOMNodeList;
typedef CAObjHandle MSXML2Obj_IXMLDOMNamedNodeMap;
typedef CAObjHandle MSXML2Obj_IXMLDOMDocument;
typedef CAObjHandle MSXML2Obj_IXMLDOMDocumentType;
typedef CAObjHandle MSXML2Obj_IXMLDOMImplementation;
typedef CAObjHandle MSXML2Obj_IXMLDOMElement;
typedef CAObjHandle MSXML2Obj_IXMLDOMDocumentFragment;
typedef CAObjHandle MSXML2Obj_IXMLDOMText;
typedef CAObjHandle MSXML2Obj_IXMLDOMComment;
typedef CAObjHandle MSXML2Obj_IXMLDOMCDATASection;
typedef CAObjHandle MSXML2Obj_IXMLDOMProcessingInstruction;
typedef CAObjHandle MSXML2Obj_IXMLDOMAttribute;
typedef CAObjHandle MSXML2Obj_IXMLDOMEntityReference;
typedef CAObjHandle MSXML2Obj_IXMLDOMParseError;
typedef CAObjHandle MSXML2Obj_IXMLDOMSchemaCollection;
typedef CAObjHandle MSXML2Obj_ISchema;
typedef CAObjHandle MSXML2Obj_ISchemaItem;
typedef CAObjHandle MSXML2Obj_IXSLProcessor;
typedef long MSXML2Type_SERVERXMLHTTP_OPTION;
typedef long MSXML2Type_SXH_PROXY_SETTING;
typedef CAObjHandle MSXML2Obj_IVBSAXEntityResolver;
typedef CAObjHandle MSXML2Obj_IVBSAXContentHandler;
typedef CAObjHandle MSXML2Obj_IVBSAXDTDHandler;
typedef CAObjHandle MSXML2Obj_IVBSAXErrorHandler;
typedef CAObjHandle MSXML2Obj_ISAXEntityResolver;
typedef CAObjHandle MSXML2Obj_ISAXContentHandler;
typedef CAObjHandle MSXML2Obj_ISAXDTDHandler;
typedef CAObjHandle MSXML2Obj_ISAXErrorHandler;
typedef CAObjHandle MSXML2Obj_ISAXLocator;
typedef CAObjHandle MSXML2Obj_ISAXAttributes;
typedef CAObjHandle MSXML2Obj_IVBSAXLocator;
typedef CAObjHandle MSXML2Obj_IVBSAXAttributes;
typedef CAObjHandle MSXML2Obj_IMXNamespacePrefixes;
typedef CAObjHandle MSXML2Obj_IXMLDOMNode;
typedef CAObjHandle MSXML2Obj_IXMLElement2;
typedef CAObjHandle MSXML2Obj_IXSLTemplate;
typedef CAObjHandle MSXML2Obj_ISAXXMLReader;
typedef CAObjHandle MSXML2Obj_IVBSAXXMLReader;
typedef CAObjHandle MSXML2Obj_ISchemaElement;
typedef long MSXML2Type_SOMITEMTYPE;
typedef CAObjHandle MSXML2Obj_ISchemaType;
typedef CAObjHandle MSXML2Obj_ISchemaComplexType;
typedef CAObjHandle MSXML2Obj_ISchemaItemCollection;
typedef long MSXML2Type_SCHEMADERIVATIONMETHOD;
typedef CAObjHandle MSXML2Obj_ISchemaStringCollection;
typedef long MSXML2Type_SCHEMATYPEVARIETY;
typedef long MSXML2Type_SCHEMAWHITESPACE;
typedef CAObjHandle MSXML2Obj_ISchemaAny;
typedef long MSXML2Type_SCHEMACONTENTTYPE;
typedef CAObjHandle MSXML2Obj_ISchemaModelGroup;
typedef long MSXML2Type_SCHEMAPROCESSCONTENTS;
typedef long MSXML2Type_SCHEMAUSE;
typedef CAObjHandle MSXML2Obj_ISchemaIdentityConstraint;
typedef CAObjHandle MSXML2Obj_IXMLElement;
typedef CAObjHandle MSXML2Obj_IXMLElementCollection;
typedef CAObjHandle MSXML2Obj_IXMLElement2;
typedef CAObjHandle MSXML2Obj_IXMLDOMSelection;
typedef long MSXML2Type_XMLELEM_TYPE;
typedef long MSXML2Type_SXH_SERVER_CERT_OPTION;
typedef HRESULT (CVICALLBACK *XMLDOMDocumentEventsRegOnondataavailable_CallbackType) (CAObjHandle caServerObjHandle,
                                                                                      void *caCallbackData,
                                                                                      long *__returnValue);
typedef HRESULT (CVICALLBACK *XMLDOMDocumentEventsRegOnonreadystatechange_CallbackType) (CAObjHandle caServerObjHandle,
                                                                                         void *caCallbackData,
                                                                                         long *__returnValue);
/* NICDBLD_END> Type Library Specific Types */

extern const IID MSXML2_IID_IXMLDOMDocument2;
extern const IID MSXML2_IID_IXMLDOMSchemaCollection;
extern const IID MSXML2_IID_IXMLDOMSchemaCollection2;
extern const IID MSXML2_IID_IXSLTemplate;
extern const IID MSXML2_IID_IDSOControl;
extern const IID MSXML2_IID_IXMLHTTPRequest;
extern const IID MSXML2_IID_IServerXMLHTTPRequest2;
extern const IID MSXML2_IID_IVBSAXXMLReader;
extern const IID MSXML2_IID_ISAXXMLReader;
extern const IID MSXML2_IID_IMXReaderControl;
extern const IID MSXML2_IID_IMXWriter;
extern const IID MSXML2_IID_ISAXContentHandler;
extern const IID MSXML2_IID_ISAXErrorHandler;
extern const IID MSXML2_IID_ISAXDTDHandler;
extern const IID MSXML2_IID_ISAXLexicalHandler;
extern const IID MSXML2_IID_ISAXDeclHandler;
extern const IID MSXML2_IID_IVBSAXContentHandler;
extern const IID MSXML2_IID_IVBSAXDeclHandler;
extern const IID MSXML2_IID_IVBSAXDTDHandler;
extern const IID MSXML2_IID_IVBSAXErrorHandler;
extern const IID MSXML2_IID_IVBSAXLexicalHandler;
extern const IID MSXML2_IID_IMXAttributes;
extern const IID MSXML2_IID_IVBSAXAttributes;
extern const IID MSXML2_IID_ISAXAttributes;
extern const IID MSXML2_IID_IVBMXNamespaceManager;
extern const IID MSXML2_IID_IMXNamespaceManager;
extern const IID MSXML2_IID_IXMLDocument2;
extern const IID MSXML2_IID_XMLDOMDocumentEvents;
extern const IID MSXML2_IID_IXMLDOMImplementation;
extern const IID MSXML2_IID_IXMLDOMNode;
extern const IID MSXML2_IID_IXMLDOMNodeList;
extern const IID MSXML2_IID_IXMLDOMNamedNodeMap;
extern const IID MSXML2_IID_IXMLDOMDocument;
extern const IID MSXML2_IID_IXMLDOMDocumentType;
extern const IID MSXML2_IID_IXMLDOMElement;
extern const IID MSXML2_IID_IXMLDOMAttribute;
extern const IID MSXML2_IID_IXMLDOMDocumentFragment;
extern const IID MSXML2_IID_IXMLDOMText;
extern const IID MSXML2_IID_IXMLDOMCharacterData;
extern const IID MSXML2_IID_IXMLDOMComment;
extern const IID MSXML2_IID_IXMLDOMCDATASection;
extern const IID MSXML2_IID_IXMLDOMProcessingInstruction;
extern const IID MSXML2_IID_IXMLDOMEntityReference;
extern const IID MSXML2_IID_IXMLDOMParseError;
extern const IID MSXML2_IID_IXMLDOMNotation;
extern const IID MSXML2_IID_IXMLDOMEntity;
extern const IID MSXML2_IID_IXTLRuntime;
extern const IID MSXML2_IID_IXSLProcessor;
extern const IID MSXML2_IID_ISAXEntityResolver;
extern const IID MSXML2_IID_ISAXLocator;
extern const IID MSXML2_IID_ISAXXMLFilter;
extern const IID MSXML2_IID_IVBSAXEntityResolver;
extern const IID MSXML2_IID_IVBSAXLocator;
extern const IID MSXML2_IID_IVBSAXXMLFilter;
extern const IID MSXML2_IID_IMXSchemaDeclHandler;
extern const IID MSXML2_IID_ISchemaElement;
extern const IID MSXML2_IID_ISchemaParticle;
extern const IID MSXML2_IID_ISchemaItem;
extern const IID MSXML2_IID_ISchema;
extern const IID MSXML2_IID_ISchemaItemCollection;
extern const IID MSXML2_IID_ISchemaStringCollection;
extern const IID MSXML2_IID_ISchemaType;
extern const IID MSXML2_IID_ISchemaComplexType;
extern const IID MSXML2_IID_ISchemaAny;
extern const IID MSXML2_IID_ISchemaModelGroup;
extern const IID MSXML2_IID_ISchemaAttribute;
extern const IID MSXML2_IID_ISchemaAttributeGroup;
extern const IID MSXML2_IID_ISchemaIdentityConstraint;
extern const IID MSXML2_IID_ISchemaNotation;
extern const IID MSXML2_IID_IXMLElementCollection;
extern const IID MSXML2_IID_IXMLDocument;
extern const IID MSXML2_IID_IXMLElement;
extern const IID MSXML2_IID_IXMLElement2;
extern const IID MSXML2_IID_IXMLAttribute;
extern const IID MSXML2_IID_IXMLDOMSelection;
extern const IID MSXML2_IID_IServerXMLHTTPRequest;
extern const IID MSXML2_IID_IMXNamespacePrefixes;

HRESULT CVIFUNC MSXML2_NewDOMDocumentIXMLDOMDocument2 (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenDOMDocumentIXMLDOMDocument2 (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveDOMDocumentIXMLDOMDocument2 (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewDOMDocument26IXMLDOMDocument2 (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenDOMDocument26IXMLDOMDocument2 (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveDOMDocument26IXMLDOMDocument2 (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewDOMDocument30IXMLDOMDocument2 (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenDOMDocument30IXMLDOMDocument2 (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveDOMDocument30IXMLDOMDocument2 (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewDOMDocument40IXMLDOMDocument2 (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenDOMDocument40IXMLDOMDocument2 (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveDOMDocument40IXMLDOMDocument2 (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewFreeThreadedDOMDocumentIXMLDOMDocument2 (const char *server,
                                                                   int supportMultithreading,
                                                                   LCID locale,
                                                                   int reserved,
                                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenFreeThreadedDOMDocumentIXMLDOMDocument2 (const char *fileName,
                                                                    const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveFreeThreadedDOMDocumentIXMLDOMDocument2 (const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewFreeThreadedDOMDocument26IXMLDOMDocument2 (const char *server,
                                                                     int supportMultithreading,
                                                                     LCID locale,
                                                                     int reserved,
                                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenFreeThreadedDOMDocument26IXMLDOMDocument2 (const char *fileName,
                                                                      const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveFreeThreadedDOMDocument26IXMLDOMDocument2 (const char *server,
                                                                        int supportMultithreading,
                                                                        LCID locale,
                                                                        int reserved,
                                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewFreeThreadedDOMDocument30IXMLDOMDocument2 (const char *server,
                                                                     int supportMultithreading,
                                                                     LCID locale,
                                                                     int reserved,
                                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenFreeThreadedDOMDocument30IXMLDOMDocument2 (const char *fileName,
                                                                      const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveFreeThreadedDOMDocument30IXMLDOMDocument2 (const char *server,
                                                                        int supportMultithreading,
                                                                        LCID locale,
                                                                        int reserved,
                                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewFreeThreadedDOMDocument40IXMLDOMDocument2 (const char *server,
                                                                     int supportMultithreading,
                                                                     LCID locale,
                                                                     int reserved,
                                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenFreeThreadedDOMDocument40IXMLDOMDocument2 (const char *fileName,
                                                                      const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveFreeThreadedDOMDocument40IXMLDOMDocument2 (const char *server,
                                                                        int supportMultithreading,
                                                                        LCID locale,
                                                                        int reserved,
                                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetnodeName (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetnodeValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2SetnodeValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetnodeType (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetparentNode (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetchildNodes (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetfirstChild (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetlastChild (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetpreviousSibling (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetnextSibling (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getattributes (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2insertBefore (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode newChild,
                                                     VARIANT refChild,
                                                     MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2replaceChild (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode newChild,
                                                     MSXML2Obj_IXMLDOMNode oldChild,
                                                     MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2removeChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode childNode,
                                                    MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2appendChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode newChild,
                                                    MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2hasChildNodes (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetownerDocument (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2cloneNode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VBOOL deep,
                                                  MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetnodeTypeString (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Gettext (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Settext (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getspecified (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getdefinition (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetnodeTypedValue (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2SetnodeTypedValue (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetdataType (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2SetdataType (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getxml (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2transformNode (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode stylesheet,
                                                      char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2selectNodes (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *queryString,
                                                    MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2selectSingleNode (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         const char *queryString,
                                                         MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getparsed (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetnamespaceURI (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getprefix (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetbaseName (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2transformNodeToObject (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IXMLDOMNode stylesheet,
                                                              VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getdoctype (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMDocumentType *documentType);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getimplementation (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMImplementation *impl);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetdocumentElement (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMElement *DOMElement);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2SetByRefdocumentElement (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                MSXML2Obj_IXMLDOMElement DOMElement);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createElement (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      const char *tagName,
                                                      MSXML2Obj_IXMLDOMElement *element);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createDocumentFragment (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               MSXML2Obj_IXMLDOMDocumentFragment *docFrag);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createTextNode (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *data,
                                                       MSXML2Obj_IXMLDOMText *text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createComment (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      const char *data,
                                                      MSXML2Obj_IXMLDOMComment *comment);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createCDATASection (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           const char *data,
                                                           MSXML2Obj_IXMLDOMCDATASection *cdata);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createProcessingInstruction (CAObjHandle objectHandle,
                                                                    ERRORINFO *errorInfo,
                                                                    const char *target,
                                                                    const char *data,
                                                                    MSXML2Obj_IXMLDOMProcessingInstruction *pi);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createAttribute (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *name,
                                                        MSXML2Obj_IXMLDOMAttribute *attribute);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createEntityReference (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              const char *name,
                                                              MSXML2Obj_IXMLDOMEntityReference *entityRef);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2getElementsByTagName (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             const char *tagName,
                                                             MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2createNode (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT type,
                                                   const char *name,
                                                   const char *namespaceURI,
                                                   MSXML2Obj_IXMLDOMNode *node);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2nodeFromID (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *idString,
                                                   MSXML2Obj_IXMLDOMNode *node);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2load (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             VARIANT xmlSource,
                                             VBOOL *isSuccessful);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetreadyState (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long *value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetparseError (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMParseError *errorObj);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Geturl (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               char **urlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getasync (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL *isAsync);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Setasync (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL isAsync);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2abort (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2loadXML (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                const char *bstrXML,
                                                VBOOL *isSuccessful);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2save (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             VARIANT destination);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetvalidateOnParse (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VBOOL *isValidating);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2SetvalidateOnParse (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VBOOL isValidating);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetresolveExternals (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VBOOL *isResolving);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2SetresolveExternals (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VBOOL isResolving);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2GetpreserveWhiteSpace (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VBOOL *isPreserving);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2SetpreserveWhiteSpace (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VBOOL isPreserving);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Setonreadystatechange (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VARIANT newValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Setondataavailable (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VARIANT newValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Setontransformnode (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VARIANT newValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getnamespaces (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMSchemaCollection *namespaceCollection);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2Getschemas (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *otherCollection);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2SetByRefschemas (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT otherCollection);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2validate (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMParseError *errorObj);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2setProperty (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *name,
                                                    VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocument2getProperty (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *name,
                                                    VARIANT *value);

HRESULT CVIFUNC MSXML2_NewXMLSchemaCacheIXMLDOMSchemaCollection (const char *server,
                                                                 int supportMultithreading,
                                                                 LCID locale,
                                                                 int reserved,
                                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXMLSchemaCacheIXMLDOMSchemaCollection (const char *fileName,
                                                                  const char *server,
                                                                  int supportMultithreading,
                                                                  LCID locale,
                                                                  int reserved,
                                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXMLSchemaCacheIXMLDOMSchemaCollection (const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewXMLSchemaCache26IXMLDOMSchemaCollection (const char *server,
                                                                   int supportMultithreading,
                                                                   LCID locale,
                                                                   int reserved,
                                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXMLSchemaCache26IXMLDOMSchemaCollection (const char *fileName,
                                                                    const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXMLSchemaCache26IXMLDOMSchemaCollection (const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewXMLSchemaCache30IXMLDOMSchemaCollection (const char *server,
                                                                   int supportMultithreading,
                                                                   LCID locale,
                                                                   int reserved,
                                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXMLSchemaCache30IXMLDOMSchemaCollection (const char *fileName,
                                                                    const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXMLSchemaCache30IXMLDOMSchemaCollection (const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollectionadd (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *namespaceURI,
                                                   VARIANT var);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollectionget (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *namespaceURI,
                                                   MSXML2Obj_IXMLDOMNode *schemaNode);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollectionremove (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      const char *namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollectionGetlength (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long *length);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollectionGetnamespaceURI (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               long index,
                                                               char **length);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollectionaddCollection (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMSchemaCollection otherCollection);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollectionGet_newEnum (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_NewIXMLDOMSchemaCollection2 (const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenIXMLDOMSchemaCollection2 (const char *fileName,
                                                     const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveIXMLDOMSchemaCollection2 (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2add (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *namespaceURI,
                                                    VARIANT var);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2get (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *namespaceURI,
                                                    MSXML2Obj_IXMLDOMNode *schemaNode);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2remove (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2Getlength (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          long *length);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2GetnamespaceURI (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                long index,
                                                                char **length);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2addCollection (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IXMLDOMSchemaCollection otherCollection);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2Get_newEnum (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2validate (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2SetvalidateOnLoad (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  VBOOL validateOnLoad);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2GetvalidateOnLoad (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  VBOOL *validateOnLoad);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2getSchema (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          const char *namespaceURI,
                                                          MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_IXMLDOMSchemaCollection2getDeclaration (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               MSXML2Obj_IXMLDOMNode node,
                                                               MSXML2Obj_ISchemaItem *item);

HRESULT CVIFUNC MSXML2_NewXSLTemplateIXSLTemplate (const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXSLTemplateIXSLTemplate (const char *fileName,
                                                    const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXSLTemplateIXSLTemplate (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewXSLTemplate26IXSLTemplate (const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXSLTemplate26IXSLTemplate (const char *fileName,
                                                      const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXSLTemplate26IXSLTemplate (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewXSLTemplate30IXSLTemplate (const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXSLTemplate30IXSLTemplate (const char *fileName,
                                                      const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXSLTemplate30IXSLTemplate (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewXSLTemplate40IXSLTemplate (const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXSLTemplate40IXSLTemplate (const char *fileName,
                                                      const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXSLTemplate40IXSLTemplate (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IXSLTemplateSetByRefstylesheet (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNode stylesheet);

HRESULT CVIFUNC MSXML2_IXSLTemplateGetstylesheet (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode *stylesheet);

HRESULT CVIFUNC MSXML2_IXSLTemplatecreateProcessor (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXSLProcessor *ppProcessor);

HRESULT CVIFUNC MSXML2_NewDSOControlIDSOControl (const char *server,
                                                 int supportMultithreading,
                                                 LCID locale, int reserved,
                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenDSOControlIDSOControl (const char *fileName,
                                                  const char *server,
                                                  int supportMultithreading,
                                                  LCID locale, int reserved,
                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveDSOControlIDSOControl (const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewDSOControl26IDSOControl (const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenDSOControl26IDSOControl (const char *fileName,
                                                    const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveDSOControl26IDSOControl (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewDSOControl30IDSOControl (const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenDSOControl30IDSOControl (const char *fileName,
                                                    const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveDSOControl30IDSOControl (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewDSOControl40IDSOControl (const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenDSOControl40IDSOControl (const char *fileName,
                                                    const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveDSOControl40IDSOControl (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IDSOControlGetXMLDocument (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMDocument *ppDoc);

HRESULT CVIFUNC MSXML2_IDSOControlSetXMLDocument (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMDocument ppDoc);

HRESULT CVIFUNC MSXML2_IDSOControlGetJavaDSOCompatible (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long *fJavaDSOCompatible);

HRESULT CVIFUNC MSXML2_IDSOControlSetJavaDSOCompatible (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long fJavaDSOCompatible);

HRESULT CVIFUNC MSXML2_IDSOControlGetreadyState (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *state);

HRESULT CVIFUNC MSXML2_NewXMLHTTPIXMLHTTPRequest (const char *server,
                                                  int supportMultithreading,
                                                  LCID locale, int reserved,
                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXMLHTTPIXMLHTTPRequest (const char *fileName,
                                                   const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXMLHTTPIXMLHTTPRequest (const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewXMLHTTP26IXMLHTTPRequest (const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXMLHTTP26IXMLHTTPRequest (const char *fileName,
                                                     const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXMLHTTP26IXMLHTTPRequest (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewXMLHTTP30IXMLHTTPRequest (const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXMLHTTP30IXMLHTTPRequest (const char *fileName,
                                                     const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXMLHTTP30IXMLHTTPRequest (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewXMLHTTP40IXMLHTTPRequest (const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenXMLHTTP40IXMLHTTPRequest (const char *fileName,
                                                     const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveXMLHTTP40IXMLHTTPRequest (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestopen (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            const char *bstrMethod,
                                            const char *bstrUrl,
                                            VARIANT varAsync, VARIANT bstrUser,
                                            VARIANT bstrPassword);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestsetRequestHeader (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *bstrHeader,
                                                        const char *bstrValue);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestgetResponseHeader (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         const char *bstrHeader,
                                                         char **pbstrValue);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestgetAllResponseHeaders (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             char **pbstrHeaders);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestsend (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            VARIANT varBody);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestabort (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestGetstatus (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *plStatus);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestGetstatusText (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **pbstrStatus);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestGetresponseXML (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      CAObjHandle *ppBody);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestGetresponseText (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **pbstrBody);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestGetresponseBody (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VARIANT *pvarBody);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestGetresponseStream (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VARIANT *pvarBody);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestGetreadyState (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long *plState);

HRESULT CVIFUNC MSXML2_IXMLHTTPRequestSetonreadystatechange (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             CAObjHandle newValue);

HRESULT CVIFUNC MSXML2_NewServerXMLHTTPIServerXMLHTTPRequest2 (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenServerXMLHTTPIServerXMLHTTPRequest2 (const char *fileName,
                                                                const char *server,
                                                                int supportMultithreading,
                                                                LCID locale,
                                                                int reserved,
                                                                CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveServerXMLHTTPIServerXMLHTTPRequest2 (const char *server,
                                                                  int supportMultithreading,
                                                                  LCID locale,
                                                                  int reserved,
                                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewServerXMLHTTP30IServerXMLHTTPRequest2 (const char *server,
                                                                 int supportMultithreading,
                                                                 LCID locale,
                                                                 int reserved,
                                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenServerXMLHTTP30IServerXMLHTTPRequest2 (const char *fileName,
                                                                  const char *server,
                                                                  int supportMultithreading,
                                                                  LCID locale,
                                                                  int reserved,
                                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveServerXMLHTTP30IServerXMLHTTPRequest2 (const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewServerXMLHTTP40IServerXMLHTTPRequest2 (const char *server,
                                                                 int supportMultithreading,
                                                                 LCID locale,
                                                                 int reserved,
                                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenServerXMLHTTP40IServerXMLHTTPRequest2 (const char *fileName,
                                                                  const char *server,
                                                                  int supportMultithreading,
                                                                  LCID locale,
                                                                  int reserved,
                                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveServerXMLHTTP40IServerXMLHTTPRequest2 (const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2open (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *bstrMethod,
                                                   const char *bstrUrl,
                                                   VARIANT varAsync,
                                                   VARIANT bstrUser,
                                                   VARIANT bstrPassword);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2setRequestHeader (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               const char *bstrHeader,
                                                               const char *bstrValue);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2getResponseHeader (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                const char *bstrHeader,
                                                                char **pbstrValue);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2getAllResponseHeaders (CAObjHandle objectHandle,
                                                                    ERRORINFO *errorInfo,
                                                                    char **pbstrHeaders);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2send (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT varBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2abort (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2Getstatus (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long *plStatus);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2GetstatusText (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            char **pbstrStatus);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2GetresponseXML (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             CAObjHandle *ppBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2GetresponseText (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              char **pbstrBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2GetresponseBody (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VARIANT *pvarBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2GetresponseStream (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                VARIANT *pvarBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2GetreadyState (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            long *plState);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2Setonreadystatechange (CAObjHandle objectHandle,
                                                                    ERRORINFO *errorInfo,
                                                                    CAObjHandle newValue);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2setTimeouts (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          long resolveTimeout,
                                                          long connectTimeout,
                                                          long sendTimeout,
                                                          long receiveTimeout);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2waitForResponse (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VARIANT timeoutInSeconds,
                                                              VBOOL *isSuccessful);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2getOption (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Type_SERVERXMLHTTP_OPTION option,
                                                        VARIANT *value);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2setOption (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Type_SERVERXMLHTTP_OPTION option,
                                                        VARIANT value);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2setProxy (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Type_SXH_PROXY_SETTING proxySetting,
                                                       VARIANT varProxyServer,
                                                       VARIANT varBypassList);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequest2setProxyCredentials (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  const char *bstrUserName,
                                                                  const char *bstrPassword);

HRESULT CVIFUNC MSXML2_NewSAXXMLReaderIVBSAXXMLReader (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXXMLReaderIVBSAXXMLReader (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXXMLReaderIVBSAXXMLReader (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXXMLReader30IVBSAXXMLReader (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXXMLReader30IVBSAXXMLReader (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXXMLReader30IVBSAXXMLReader (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXXMLReader40IVBSAXXMLReader (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXXMLReader40IVBSAXXMLReader (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXXMLReader40IVBSAXXMLReader (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReadergetFeature (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *strName,
                                                  VBOOL *fValue);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderputFeature (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *strName,
                                                  VBOOL fValue);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReadergetProperty (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *strName,
                                                   VARIANT *varValue);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderputProperty (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *strName,
                                                   VARIANT varValue);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderGetentityResolver (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IVBSAXEntityResolver *oResolver);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderSetByRefentityResolver (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IVBSAXEntityResolver oResolver);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderGetcontentHandler (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IVBSAXContentHandler *oHandler);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderSetByRefcontentHandler (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IVBSAXContentHandler oHandler);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderGetdtdHandler (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IVBSAXDTDHandler *oHandler);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderSetByRefdtdHandler (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IVBSAXDTDHandler oHandler);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderGeterrorHandler (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IVBSAXErrorHandler *oHandler);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderSetByReferrorHandler (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IVBSAXErrorHandler oHandler);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderGetbaseURL (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **strBaseURL);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderSetbaseURL (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *strBaseURL);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderGetsecureBaseURL (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **strSecureBaseURL);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderSetsecureBaseURL (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *strSecureBaseURL);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderparse (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             VARIANT varInput);

HRESULT CVIFUNC MSXML2_IVBSAXXMLReaderparseURL (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                const char *strURL);

HRESULT CVIFUNC MSXML2_NewSAXXMLReaderISAXXMLReader (const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXXMLReaderISAXXMLReader (const char *fileName,
                                                      const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXXMLReaderISAXXMLReader (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXXMLReader30ISAXXMLReader (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXXMLReader30ISAXXMLReader (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXXMLReader30ISAXXMLReader (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXXMLReader40ISAXXMLReader (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXXMLReader40ISAXXMLReader (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXXMLReader40ISAXXMLReader (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ISAXXMLReadergetFeature (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                unsigned short *pwchName,
                                                VBOOL *pvfValue);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderputFeature (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                unsigned short *pwchName,
                                                VBOOL vfValue);

HRESULT CVIFUNC MSXML2_ISAXXMLReadergetProperty (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 unsigned short *pwchName,
                                                 VARIANT *pvarValue);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderputProperty (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 unsigned short *pwchName,
                                                 VARIANT varValue);

HRESULT CVIFUNC MSXML2_ISAXXMLReadergetEntityResolver (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISAXEntityResolver *ppResolver);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderputEntityResolver (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISAXEntityResolver pResolver);

HRESULT CVIFUNC MSXML2_ISAXXMLReadergetContentHandler (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISAXContentHandler *ppHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderputContentHandler (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISAXContentHandler pHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLReadergetDTDHandler (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_ISAXDTDHandler *ppHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderputDTDHandler (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_ISAXDTDHandler pHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLReadergetErrorHandler (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_ISAXErrorHandler *ppHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderputErrorHandler (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_ISAXErrorHandler pHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderputBaseURL (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                unsigned short *pwchBaseUrl);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderputSecureBaseURL (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      unsigned short *pwchSecureBaseUrl);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderparse (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           VARIANT varInput);

HRESULT CVIFUNC MSXML2_ISAXXMLReaderparseURL (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              unsigned short *pwchUrl);

HRESULT CVIFUNC MSXML2_NewSAXXMLReaderIMXReaderControl (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXXMLReaderIMXReaderControl (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXXMLReaderIMXReaderControl (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXXMLReader30IMXReaderControl (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXXMLReader30IMXReaderControl (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXXMLReader30IMXReaderControl (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IMXReaderControlabort (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IMXReaderControlresume (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IMXReaderControlsuspend (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterIMXWriter (const char *server,
                                                int supportMultithreading,
                                                LCID locale, int reserved,
                                                CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterIMXWriter (const char *fileName,
                                                 const char *server,
                                                 int supportMultithreading,
                                                 LCID locale, int reserved,
                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterIMXWriter (const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30IMXWriter (const char *server,
                                                  int supportMultithreading,
                                                  LCID locale, int reserved,
                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30IMXWriter (const char *fileName,
                                                   const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30IMXWriter (const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40IMXWriter (const char *server,
                                                  int supportMultithreading,
                                                  LCID locale, int reserved,
                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40IMXWriter (const char *fileName,
                                                   const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40IMXWriter (const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterIMXWriter (const char *server,
                                                 int supportMultithreading,
                                                 LCID locale, int reserved,
                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterIMXWriter (const char *fileName,
                                                  const char *server,
                                                  int supportMultithreading,
                                                  LCID locale, int reserved,
                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterIMXWriter (const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30IMXWriter (const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30IMXWriter (const char *fileName,
                                                    const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30IMXWriter (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40IMXWriter (const char *server,
                                                   int supportMultithreading,
                                                   LCID locale, int reserved,
                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40IMXWriter (const char *fileName,
                                                    const char *server,
                                                    int supportMultithreading,
                                                    LCID locale, int reserved,
                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40IMXWriter (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IMXWriterSetoutput (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           VARIANT varDestination);

HRESULT CVIFUNC MSXML2_IMXWriterGetoutput (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           VARIANT *varDestination);

HRESULT CVIFUNC MSXML2_IMXWriterSetencoding (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             const char *strEncoding);

HRESULT CVIFUNC MSXML2_IMXWriterGetencoding (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             char **strEncoding);

HRESULT CVIFUNC MSXML2_IMXWriterSetbyteOrderMark (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VBOOL fWriteByteOrderMark);

HRESULT CVIFUNC MSXML2_IMXWriterGetbyteOrderMark (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VBOOL *fWriteByteOrderMark);

HRESULT CVIFUNC MSXML2_IMXWriterSetindent (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           VBOOL fIndentMode);

HRESULT CVIFUNC MSXML2_IMXWriterGetindent (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           VBOOL *fIndentMode);

HRESULT CVIFUNC MSXML2_IMXWriterSetstandalone (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VBOOL fValue);

HRESULT CVIFUNC MSXML2_IMXWriterGetstandalone (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VBOOL *fValue);

HRESULT CVIFUNC MSXML2_IMXWriterSetomitXMLDeclaration (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VBOOL fValue);

HRESULT CVIFUNC MSXML2_IMXWriterGetomitXMLDeclaration (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VBOOL *fValue);

HRESULT CVIFUNC MSXML2_IMXWriterSetversion (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            const char *strVersion);

HRESULT CVIFUNC MSXML2_IMXWriterGetversion (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            char **strVersion);

HRESULT CVIFUNC MSXML2_IMXWriterSetdisableOutputEscaping (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VBOOL fValue);

HRESULT CVIFUNC MSXML2_IMXWriterGetdisableOutputEscaping (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VBOOL *fValue);

HRESULT CVIFUNC MSXML2_IMXWriterflush (CAObjHandle objectHandle,
                                       ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterISAXContentHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterISAXContentHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterISAXContentHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30ISAXContentHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30ISAXContentHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30ISAXContentHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40ISAXContentHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40ISAXContentHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40ISAXContentHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterISAXContentHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterISAXContentHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterISAXContentHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30ISAXContentHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30ISAXContentHandler (const char *fileName,
                                                             const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30ISAXContentHandler (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40ISAXContentHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40ISAXContentHandler (const char *fileName,
                                                             const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40ISAXContentHandler (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerputDocumentLocator (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_ISAXLocator pLocator);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerstartDocument (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerendDocument (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerstartPrefixMapping (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             unsigned short *pwchPrefix,
                                                             long cchPrefix,
                                                             unsigned short *pwchUri,
                                                             long cchUri);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerendPrefixMapping (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           unsigned short *pwchPrefix,
                                                           long cchPrefix);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerstartElement (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       unsigned short *pwchNamespaceUri,
                                                       long cchNamespaceUri,
                                                       unsigned short *pwchLocalName,
                                                       long cchLocalName,
                                                       unsigned short *pwchQName,
                                                       long cchQName,
                                                       MSXML2Obj_ISAXAttributes pAttributes);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerendElement (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     unsigned short *pwchNamespaceUri,
                                                     long cchNamespaceUri,
                                                     unsigned short *pwchLocalName,
                                                     long cchLocalName,
                                                     unsigned short *pwchQName,
                                                     long cchQName);

HRESULT CVIFUNC MSXML2_ISAXContentHandlercharacters (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     unsigned short *pwchChars,
                                                     long cchChars);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerignorableWhitespace (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              unsigned short *pwchChars,
                                                              long cchChars);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerprocessingInstruction (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                unsigned short *pwchTarget,
                                                                long cchTarget,
                                                                unsigned short *pwchData,
                                                                long cchData);

HRESULT CVIFUNC MSXML2_ISAXContentHandlerskippedEntity (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        unsigned short *pwchName,
                                                        long cchName);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterISAXErrorHandler (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterISAXErrorHandler (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterISAXErrorHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30ISAXErrorHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30ISAXErrorHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30ISAXErrorHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40ISAXErrorHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40ISAXErrorHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40ISAXErrorHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterISAXErrorHandler (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterISAXErrorHandler (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterISAXErrorHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30ISAXErrorHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30ISAXErrorHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30ISAXErrorHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40ISAXErrorHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40ISAXErrorHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40ISAXErrorHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ISAXErrorHandlererror (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              MSXML2Obj_ISAXLocator pLocator,
                                              unsigned short *pwchErrorMessage,
                                              long hrErrorCode);

HRESULT CVIFUNC MSXML2_ISAXErrorHandlerfatalError (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_ISAXLocator pLocator,
                                                   unsigned short *pwchErrorMessage,
                                                   long hrErrorCode);

HRESULT CVIFUNC MSXML2_ISAXErrorHandlerignorableWarning (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_ISAXLocator pLocator,
                                                         unsigned short *pwchErrorMessage,
                                                         long hrErrorCode);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterISAXDTDHandler (const char *server,
                                                     int supportMultithreading,
                                                     LCID locale, int reserved,
                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterISAXDTDHandler (const char *fileName,
                                                      const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterISAXDTDHandler (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30ISAXDTDHandler (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30ISAXDTDHandler (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30ISAXDTDHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40ISAXDTDHandler (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40ISAXDTDHandler (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40ISAXDTDHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterISAXDTDHandler (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterISAXDTDHandler (const char *fileName,
                                                       const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterISAXDTDHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30ISAXDTDHandler (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30ISAXDTDHandler (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30ISAXDTDHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40ISAXDTDHandler (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40ISAXDTDHandler (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40ISAXDTDHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ISAXDTDHandlernotationDecl (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   unsigned short *pwchName,
                                                   long cchName,
                                                   unsigned short *pwchPublicId,
                                                   long cchPublicId,
                                                   unsigned short *pwchSystemId,
                                                   long cchSystemId);

HRESULT CVIFUNC MSXML2_ISAXDTDHandlerunparsedEntityDecl (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         unsigned short *pwchName,
                                                         long cchName,
                                                         unsigned short *pwchPublicId,
                                                         long cchPublicId,
                                                         unsigned short *pwchSystemId,
                                                         long cchSystemId,
                                                         unsigned short *pwchNotationName,
                                                         long cchNotationName);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterISAXLexicalHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterISAXLexicalHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterISAXLexicalHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30ISAXLexicalHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30ISAXLexicalHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30ISAXLexicalHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40ISAXLexicalHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40ISAXLexicalHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40ISAXLexicalHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterISAXLexicalHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterISAXLexicalHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterISAXLexicalHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30ISAXLexicalHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30ISAXLexicalHandler (const char *fileName,
                                                             const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30ISAXLexicalHandler (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40ISAXLexicalHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40ISAXLexicalHandler (const char *fileName,
                                                             const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40ISAXLexicalHandler (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ISAXLexicalHandlerstartDTD (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   unsigned short *pwchName,
                                                   long cchName,
                                                   unsigned short *pwchPublicId,
                                                   long cchPublicId,
                                                   unsigned short *pwchSystemId,
                                                   long cchSystemId);

HRESULT CVIFUNC MSXML2_ISAXLexicalHandlerendDTD (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_ISAXLexicalHandlerstartEntity (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      unsigned short *pwchName,
                                                      long cchName);

HRESULT CVIFUNC MSXML2_ISAXLexicalHandlerendEntity (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    unsigned short *pwchName,
                                                    long cchName);

HRESULT CVIFUNC MSXML2_ISAXLexicalHandlerstartCDATA (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_ISAXLexicalHandlerendCDATA (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_ISAXLexicalHandlercomment (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  unsigned short *pwchChars,
                                                  long cchChars);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterISAXDeclHandler (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterISAXDeclHandler (const char *fileName,
                                                       const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterISAXDeclHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30ISAXDeclHandler (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30ISAXDeclHandler (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30ISAXDeclHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40ISAXDeclHandler (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40ISAXDeclHandler (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40ISAXDeclHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterISAXDeclHandler (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterISAXDeclHandler (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterISAXDeclHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30ISAXDeclHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30ISAXDeclHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30ISAXDeclHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40ISAXDeclHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40ISAXDeclHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40ISAXDeclHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ISAXDeclHandlerelementDecl (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   unsigned short *pwchName,
                                                   long cchName,
                                                   unsigned short *pwchModel,
                                                   long cchModel);

HRESULT CVIFUNC MSXML2_ISAXDeclHandlerattributeDecl (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     unsigned short *pwchElementName,
                                                     long cchElementName,
                                                     unsigned short *pwchAttributeName,
                                                     long cchAttributeName,
                                                     unsigned short *pwchType,
                                                     long cchType,
                                                     unsigned short *pwchValueDefault,
                                                     long cchValueDefault,
                                                     unsigned short *pwchValue,
                                                     long cchValue);

HRESULT CVIFUNC MSXML2_ISAXDeclHandlerinternalEntityDecl (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          unsigned short *pwchName,
                                                          long cchName,
                                                          unsigned short *pwchValue,
                                                          long cchValue);

HRESULT CVIFUNC MSXML2_ISAXDeclHandlerexternalEntityDecl (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          unsigned short *pwchName,
                                                          long cchName,
                                                          unsigned short *pwchPublicId,
                                                          long cchPublicId,
                                                          unsigned short *pwchSystemId,
                                                          long cchSystemId);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterIVBSAXContentHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterIVBSAXContentHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterIVBSAXContentHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30IVBSAXContentHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30IVBSAXContentHandler (const char *fileName,
                                                              const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30IVBSAXContentHandler (const char *server,
                                                                int supportMultithreading,
                                                                LCID locale,
                                                                int reserved,
                                                                CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40IVBSAXContentHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40IVBSAXContentHandler (const char *fileName,
                                                              const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40IVBSAXContentHandler (const char *server,
                                                                int supportMultithreading,
                                                                LCID locale,
                                                                int reserved,
                                                                CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterIVBSAXContentHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterIVBSAXContentHandler (const char *fileName,
                                                             const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterIVBSAXContentHandler (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30IVBSAXContentHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30IVBSAXContentHandler (const char *fileName,
                                                               const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30IVBSAXContentHandler (const char *server,
                                                                 int supportMultithreading,
                                                                 LCID locale,
                                                                 int reserved,
                                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40IVBSAXContentHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40IVBSAXContentHandler (const char *fileName,
                                                               const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40IVBSAXContentHandler (const char *server,
                                                                 int supportMultithreading,
                                                                 LCID locale,
                                                                 int reserved,
                                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerSetByRefdocumentLocator (CAObjHandle objectHandle,
                                                                    ERRORINFO *errorInfo,
                                                                    MSXML2Obj_IVBSAXLocator newValue);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerstartDocument (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerendDocument (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerstartPrefixMapping (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               char **strPrefix,
                                                               char **strURI);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerendPrefixMapping (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             char **strPrefix);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerstartElement (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         char **strNamespaceURI,
                                                         char **strLocalName,
                                                         char **strQName,
                                                         MSXML2Obj_IVBSAXAttributes oAttributes);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerendElement (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **strNamespaceURI,
                                                       char **strLocalName,
                                                       char **strQName);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlercharacters (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **strChars);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerignorableWhitespace (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                char **strChars);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerprocessingInstruction (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  char **strTarget,
                                                                  char **strData);

HRESULT CVIFUNC MSXML2_IVBSAXContentHandlerskippedEntity (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **strName);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterIVBSAXDeclHandler (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterIVBSAXDeclHandler (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterIVBSAXDeclHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30IVBSAXDeclHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30IVBSAXDeclHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30IVBSAXDeclHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40IVBSAXDeclHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40IVBSAXDeclHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40IVBSAXDeclHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterIVBSAXDeclHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterIVBSAXDeclHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterIVBSAXDeclHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30IVBSAXDeclHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30IVBSAXDeclHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30IVBSAXDeclHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40IVBSAXDeclHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40IVBSAXDeclHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40IVBSAXDeclHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IVBSAXDeclHandlerelementDecl (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **strName,
                                                     char **strModel);

HRESULT CVIFUNC MSXML2_IVBSAXDeclHandlerattributeDecl (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **strElementName,
                                                       char **strAttributeName,
                                                       char **strType,
                                                       char **strValueDefault,
                                                       char **strValue);

HRESULT CVIFUNC MSXML2_IVBSAXDeclHandlerinternalEntityDecl (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            char **strName,
                                                            char **strValue);

HRESULT CVIFUNC MSXML2_IVBSAXDeclHandlerexternalEntityDecl (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            char **strName,
                                                            char **strPublicId,
                                                            char **strSystemId);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterIVBSAXDTDHandler (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterIVBSAXDTDHandler (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterIVBSAXDTDHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30IVBSAXDTDHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30IVBSAXDTDHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30IVBSAXDTDHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40IVBSAXDTDHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40IVBSAXDTDHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40IVBSAXDTDHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterIVBSAXDTDHandler (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterIVBSAXDTDHandler (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterIVBSAXDTDHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30IVBSAXDTDHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30IVBSAXDTDHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30IVBSAXDTDHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40IVBSAXDTDHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40IVBSAXDTDHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40IVBSAXDTDHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IVBSAXDTDHandlernotationDecl (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **strName,
                                                     char **strPublicId,
                                                     char **strSystemId);

HRESULT CVIFUNC MSXML2_IVBSAXDTDHandlerunparsedEntityDecl (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **strName,
                                                           char **strPublicId,
                                                           char **strSystemId,
                                                           char **strNotationName);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterIVBSAXErrorHandler (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterIVBSAXErrorHandler (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterIVBSAXErrorHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30IVBSAXErrorHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30IVBSAXErrorHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30IVBSAXErrorHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40IVBSAXErrorHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40IVBSAXErrorHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40IVBSAXErrorHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterIVBSAXErrorHandler (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterIVBSAXErrorHandler (const char *fileName,
                                                           const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterIVBSAXErrorHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30IVBSAXErrorHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30IVBSAXErrorHandler (const char *fileName,
                                                             const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30IVBSAXErrorHandler (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40IVBSAXErrorHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40IVBSAXErrorHandler (const char *fileName,
                                                             const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40IVBSAXErrorHandler (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IVBSAXErrorHandlererror (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IVBSAXLocator oLocator,
                                                char **strErrorMessage,
                                                long nErrorCode);

HRESULT CVIFUNC MSXML2_IVBSAXErrorHandlerfatalError (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IVBSAXLocator oLocator,
                                                     char **strErrorMessage,
                                                     long nErrorCode);

HRESULT CVIFUNC MSXML2_IVBSAXErrorHandlerignorableWarning (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IVBSAXLocator oLocator,
                                                           char **strErrorMessage,
                                                           long nErrorCode);

HRESULT CVIFUNC MSXML2_NewMXXMLWriterIVBSAXLexicalHandler (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriterIVBSAXLexicalHandler (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriterIVBSAXLexicalHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter30IVBSAXLexicalHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter30IVBSAXLexicalHandler (const char *fileName,
                                                              const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter30IVBSAXLexicalHandler (const char *server,
                                                                int supportMultithreading,
                                                                LCID locale,
                                                                int reserved,
                                                                CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXXMLWriter40IVBSAXLexicalHandler (const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXXMLWriter40IVBSAXLexicalHandler (const char *fileName,
                                                              const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXXMLWriter40IVBSAXLexicalHandler (const char *server,
                                                                int supportMultithreading,
                                                                LCID locale,
                                                                int reserved,
                                                                CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriterIVBSAXLexicalHandler (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriterIVBSAXLexicalHandler (const char *fileName,
                                                             const char *server,
                                                             int supportMultithreading,
                                                             LCID locale,
                                                             int reserved,
                                                             CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriterIVBSAXLexicalHandler (const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter30IVBSAXLexicalHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter30IVBSAXLexicalHandler (const char *fileName,
                                                               const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter30IVBSAXLexicalHandler (const char *server,
                                                                 int supportMultithreading,
                                                                 LCID locale,
                                                                 int reserved,
                                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXHTMLWriter40IVBSAXLexicalHandler (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXHTMLWriter40IVBSAXLexicalHandler (const char *fileName,
                                                               const char *server,
                                                               int supportMultithreading,
                                                               LCID locale,
                                                               int reserved,
                                                               CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXHTMLWriter40IVBSAXLexicalHandler (const char *server,
                                                                 int supportMultithreading,
                                                                 LCID locale,
                                                                 int reserved,
                                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IVBSAXLexicalHandlerstartDTD (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **strName,
                                                     char **strPublicId,
                                                     char **strSystemId);

HRESULT CVIFUNC MSXML2_IVBSAXLexicalHandlerendDTD (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IVBSAXLexicalHandlerstartEntity (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **strName);

HRESULT CVIFUNC MSXML2_IVBSAXLexicalHandlerendEntity (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **strName);

HRESULT CVIFUNC MSXML2_IVBSAXLexicalHandlerstartCDATA (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IVBSAXLexicalHandlerendCDATA (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IVBSAXLexicalHandlercomment (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **strChars);

HRESULT CVIFUNC MSXML2_NewSAXAttributesIMXAttributes (const char *server,
                                                      int supportMultithreading,
                                                      LCID locale, int reserved,
                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributesIMXAttributes (const char *fileName,
                                                       const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributesIMXAttributes (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXAttributes30IMXAttributes (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributes30IMXAttributes (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributes30IMXAttributes (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXAttributes40IMXAttributes (const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributes40IMXAttributes (const char *fileName,
                                                         const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributes40IMXAttributes (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IMXAttributesaddAttribute (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *strURI,
                                                  const char *strLocalName,
                                                  const char *strQName,
                                                  const char *strType,
                                                  const char *strValue);

HRESULT CVIFUNC MSXML2_IMXAttributesaddAttributeFromIndex (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VARIANT varAtts,
                                                           long nIndex);

HRESULT CVIFUNC MSXML2_IMXAttributesclear (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IMXAttributesremoveAttribute (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long nIndex);

HRESULT CVIFUNC MSXML2_IMXAttributessetAttribute (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long nIndex,
                                                  const char *strURI,
                                                  const char *strLocalName,
                                                  const char *strQName,
                                                  const char *strType,
                                                  const char *strValue);

HRESULT CVIFUNC MSXML2_IMXAttributessetAttributes (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT varAtts);

HRESULT CVIFUNC MSXML2_IMXAttributessetLocalName (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long nIndex,
                                                  const char *strLocalName);

HRESULT CVIFUNC MSXML2_IMXAttributessetQName (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long nIndex,
                                              const char *strQName);

HRESULT CVIFUNC MSXML2_IMXAttributessetType (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long nIndex,
                                             const char *strType);

HRESULT CVIFUNC MSXML2_IMXAttributessetURI (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long nIndex,
                                            const char *strURI);

HRESULT CVIFUNC MSXML2_IMXAttributessetValue (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long nIndex,
                                              const char *strValue);

HRESULT CVIFUNC MSXML2_NewSAXAttributesIVBSAXAttributes (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributesIVBSAXAttributes (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributesIVBSAXAttributes (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXAttributes30IVBSAXAttributes (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributes30IVBSAXAttributes (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributes30IVBSAXAttributes (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXAttributes40IVBSAXAttributes (const char *server,
                                                           int supportMultithreading,
                                                           LCID locale,
                                                           int reserved,
                                                           CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributes40IVBSAXAttributes (const char *fileName,
                                                            const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributes40IVBSAXAttributes (const char *server,
                                                              int supportMultithreading,
                                                              LCID locale,
                                                              int reserved,
                                                              CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesGetlength (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long *nLength);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetURI (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long nIndex,
                                               char **strURI);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetLocalName (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long nIndex,
                                                     char **strLocalName);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetQName (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long nIndex, char **strQName);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetIndexFromName (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         const char *strURI,
                                                         const char *strLocalName,
                                                         long *nIndex);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetIndexFromQName (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          const char *strQName,
                                                          long *nIndex);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetType (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                long nIndex, char **strType);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetTypeFromName (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *strURI,
                                                        const char *strLocalName,
                                                        char **strType);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetTypeFromQName (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         const char *strQName,
                                                         char **strType);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetValue (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long nIndex, char **strValue);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetValueFromName (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         const char *strURI,
                                                         const char *strLocalName,
                                                         char **strValue);

HRESULT CVIFUNC MSXML2_IVBSAXAttributesgetValueFromQName (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          const char *strQName,
                                                          char **strValue);

HRESULT CVIFUNC MSXML2_NewSAXAttributesISAXAttributes (const char *server,
                                                       int supportMultithreading,
                                                       LCID locale, int reserved,
                                                       CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributesISAXAttributes (const char *fileName,
                                                        const char *server,
                                                        int supportMultithreading,
                                                        LCID locale,
                                                        int reserved,
                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributesISAXAttributes (const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXAttributes30ISAXAttributes (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributes30ISAXAttributes (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributes30ISAXAttributes (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewSAXAttributes40ISAXAttributes (const char *server,
                                                         int supportMultithreading,
                                                         LCID locale,
                                                         int reserved,
                                                         CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenSAXAttributes40ISAXAttributes (const char *fileName,
                                                          const char *server,
                                                          int supportMultithreading,
                                                          LCID locale,
                                                          int reserved,
                                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveSAXAttributes40ISAXAttributes (const char *server,
                                                            int supportMultithreading,
                                                            LCID locale,
                                                            int reserved,
                                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ISAXAttributesgetLength (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                long *pnLength);

HRESULT CVIFUNC MSXML2_ISAXAttributesgetIndexFromName (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       unsigned short *pwchUri,
                                                       long cchUri,
                                                       unsigned short *pwchLocalName,
                                                       long cchLocalName,
                                                       long *pnIndex);

HRESULT CVIFUNC MSXML2_ISAXAttributesgetIndexFromQName (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        unsigned short *pwchQName,
                                                        long cchQName,
                                                        long *pnIndex);

HRESULT CVIFUNC MSXML2_NewMXNamespaceManagerIVBMXNamespaceManager (const char *server,
                                                                   int supportMultithreading,
                                                                   LCID locale,
                                                                   int reserved,
                                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXNamespaceManagerIVBMXNamespaceManager (const char *fileName,
                                                                    const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXNamespaceManagerIVBMXNamespaceManager (const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXNamespaceManager40IVBMXNamespaceManager (const char *server,
                                                                     int supportMultithreading,
                                                                     LCID locale,
                                                                     int reserved,
                                                                     CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXNamespaceManager40IVBMXNamespaceManager (const char *fileName,
                                                                      const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXNamespaceManager40IVBMXNamespaceManager (const char *server,
                                                                        int supportMultithreading,
                                                                        LCID locale,
                                                                        int reserved,
                                                                        CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagerSetallowOverride (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VBOOL fOverride);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagerGetallowOverride (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VBOOL *fOverride);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagerreset (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagerpushContext (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagerpushNodeContext (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNode contextNode,
                                                             VBOOL fDeep);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagerpopContext (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagerdeclarePrefix (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           const char *prefix,
                                                           const char *namespaceURI);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagergetDeclaredPrefixes (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Obj_IMXNamespacePrefixes *prefixes);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagergetPrefixes (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         const char *namespaceURI,
                                                         MSXML2Obj_IMXNamespacePrefixes *prefixes);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagergetURI (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *prefix,
                                                    VARIANT *uri);

HRESULT CVIFUNC MSXML2_IVBMXNamespaceManagergetURIFromNode (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            const char *strPrefix,
                                                            MSXML2Obj_IXMLDOMNode contextNode,
                                                            VARIANT *uri);

HRESULT CVIFUNC MSXML2_NewMXNamespaceManagerIMXNamespaceManager (const char *server,
                                                                 int supportMultithreading,
                                                                 LCID locale,
                                                                 int reserved,
                                                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXNamespaceManagerIMXNamespaceManager (const char *fileName,
                                                                  const char *server,
                                                                  int supportMultithreading,
                                                                  LCID locale,
                                                                  int reserved,
                                                                  CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXNamespaceManagerIMXNamespaceManager (const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_NewMXNamespaceManager40IMXNamespaceManager (const char *server,
                                                                   int supportMultithreading,
                                                                   LCID locale,
                                                                   int reserved,
                                                                   CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenMXNamespaceManager40IMXNamespaceManager (const char *fileName,
                                                                    const char *server,
                                                                    int supportMultithreading,
                                                                    LCID locale,
                                                                    int reserved,
                                                                    CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveMXNamespaceManager40IMXNamespaceManager (const char *server,
                                                                      int supportMultithreading,
                                                                      LCID locale,
                                                                      int reserved,
                                                                      CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagerputAllowOverride (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VBOOL fOverride);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagergetAllowOverride (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VBOOL *fOverride);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagerreset (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagerpushContext (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagerpushNodeContext (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode contextNode,
                                                           VBOOL fDeep);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagerpopContext (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagerdeclarePrefix (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         unsigned short *prefix,
                                                         unsigned short *namespaceURI);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagergetDeclaredPrefix (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             long nIndex,
                                                             unsigned short *pwchPrefix,
                                                             long *pcchPrefix);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagergetPrefix (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     unsigned short *pwszNamespaceURI,
                                                     long nIndex,
                                                     unsigned short *pwchPrefix,
                                                     long *pcchPrefix);

HRESULT CVIFUNC MSXML2_IMXNamespaceManagergetURI (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  unsigned short *pwchPrefix,
                                                  MSXML2Obj_IXMLDOMNode pContextNode,
                                                  unsigned short *pwchUri,
                                                  long *pcchUri);

HRESULT CVIFUNC MSXML2_NewIXMLDocument2 (const char *server,
                                         int supportMultithreading, LCID locale,
                                         int reserved, CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_OpenIXMLDocument2 (const char *fileName,
                                          const char *server,
                                          int supportMultithreading, LCID locale,
                                          int reserved,
                                          CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_ActiveIXMLDocument2 (const char *server,
                                            int supportMultithreading,
                                            LCID locale, int reserved,
                                            CAObjHandle *objectHandle);

HRESULT CVIFUNC MSXML2_IXMLDocument2Getroot (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             MSXML2Obj_IXMLElement2 *p);

HRESULT CVIFUNC MSXML2_IXMLDocument2GetfileSize (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2GetfileModifiedDate (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2GetfileUpdatedDate (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2Geturl (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2Seturl (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, const char *p);

HRESULT CVIFUNC MSXML2_IXMLDocument2GetmimeType (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2GetreadyState (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pl);

HRESULT CVIFUNC MSXML2_IXMLDocument2Getcharset (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2Setcharset (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                const char *p);

HRESULT CVIFUNC MSXML2_IXMLDocument2Getversion (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2Getdoctype (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2GetdtdURL (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocument2createElement (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT vType, VARIANT var1,
                                                   MSXML2Obj_IXMLElement2 *ppElem);

HRESULT CVIFUNC MSXML2_IXMLDocument2Getasync (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, VBOOL *pf);

HRESULT CVIFUNC MSXML2_IXMLDocument2Setasync (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, VBOOL pf);

HRESULT CVIFUNC MSXML2_IXMLDOMImplementationhasFeature (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *feature,
                                                        const char *version,
                                                        VBOOL *hasFeature);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetnodeName (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetnodeValue (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeSetnodeValue (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetnodeType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetparentNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetchildNodes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetfirstChild (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetlastChild (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetpreviousSibling (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetnextSibling (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetattributes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeinsertBefore (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode newChild,
                                                VARIANT refChild,
                                                MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNodereplaceChild (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode newChild,
                                                MSXML2Obj_IXMLDOMNode oldChild,
                                                MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNoderemoveChild (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLDOMNode childNode,
                                               MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeappendChild (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLDOMNode newChild,
                                               MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNodehasChildNodes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetownerDocument (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMNodecloneNode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, VBOOL deep,
                                             MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetnodeTypeString (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGettext (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeSettext (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetspecified (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetdefinition (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetnodeTypedValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeSetnodeTypedValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetdataType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeSetdataType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetxml (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMNodetransformNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode stylesheet,
                                                 char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeselectNodes (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *queryString,
                                               MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeselectSingleNode (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *queryString,
                                                    MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetparsed (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetnamespaceURI (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetprefix (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeGetbaseName (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMNodetransformNodeToObject (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode stylesheet,
                                                         VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeListGetitem (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long index,
                                               MSXML2Obj_IXMLDOMNode *listItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeListGetlength (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *listLength);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeListnextNode (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode *nextItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeListreset (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLDOMNodeListGet_newEnum (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapgetNamedItem (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *name,
                                                        MSXML2Obj_IXMLDOMNode *namedItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapsetNamedItem (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode newItem,
                                                        MSXML2Obj_IXMLDOMNode *nameItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapremoveNamedItem (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           const char *name,
                                                           MSXML2Obj_IXMLDOMNode *namedItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapGetitem (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long index,
                                                   MSXML2Obj_IXMLDOMNode *listItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapGetlength (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long *listLength);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapgetQualifiedItem (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            const char *baseName,
                                                            const char *namespaceURI,
                                                            MSXML2Obj_IXMLDOMNode *qualifiedItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapremoveQualifiedItem (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               const char *baseName,
                                                               const char *namespaceURI,
                                                               MSXML2Obj_IXMLDOMNode *qualifiedItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapnextNode (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *nextItem);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapreset (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLDOMNamedNodeMapGet_newEnum (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetnodeName (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetnodeValue (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetnodeValue (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetnodeType (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetparentNode (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetchildNodes (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetfirstChild (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetlastChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetpreviousSibling (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetnextSibling (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetattributes (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentinsertBefore (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode newChild,
                                                    VARIANT refChild,
                                                    MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentreplaceChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode newChild,
                                                    MSXML2Obj_IXMLDOMNode oldChild,
                                                    MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentremoveChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode childNode,
                                                   MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentappendChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode newChild,
                                                   MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumenthasChildNodes (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetownerDocument (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcloneNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL deep,
                                                 MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetnodeTypeString (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGettext (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSettext (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetspecified (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetdefinition (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetnodeTypedValue (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetnodeTypedValue (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetdataType (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetdataType (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetxml (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumenttransformNode (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode stylesheet,
                                                     char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentselectNodes (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *queryString,
                                                   MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentselectSingleNode (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *queryString,
                                                        MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetparsed (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetnamespaceURI (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetprefix (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetbaseName (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumenttransformNodeToObject (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNode stylesheet,
                                                             VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetdoctype (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMDocumentType *documentType);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetimplementation (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMImplementation *impl);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetdocumentElement (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMElement *DOMElement);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetByRefdocumentElement (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               MSXML2Obj_IXMLDOMElement DOMElement);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateElement (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     const char *tagName,
                                                     MSXML2Obj_IXMLDOMElement *element);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateDocumentFragment (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IXMLDOMDocumentFragment *docFrag);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateTextNode (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      const char *data,
                                                      MSXML2Obj_IXMLDOMText *text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateComment (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     const char *data,
                                                     MSXML2Obj_IXMLDOMComment *comment);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateCDATASection (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          const char *data,
                                                          MSXML2Obj_IXMLDOMCDATASection *cdata);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateProcessingInstruction (CAObjHandle objectHandle,
                                                                   ERRORINFO *errorInfo,
                                                                   const char *target,
                                                                   const char *data,
                                                                   MSXML2Obj_IXMLDOMProcessingInstruction *pi);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateAttribute (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *name,
                                                       MSXML2Obj_IXMLDOMAttribute *attribute);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateEntityReference (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             const char *name,
                                                             MSXML2Obj_IXMLDOMEntityReference *entityRef);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentgetElementsByTagName (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            const char *tagName,
                                                            MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentcreateNode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT type, const char *name,
                                                  const char *namespaceURI,
                                                  MSXML2Obj_IXMLDOMNode *node);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentnodeFromID (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *idString,
                                                  MSXML2Obj_IXMLDOMNode *node);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentload (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            VARIANT xmlSource,
                                            VBOOL *isSuccessful);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetreadyState (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long *value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetparseError (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMParseError *errorObj);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGeturl (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              char **urlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetasync (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VBOOL *isAsync);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetasync (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VBOOL isAsync);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentabort (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentloadXML (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *bstrXML,
                                               VBOOL *isSuccessful);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentsave (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            VARIANT destination);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetvalidateOnParse (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VBOOL *isValidating);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetvalidateOnParse (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VBOOL isValidating);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetresolveExternals (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VBOOL *isResolving);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetresolveExternals (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VBOOL isResolving);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentGetpreserveWhiteSpace (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VBOOL *isPreserving);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetpreserveWhiteSpace (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VBOOL isPreserving);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetonreadystatechange (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VARIANT newValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetondataavailable (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VARIANT newValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentSetontransformnode (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VARIANT newValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetnodeName (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetnodeValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeSetnodeValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetnodeType (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetparentNode (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetchildNodes (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetfirstChild (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetlastChild (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetpreviousSibling (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetnextSibling (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetattributes (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeinsertBefore (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode newChild,
                                                        VARIANT refChild,
                                                        MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypereplaceChild (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode newChild,
                                                        MSXML2Obj_IXMLDOMNode oldChild,
                                                        MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTyperemoveChild (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNode childNode,
                                                       MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeappendChild (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNode newChild,
                                                       MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypehasChildNodes (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetownerDocument (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypecloneNode (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL deep,
                                                     MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetnodeTypeString (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGettext (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeSettext (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetspecified (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetdefinition (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetnodeTypedValue (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeSetnodeTypedValue (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetdataType (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeSetdataType (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetxml (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypetransformNode (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode stylesheet,
                                                         char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeselectNodes (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *queryString,
                                                       MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeselectSingleNode (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            const char *queryString,
                                                            MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetparsed (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetnamespaceURI (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetprefix (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetbaseName (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypetransformNodeToObject (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Obj_IXMLDOMNode stylesheet,
                                                                 VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetname (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **rootName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetentities (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNamedNodeMap *entityMap);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentTypeGetnotations (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNamedNodeMap *notationMap);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetnodeName (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetnodeValue (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMElementSetnodeValue (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetnodeType (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetparentNode (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetchildNodes (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetfirstChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetlastChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetpreviousSibling (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetnextSibling (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetattributes (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMElementinsertBefore (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode newChild,
                                                   VARIANT refChild,
                                                   MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMElementreplaceChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode newChild,
                                                   MSXML2Obj_IXMLDOMNode oldChild,
                                                   MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMElementremoveChild (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode childNode,
                                                  MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMElementappendChild (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode newChild,
                                                  MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMElementhasChildNodes (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetownerDocument (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMElementcloneNode (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, VBOOL deep,
                                                MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetnodeTypeString (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGettext (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMElementSettext (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetspecified (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetdefinition (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetnodeTypedValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMElementSetnodeTypedValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetdataType (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMElementSetdataType (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetxml (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMElementtransformNode (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode stylesheet,
                                                    char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMElementselectNodes (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *queryString,
                                                  MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMElementselectSingleNode (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *queryString,
                                                       MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetparsed (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetnamespaceURI (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetprefix (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGetbaseName (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMElementtransformNodeToObject (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode stylesheet,
                                                            VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMElementGettagName (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 char **tagName);

HRESULT CVIFUNC MSXML2_IXMLDOMElementgetAttribute (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *name,
                                                   VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMElementsetAttribute (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *name,
                                                   VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMElementremoveAttribute (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      const char *name);

HRESULT CVIFUNC MSXML2_IXMLDOMElementgetAttributeNode (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *name,
                                                       MSXML2Obj_IXMLDOMAttribute *attributeNode);

HRESULT CVIFUNC MSXML2_IXMLDOMElementsetAttributeNode (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMAttribute DOMAttribute,
                                                       MSXML2Obj_IXMLDOMAttribute *attributeNode);

HRESULT CVIFUNC MSXML2_IXMLDOMElementremoveAttributeNode (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMAttribute DOMAttribute,
                                                          MSXML2Obj_IXMLDOMAttribute *attributeNode);

HRESULT CVIFUNC MSXML2_IXMLDOMElementgetElementsByTagName (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           const char *tagName,
                                                           MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMElementnormalize (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetnodeName (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetnodeValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeSetnodeValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetnodeType (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetparentNode (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetchildNodes (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetfirstChild (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetlastChild (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetpreviousSibling (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetnextSibling (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetattributes (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeinsertBefore (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode newChild,
                                                     VARIANT refChild,
                                                     MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributereplaceChild (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode newChild,
                                                     MSXML2Obj_IXMLDOMNode oldChild,
                                                     MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeremoveChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode childNode,
                                                    MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeappendChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode newChild,
                                                    MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributehasChildNodes (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetownerDocument (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributecloneNode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VBOOL deep,
                                                  MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetnodeTypeString (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGettext (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeSettext (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetspecified (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetdefinition (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetnodeTypedValue (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeSetnodeTypedValue (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetdataType (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeSetdataType (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetxml (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributetransformNode (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode stylesheet,
                                                      char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeselectNodes (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *queryString,
                                                    MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeselectSingleNode (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         const char *queryString,
                                                         MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetparsed (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetnamespaceURI (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetprefix (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetbaseName (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributetransformNodeToObject (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IXMLDOMNode stylesheet,
                                                              VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetname (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                char **attributeName);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeGetvalue (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VARIANT *attributeValue);

HRESULT CVIFUNC MSXML2_IXMLDOMAttributeSetvalue (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VARIANT attributeValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetnodeName (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetnodeValue (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentSetnodeValue (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetnodeType (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetparentNode (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetchildNodes (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetfirstChild (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetlastChild (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetpreviousSibling (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetnextSibling (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetattributes (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentinsertBefore (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode newChild,
                                                            VARIANT refChild,
                                                            MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentreplaceChild (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode newChild,
                                                            MSXML2Obj_IXMLDOMNode oldChild,
                                                            MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentremoveChild (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode childNode,
                                                           MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentappendChild (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode newChild,
                                                           MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmenthasChildNodes (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetownerDocument (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentcloneNode (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VBOOL deep,
                                                         MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetnodeTypeString (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGettext (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentSettext (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetspecified (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetdefinition (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetnodeTypedValue (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentSetnodeTypedValue (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetdataType (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentSetdataType (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetxml (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmenttransformNode (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNode stylesheet,
                                                             char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentselectNodes (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           const char *queryString,
                                                           MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentselectSingleNode (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                const char *queryString,
                                                                MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetparsed (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetnamespaceURI (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetprefix (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmentGetbaseName (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMDocumentFragmenttransformNodeToObject (CAObjHandle objectHandle,
                                                                     ERRORINFO *errorInfo,
                                                                     MSXML2Obj_IXMLDOMNode stylesheet,
                                                                     VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetnodeName (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetnodeValue (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMTextSetnodeValue (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetnodeType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetparentNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetchildNodes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetfirstChild (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetlastChild (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetpreviousSibling (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetnextSibling (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetattributes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMTextinsertBefore (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode newChild,
                                                VARIANT refChild,
                                                MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMTextreplaceChild (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode newChild,
                                                MSXML2Obj_IXMLDOMNode oldChild,
                                                MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMTextremoveChild (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLDOMNode childNode,
                                               MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMTextappendChild (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLDOMNode newChild,
                                               MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMTexthasChildNodes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetownerDocument (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMTextcloneNode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, VBOOL deep,
                                             MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetnodeTypeString (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGettext (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMTextSettext (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetspecified (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetdefinition (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetnodeTypedValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMTextSetnodeTypedValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetdataType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMTextSetdataType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetxml (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMTexttransformNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode stylesheet,
                                                 char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMTextselectNodes (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *queryString,
                                               MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMTextselectSingleNode (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *queryString,
                                                    MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetparsed (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetnamespaceURI (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetprefix (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetbaseName (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMTexttransformNodeToObject (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode stylesheet,
                                                         VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetdata (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, char **data);

HRESULT CVIFUNC MSXML2_IXMLDOMTextSetdata (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMTextGetlength (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             long *dataLength);

HRESULT CVIFUNC MSXML2_IXMLDOMTextsubstringData (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long offset, long count,
                                                 char **data);

HRESULT CVIFUNC MSXML2_IXMLDOMTextappendData (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMTextinsertData (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long offset,
                                              const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMTextdeleteData (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long offset,
                                              long count);

HRESULT CVIFUNC MSXML2_IXMLDOMTextreplaceData (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long offset,
                                               long count, const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMTextsplitText (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long offset,
                                             MSXML2Obj_IXMLDOMText *rightHandTextNode);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetnodeName (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetnodeValue (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataSetnodeValue (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetnodeType (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetparentNode (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetchildNodes (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetfirstChild (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetlastChild (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetpreviousSibling (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetnextSibling (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetattributes (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatainsertBefore (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode newChild,
                                                         VARIANT refChild,
                                                         MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatareplaceChild (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode newChild,
                                                         MSXML2Obj_IXMLDOMNode oldChild,
                                                         MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataremoveChild (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode childNode,
                                                        MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataappendChild (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode newChild,
                                                        MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatahasChildNodes (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetownerDocument (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatacloneNode (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      VBOOL deep,
                                                      MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetnodeTypeString (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGettext (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataSettext (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetspecified (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetdefinition (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetnodeTypedValue (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataSetnodeTypedValue (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetdataType (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataSetdataType (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetxml (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatatransformNode (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode stylesheet,
                                                          char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataselectNodes (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *queryString,
                                                        MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataselectSingleNode (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             const char *queryString,
                                                             MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetparsed (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetnamespaceURI (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetprefix (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetbaseName (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatatransformNodeToObject (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_IXMLDOMNode stylesheet,
                                                                  VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetdata (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **data);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataSetdata (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataGetlength (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long *dataLength);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatasubstringData (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          long offset,
                                                          long count,
                                                          char **data);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDataappendData (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatainsertData (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long offset,
                                                       const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatadeleteData (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long offset, long count);

HRESULT CVIFUNC MSXML2_IXMLDOMCharacterDatareplaceData (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long offset, long count,
                                                        const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetnodeName (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetnodeValue (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentSetnodeValue (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetnodeType (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetparentNode (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetchildNodes (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetfirstChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetlastChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetpreviousSibling (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetnextSibling (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetattributes (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentinsertBefore (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode newChild,
                                                   VARIANT refChild,
                                                   MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentreplaceChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode newChild,
                                                   MSXML2Obj_IXMLDOMNode oldChild,
                                                   MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentremoveChild (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode childNode,
                                                  MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentappendChild (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode newChild,
                                                  MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCommenthasChildNodes (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetownerDocument (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentcloneNode (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, VBOOL deep,
                                                MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetnodeTypeString (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGettext (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentSettext (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetspecified (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetdefinition (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetnodeTypedValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentSetnodeTypedValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetdataType (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentSetdataType (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetxml (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMCommenttransformNode (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode stylesheet,
                                                    char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentselectNodes (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *queryString,
                                                  MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentselectSingleNode (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *queryString,
                                                       MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetparsed (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetnamespaceURI (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetprefix (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetbaseName (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMCommenttransformNodeToObject (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode stylesheet,
                                                            VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetdata (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, char **data);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentSetdata (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentGetlength (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                long *dataLength);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentsubstringData (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    long offset, long count,
                                                    char **data);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentappendData (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentinsertData (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long offset, const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentdeleteData (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long offset, long count);

HRESULT CVIFUNC MSXML2_IXMLDOMCommentreplaceData (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long offset, long count,
                                                  const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetnodeName (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetnodeValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionSetnodeValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetnodeType (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetparentNode (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetchildNodes (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetfirstChild (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetlastChild (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetpreviousSibling (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetnextSibling (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetattributes (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectioninsertBefore (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode newChild,
                                                        VARIANT refChild,
                                                        MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionreplaceChild (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode newChild,
                                                        MSXML2Obj_IXMLDOMNode oldChild,
                                                        MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionremoveChild (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNode childNode,
                                                       MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionappendChild (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNode newChild,
                                                       MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionhasChildNodes (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetownerDocument (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectioncloneNode (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL deep,
                                                     MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetnodeTypeString (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGettext (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionSettext (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetspecified (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetdefinition (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetnodeTypedValue (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionSetnodeTypedValue (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetdataType (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionSetdataType (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetxml (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectiontransformNode (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode stylesheet,
                                                         char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionselectNodes (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *queryString,
                                                       MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionselectSingleNode (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            const char *queryString,
                                                            MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetparsed (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetnamespaceURI (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetprefix (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetbaseName (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectiontransformNodeToObject (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Obj_IXMLDOMNode stylesheet,
                                                                 VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetdata (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **data);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionSetdata (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionGetlength (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long *dataLength);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionsubstringData (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long offset, long count,
                                                         char **data);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionappendData (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectioninsertData (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long offset,
                                                      const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectiondeleteData (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long offset, long count);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionreplaceData (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long offset, long count,
                                                       const char *data);

HRESULT CVIFUNC MSXML2_IXMLDOMCDATASectionsplitText (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long offset,
                                                     MSXML2Obj_IXMLDOMText *rightHandTextNode);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetnodeName (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetnodeValue (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionSetnodeValue (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetnodeType (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetparentNode (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetchildNodes (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetfirstChild (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetlastChild (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetpreviousSibling (CAObjHandle objectHandle,
                                                                       ERRORINFO *errorInfo,
                                                                       MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetnextSibling (CAObjHandle objectHandle,
                                                                   ERRORINFO *errorInfo,
                                                                   MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetattributes (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructioninsertBefore (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Obj_IXMLDOMNode newChild,
                                                                 VARIANT refChild,
                                                                 MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionreplaceChild (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Obj_IXMLDOMNode newChild,
                                                                 MSXML2Obj_IXMLDOMNode oldChild,
                                                                 MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionremoveChild (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                MSXML2Obj_IXMLDOMNode childNode,
                                                                MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionappendChild (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                MSXML2Obj_IXMLDOMNode newChild,
                                                                MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionhasChildNodes (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetownerDocument (CAObjHandle objectHandle,
                                                                     ERRORINFO *errorInfo,
                                                                     MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructioncloneNode (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VBOOL deep,
                                                              MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetnodeTypeString (CAObjHandle objectHandle,
                                                                      ERRORINFO *errorInfo,
                                                                      char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGettext (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionSettext (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetspecified (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetdefinition (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetnodeTypedValue (CAObjHandle objectHandle,
                                                                      ERRORINFO *errorInfo,
                                                                      VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionSetnodeTypedValue (CAObjHandle objectHandle,
                                                                      ERRORINFO *errorInfo,
                                                                      VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetdataType (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionSetdataType (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetxml (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructiontransformNode (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_IXMLDOMNode stylesheet,
                                                                  char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionselectNodes (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                const char *queryString,
                                                                MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionselectSingleNode (CAObjHandle objectHandle,
                                                                     ERRORINFO *errorInfo,
                                                                     const char *queryString,
                                                                     MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetparsed (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetnamespaceURI (CAObjHandle objectHandle,
                                                                    ERRORINFO *errorInfo,
                                                                    char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetprefix (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetbaseName (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructiontransformNodeToObject (CAObjHandle objectHandle,
                                                                          ERRORINFO *errorInfo,
                                                                          MSXML2Obj_IXMLDOMNode stylesheet,
                                                                          VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGettarget (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionGetdata (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            char **value);

HRESULT CVIFUNC MSXML2_IXMLDOMProcessingInstructionSetdata (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            const char *value);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetnodeName (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetnodeValue (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceSetnodeValue (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetnodeType (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetparentNode (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetchildNodes (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetfirstChild (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetlastChild (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetpreviousSibling (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetnextSibling (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetattributes (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceinsertBefore (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode newChild,
                                                           VARIANT refChild,
                                                           MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferencereplaceChild (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode newChild,
                                                           MSXML2Obj_IXMLDOMNode oldChild,
                                                           MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceremoveChild (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode childNode,
                                                          MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceappendChild (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode newChild,
                                                          MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferencehasChildNodes (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetownerDocument (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferencecloneNode (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VBOOL deep,
                                                        MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetnodeTypeString (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGettext (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceSettext (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetspecified (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetdefinition (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetnodeTypedValue (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceSetnodeTypedValue (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetdataType (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceSetdataType (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetxml (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferencetransformNode (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            MSXML2Obj_IXMLDOMNode stylesheet,
                                                            char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceselectNodes (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          const char *queryString,
                                                          MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceselectSingleNode (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               const char *queryString,
                                                               MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetparsed (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetnamespaceURI (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetprefix (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferenceGetbaseName (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityReferencetransformNodeToObject (CAObjHandle objectHandle,
                                                                    ERRORINFO *errorInfo,
                                                                    MSXML2Obj_IXMLDOMNode stylesheet,
                                                                    VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMParseErrorGeterrorCode (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long *errorCode);

HRESULT CVIFUNC MSXML2_IXMLDOMParseErrorGeturl (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                char **urlString);

HRESULT CVIFUNC MSXML2_IXMLDOMParseErrorGetreason (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **reasonString);

HRESULT CVIFUNC MSXML2_IXMLDOMParseErrorGetsrcText (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **sourceString);

HRESULT CVIFUNC MSXML2_IXMLDOMParseErrorGetline (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *lineNumber);

HRESULT CVIFUNC MSXML2_IXMLDOMParseErrorGetlinepos (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    long *linePosition);

HRESULT CVIFUNC MSXML2_IXMLDOMParseErrorGetfilepos (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    long *filePosition);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetnodeName (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetnodeValue (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationSetnodeValue (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetnodeType (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetparentNode (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetchildNodes (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetfirstChild (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetlastChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetpreviousSibling (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetnextSibling (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetattributes (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationinsertBefore (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode newChild,
                                                    VARIANT refChild,
                                                    MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationreplaceChild (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode newChild,
                                                    MSXML2Obj_IXMLDOMNode oldChild,
                                                    MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationremoveChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode childNode,
                                                   MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationappendChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode newChild,
                                                   MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationhasChildNodes (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetownerDocument (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationcloneNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL deep,
                                                 MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetnodeTypeString (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGettext (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationSettext (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetspecified (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetdefinition (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetnodeTypedValue (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationSetnodeTypedValue (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetdataType (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationSetdataType (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetxml (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationtransformNode (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_IXMLDOMNode stylesheet,
                                                     char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationselectNodes (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *queryString,
                                                   MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationselectSingleNode (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *queryString,
                                                        MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetparsed (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetnamespaceURI (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetprefix (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetbaseName (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationtransformNodeToObject (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IXMLDOMNode stylesheet,
                                                             VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetpublicId (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *publicId);

HRESULT CVIFUNC MSXML2_IXMLDOMNotationGetsystemId (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *systemId);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetnodeName (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 char **name);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetnodeValue (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMEntitySetnodeValue (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT value);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetnodeType (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetparentNode (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetchildNodes (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetfirstChild (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetlastChild (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetpreviousSibling (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetnextSibling (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetattributes (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityinsertBefore (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode newChild,
                                                  VARIANT refChild,
                                                  MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityreplaceChild (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode newChild,
                                                  MSXML2Obj_IXMLDOMNode oldChild,
                                                  MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityremoveChild (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode childNode,
                                                 MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityappendChild (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode newChild,
                                                 MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityhasChildNodes (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetownerDocument (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXMLDOMEntitycloneNode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, VBOOL deep,
                                               MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetnodeTypeString (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **nodeType);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGettext (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, char **text);

HRESULT CVIFUNC MSXML2_IXMLDOMEntitySettext (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             const char *text);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetspecified (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetdefinition (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetnodeTypedValue (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMEntitySetnodeTypedValue (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetdataType (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMEntitySetdataType (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetxml (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMEntitytransformNode (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode stylesheet,
                                                   char **xmlString);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityselectNodes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 const char *queryString,
                                                 MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityselectSingleNode (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      const char *queryString,
                                                      MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetparsed (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetnamespaceURI (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetprefix (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               char **prefixString);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetbaseName (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 char **nameString);

HRESULT CVIFUNC MSXML2_IXMLDOMEntitytransformNodeToObject (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_IXMLDOMNode stylesheet,
                                                           VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetpublicId (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VARIANT *publicId);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetsystemId (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VARIANT *systemId);

HRESULT CVIFUNC MSXML2_IXMLDOMEntityGetnotationName (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **name);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetnodeName (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetnodeValue (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VARIANT *value);

HRESULT CVIFUNC MSXML2_IXTLRuntimeSetnodeValue (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VARIANT value);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetnodeType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Type_DOMNodeType *type);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetparentNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *parent);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetchildNodes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNodeList *childList);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetfirstChild (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *firstChild);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetlastChild (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode *lastChild);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetpreviousSibling (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXMLDOMNode *previousSibling);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetnextSibling (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLDOMNode *nextSibling);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetattributes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNamedNodeMap *attributeMap);

HRESULT CVIFUNC MSXML2_IXTLRuntimeinsertBefore (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode newChild,
                                                VARIANT refChild,
                                                MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXTLRuntimereplaceChild (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode newChild,
                                                MSXML2Obj_IXMLDOMNode oldChild,
                                                MSXML2Obj_IXMLDOMNode *outOldChild);

HRESULT CVIFUNC MSXML2_IXTLRuntimeremoveChild (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLDOMNode childNode,
                                               MSXML2Obj_IXMLDOMNode *oldChild);

HRESULT CVIFUNC MSXML2_IXTLRuntimeappendChild (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLDOMNode newChild,
                                               MSXML2Obj_IXMLDOMNode *outNewChild);

HRESULT CVIFUNC MSXML2_IXTLRuntimehasChildNodes (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 VBOOL *hasChild);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetownerDocument (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_IXMLDOMDocument *DOMDocument);

HRESULT CVIFUNC MSXML2_IXTLRuntimecloneNode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, VBOOL deep,
                                             MSXML2Obj_IXMLDOMNode *cloneRoot);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetnodeTypeString (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **nodeType);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGettext (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, char **text);

HRESULT CVIFUNC MSXML2_IXTLRuntimeSettext (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           const char *text);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetspecified (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VBOOL *isSpecified);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetdefinition (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *definitionNode);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetnodeTypedValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT *typedValue);

HRESULT CVIFUNC MSXML2_IXTLRuntimeSetnodeTypedValue (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT typedValue);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetdataType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VARIANT *dataTypeName);

HRESULT CVIFUNC MSXML2_IXTLRuntimeSetdataType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *dataTypeName);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetxml (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, char **xmlString);

HRESULT CVIFUNC MSXML2_IXTLRuntimetransformNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode stylesheet,
                                                 char **xmlString);

HRESULT CVIFUNC MSXML2_IXTLRuntimeselectNodes (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *queryString,
                                               MSXML2Obj_IXMLDOMNodeList *resultList);

HRESULT CVIFUNC MSXML2_IXTLRuntimeselectSingleNode (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *queryString,
                                                    MSXML2Obj_IXMLDOMNode *resultNode);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetparsed (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             VBOOL *isParsed);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetnamespaceURI (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetprefix (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             char **prefixString);

HRESULT CVIFUNC MSXML2_IXTLRuntimeGetbaseName (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               char **nameString);

HRESULT CVIFUNC MSXML2_IXTLRuntimetransformNodeToObject (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IXMLDOMNode stylesheet,
                                                         VARIANT outputObject);

HRESULT CVIFUNC MSXML2_IXTLRuntimeuniqueID (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            MSXML2Obj_IXMLDOMNode pNode,
                                            long *pID);

HRESULT CVIFUNC MSXML2_IXTLRuntimedepth (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo,
                                         MSXML2Obj_IXMLDOMNode pNode,
                                         long *pDepth);

HRESULT CVIFUNC MSXML2_IXTLRuntimechildNumber (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLDOMNode pNode,
                                               long *pNumber);

HRESULT CVIFUNC MSXML2_IXTLRuntimeancestorChildNumber (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       const char *bstrNodeName,
                                                       MSXML2Obj_IXMLDOMNode pNode,
                                                       long *pNumber);

HRESULT CVIFUNC MSXML2_IXTLRuntimeabsoluteChildNumber (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_IXMLDOMNode pNode,
                                                       long *pNumber);

HRESULT CVIFUNC MSXML2_IXTLRuntimeformatIndex (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long lIndex,
                                               const char *bstrFormat,
                                               char **pbstrFormattedString);

HRESULT CVIFUNC MSXML2_IXTLRuntimeformatNumber (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                double dblNumber,
                                                const char *bstrFormat,
                                                char **pbstrFormattedString);

HRESULT CVIFUNC MSXML2_IXTLRuntimeformatDate (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              VARIANT varDate,
                                              const char *bstrFormat,
                                              VARIANT varDestLocale,
                                              char **pbstrFormattedString);

HRESULT CVIFUNC MSXML2_IXTLRuntimeformatTime (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              VARIANT varTime,
                                              const char *bstrFormat,
                                              VARIANT varDestLocale,
                                              char **pbstrFormattedString);

HRESULT CVIFUNC MSXML2_IXSLProcessorSetinput (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, VARIANT pVar);

HRESULT CVIFUNC MSXML2_IXSLProcessorGetinput (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              VARIANT *pVar);

HRESULT CVIFUNC MSXML2_IXSLProcessorGetownerTemplate (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IXSLTemplate *ppTemplate);

HRESULT CVIFUNC MSXML2_IXSLProcessorsetStartMode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *mode,
                                                  const char *namespaceURI);

HRESULT CVIFUNC MSXML2_IXSLProcessorGetstartMode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **mode);

HRESULT CVIFUNC MSXML2_IXSLProcessorGetstartModeURI (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **namespaceURI);

HRESULT CVIFUNC MSXML2_IXSLProcessorSetoutput (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VARIANT pOutput);

HRESULT CVIFUNC MSXML2_IXSLProcessorGetoutput (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VARIANT *pOutput);

HRESULT CVIFUNC MSXML2_IXSLProcessortransform (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VBOOL *pDone);

HRESULT CVIFUNC MSXML2_IXSLProcessorreset (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXSLProcessorGetreadyState (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pReadyState);

HRESULT CVIFUNC MSXML2_IXSLProcessoraddParameter (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *baseName,
                                                  VARIANT parameter,
                                                  const char *namespaceURI);

HRESULT CVIFUNC MSXML2_IXSLProcessoraddObject (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               CAObjHandle obj,
                                               const char *namespaceURI);

HRESULT CVIFUNC MSXML2_IXSLProcessorGetstylesheet (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode *stylesheet);

HRESULT CVIFUNC MSXML2_ISAXEntityResolverresolveEntity (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        unsigned short *pwchPublicId,
                                                        unsigned short *pwchSystemId,
                                                        VARIANT *pvarInput);

HRESULT CVIFUNC MSXML2_ISAXLocatorgetColumnNumber (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pnColumn);

HRESULT CVIFUNC MSXML2_ISAXLocatorgetLineNumber (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pnLine);

HRESULT CVIFUNC MSXML2_ISAXXMLFiltergetFeature (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                unsigned short *pwchName,
                                                VBOOL *pvfValue);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputFeature (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                unsigned short *pwchName,
                                                VBOOL vfValue);

HRESULT CVIFUNC MSXML2_ISAXXMLFiltergetProperty (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 unsigned short *pwchName,
                                                 VARIANT *pvarValue);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputProperty (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 unsigned short *pwchName,
                                                 VARIANT varValue);

HRESULT CVIFUNC MSXML2_ISAXXMLFiltergetEntityResolver (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISAXEntityResolver *ppResolver);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputEntityResolver (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISAXEntityResolver pResolver);

HRESULT CVIFUNC MSXML2_ISAXXMLFiltergetContentHandler (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISAXContentHandler *ppHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputContentHandler (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISAXContentHandler pHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLFiltergetDTDHandler (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_ISAXDTDHandler *ppHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputDTDHandler (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_ISAXDTDHandler pHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLFiltergetErrorHandler (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_ISAXErrorHandler *ppHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputErrorHandler (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Obj_ISAXErrorHandler pHandler);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputBaseURL (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                unsigned short *pwchBaseUrl);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputSecureBaseURL (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      unsigned short *pwchSecureBaseUrl);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterparse (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           VARIANT varInput);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterparseURL (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              unsigned short *pwchUrl);

HRESULT CVIFUNC MSXML2_ISAXXMLFiltergetParent (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_ISAXXMLReader *ppReader);

HRESULT CVIFUNC MSXML2_ISAXXMLFilterputParent (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_ISAXXMLReader pReader);

HRESULT CVIFUNC MSXML2_IVBSAXEntityResolverresolveEntity (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **strPublicId,
                                                          char **strSystemId,
                                                          VARIANT *varInput);

HRESULT CVIFUNC MSXML2_IVBSAXLocatorGetcolumnNumber (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long *nColumn);

HRESULT CVIFUNC MSXML2_IVBSAXLocatorGetlineNumber (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *nLine);

HRESULT CVIFUNC MSXML2_IVBSAXLocatorGetpublicId (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 char **strPublicId);

HRESULT CVIFUNC MSXML2_IVBSAXLocatorGetsystemId (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 char **strSystemId);

HRESULT CVIFUNC MSXML2_IVBSAXXMLFilterGetparent (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IVBSAXXMLReader *oReader);

HRESULT CVIFUNC MSXML2_IVBSAXXMLFilterSetByRefparent (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IVBSAXXMLReader oReader);

HRESULT CVIFUNC MSXML2_IMXSchemaDeclHandlerschemaElementDecl (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_ISchemaElement oSchemaElement);

HRESULT CVIFUNC MSXML2_ISchemaElementGetname (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_ISchemaElementGetnamespaceURI (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaElementGetschema (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaElementGetid (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaElementGetitemType (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaElementGetunhandledAttributes (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaElementwriteAnnotation (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      LPUNKNOWN annotationSink,
                                                      VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaElementGetminOccurs (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *minOccurs);

HRESULT CVIFUNC MSXML2_ISchemaElementGetmaxOccurs (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   VARIANT *maxOccurs);

HRESULT CVIFUNC MSXML2_ISchemaElementGettype (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              MSXML2Obj_ISchemaType *type);

HRESULT CVIFUNC MSXML2_ISchemaElementGetscope (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_ISchemaComplexType *scope);

HRESULT CVIFUNC MSXML2_ISchemaElementGetdefaultValue (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **defaultValue);

HRESULT CVIFUNC MSXML2_ISchemaElementGetfixedValue (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    char **fixedValue);

HRESULT CVIFUNC MSXML2_ISchemaElementGetisNillable (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VBOOL *nillable);

HRESULT CVIFUNC MSXML2_ISchemaElementGetidentityConstraints (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_ISchemaItemCollection *constraints);

HRESULT CVIFUNC MSXML2_ISchemaElementGetsubstitutionGroup (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_ISchemaElement *element);

HRESULT CVIFUNC MSXML2_ISchemaElementGetsubstitutionGroupExclusions (CAObjHandle objectHandle,
                                                                     ERRORINFO *errorInfo,
                                                                     MSXML2Type_SCHEMADERIVATIONMETHOD *exclusions);

HRESULT CVIFUNC MSXML2_ISchemaElementGetdisallowedSubstitutions (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Type_SCHEMADERIVATIONMETHOD *disallowed);

HRESULT CVIFUNC MSXML2_ISchemaElementGetisAbstract (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VBOOL *abstract);

HRESULT CVIFUNC MSXML2_ISchemaElementGetisReference (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VBOOL *reference);

HRESULT CVIFUNC MSXML2_ISchemaParticleGetname (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_ISchemaParticleGetnamespaceURI (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaParticleGetschema (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaParticleGetid (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaParticleGetitemType (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaParticleGetunhandledAttributes (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaParticlewriteAnnotation (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       LPUNKNOWN annotationSink,
                                                       VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaParticleGetminOccurs (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT *minOccurs);

HRESULT CVIFUNC MSXML2_ISchemaParticleGetmaxOccurs (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT *maxOccurs);

HRESULT CVIFUNC MSXML2_ISchemaItemGetname (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_ISchemaItemGetnamespaceURI (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaItemGetschema (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaItemGetid (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaItemGetitemType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaItemGetunhandledAttributes (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaItemwriteAnnotation (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   LPUNKNOWN annotationSink,
                                                   VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaGetname (CAObjHandle objectHandle,
                                       ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_ISchemaGetnamespaceURI (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaGetschema (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo,
                                         MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaGetid (CAObjHandle objectHandle,
                                     ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaGetitemType (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaGetunhandledAttributes (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemawriteAnnotation (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               LPUNKNOWN annotationSink,
                                               VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaGettargetNamespace (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **targetNamespace);

HRESULT CVIFUNC MSXML2_ISchemaGetversion (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, char **version);

HRESULT CVIFUNC MSXML2_ISchemaGettypes (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo,
                                        MSXML2Obj_ISchemaItemCollection *types);

HRESULT CVIFUNC MSXML2_ISchemaGetelements (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           MSXML2Obj_ISchemaItemCollection *elements);

HRESULT CVIFUNC MSXML2_ISchemaGetattributes (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             MSXML2Obj_ISchemaItemCollection *attributes);

HRESULT CVIFUNC MSXML2_ISchemaGetattributeGroups (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_ISchemaItemCollection *attributeGroups);

HRESULT CVIFUNC MSXML2_ISchemaGetmodelGroups (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              MSXML2Obj_ISchemaItemCollection *modelGroups);

HRESULT CVIFUNC MSXML2_ISchemaGetnotations (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            MSXML2Obj_ISchemaItemCollection *notations);

HRESULT CVIFUNC MSXML2_ISchemaGetschemaLocations (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_ISchemaStringCollection *schemaLocations);

HRESULT CVIFUNC MSXML2_ISchemaItemCollectionGetitem (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long index,
                                                     MSXML2Obj_ISchemaItem *item);

HRESULT CVIFUNC MSXML2_ISchemaItemCollectionitemByName (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        const char *name,
                                                        MSXML2Obj_ISchemaItem *item);

HRESULT CVIFUNC MSXML2_ISchemaItemCollectionitemByQName (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         const char *name,
                                                         const char *namespaceURI,
                                                         MSXML2Obj_ISchemaItem *item);

HRESULT CVIFUNC MSXML2_ISchemaItemCollectionGetlength (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *length);

HRESULT CVIFUNC MSXML2_ISchemaItemCollectionGet_newEnum (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_ISchemaStringCollectionGetitem (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long index, char **bstr);

HRESULT CVIFUNC MSXML2_ISchemaStringCollectionGetlength (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long *length);

HRESULT CVIFUNC MSXML2_ISchemaStringCollectionGet_newEnum (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetname (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetnamespaceURI (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetschema (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetid (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetitemType (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetunhandledAttributes (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaTypewriteAnnotation (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   LPUNKNOWN annotationSink,
                                                   VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetbaseTypes (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_ISchemaItemCollection *baseTypes);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetfinal (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            MSXML2Type_SCHEMADERIVATIONMETHOD *final);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetvariety (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              MSXML2Type_SCHEMATYPEVARIETY *variety);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetderivedBy (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Type_SCHEMADERIVATIONMETHOD *derivedBy);

HRESULT CVIFUNC MSXML2_ISchemaTypeisValid (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo,
                                           const char *data, VBOOL *valid);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetminExclusive (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **minExclusive);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetminInclusive (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **minInclusive);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetmaxExclusive (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **maxExclusive);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetmaxInclusive (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **maxInclusive);

HRESULT CVIFUNC MSXML2_ISchemaTypeGettotalDigits (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT *totalDigits);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetfractionDigits (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     VARIANT *fractionDigits);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetlength (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             VARIANT *length);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetminLength (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VARIANT *minLength);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetmaxLength (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                VARIANT *maxLength);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetenumeration (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_ISchemaStringCollection *enumeration);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetwhitespace (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Type_SCHEMAWHITESPACE *whitespace);

HRESULT CVIFUNC MSXML2_ISchemaTypeGetpatterns (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_ISchemaStringCollection *patterns);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetname (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **name);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetnamespaceURI (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetschema (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetid (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetitemType (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetunhandledAttributes (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypewriteAnnotation (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          LPUNKNOWN annotationSink,
                                                          VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetbaseTypes (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISchemaItemCollection *baseTypes);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetfinal (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Type_SCHEMADERIVATIONMETHOD *final);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetvariety (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Type_SCHEMATYPEVARIETY *variety);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetderivedBy (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Type_SCHEMADERIVATIONMETHOD *derivedBy);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeisValid (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *data, VBOOL *valid);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetminExclusive (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **minExclusive);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetminInclusive (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **minInclusive);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetmaxExclusive (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **maxExclusive);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetmaxInclusive (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          char **maxInclusive);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGettotalDigits (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         VARIANT *totalDigits);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetfractionDigits (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            VARIANT *fractionDigits);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetlength (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    VARIANT *length);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetminLength (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VARIANT *minLength);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetmaxLength (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VARIANT *maxLength);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetenumeration (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_ISchemaStringCollection *enumeration);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetwhitespace (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Type_SCHEMAWHITESPACE *whitespace);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetpatterns (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_ISchemaStringCollection *patterns);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetisAbstract (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        VBOOL *abstract);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetanyAttribute (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_ISchemaAny *anyAttribute);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetattributes (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_ISchemaItemCollection *attributes);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetcontentType (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Type_SCHEMACONTENTTYPE *contentType);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetcontentModel (CAObjHandle objectHandle,
                                                          ERRORINFO *errorInfo,
                                                          MSXML2Obj_ISchemaModelGroup *contentModel);

HRESULT CVIFUNC MSXML2_ISchemaComplexTypeGetprohibitedSubstitutions (CAObjHandle objectHandle,
                                                                     ERRORINFO *errorInfo,
                                                                     MSXML2Type_SCHEMADERIVATIONMETHOD *prohibited);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetname (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetnamespaceURI (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetschema (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetid (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetitemType (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetunhandledAttributes (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaAnywriteAnnotation (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  LPUNKNOWN annotationSink,
                                                  VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetminOccurs (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VARIANT *minOccurs);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetmaxOccurs (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               VARIANT *maxOccurs);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetnamespaces (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_ISchemaStringCollection *namespaces);

HRESULT CVIFUNC MSXML2_ISchemaAnyGetprocessContents (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Type_SCHEMAPROCESSCONTENTS *processContents);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetname (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 char **name);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetnamespaceURI (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetschema (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetid (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetitemType (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetunhandledAttributes (CAObjHandle objectHandle,
                                                                ERRORINFO *errorInfo,
                                                                MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupwriteAnnotation (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         LPUNKNOWN annotationSink,
                                                         VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetminOccurs (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      VARIANT *minOccurs);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetmaxOccurs (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      VARIANT *maxOccurs);

HRESULT CVIFUNC MSXML2_ISchemaModelGroupGetparticles (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      MSXML2Obj_ISchemaItemCollection *particles);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetname (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                char **name);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetnamespaceURI (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetschema (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetid (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetitemType (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetunhandledAttributes (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaAttributewriteAnnotation (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        LPUNKNOWN annotationSink,
                                                        VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGettype (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_ISchemaType *type);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetscope (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_ISchemaComplexType *scope);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetdefaultValue (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **defaultValue);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetfixedValue (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      char **fixedValue);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetuse (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Type_SCHEMAUSE *use);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGetisReference (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       VBOOL *reference);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupGetname (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     char **name);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupGetnamespaceURI (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupGetschema (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupGetid (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   char **id);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupGetitemType (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupGetunhandledAttributes (CAObjHandle objectHandle,
                                                                    ERRORINFO *errorInfo,
                                                                    MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupwriteAnnotation (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             LPUNKNOWN annotationSink,
                                                             VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupGetanyAttribute (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Obj_ISchemaAny *anyAttribute);

HRESULT CVIFUNC MSXML2_ISchemaAttributeGroupGetattributes (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_ISchemaItemCollection *attributes);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetname (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         char **name);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetnamespaceURI (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetschema (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetid (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **id);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetitemType (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetunhandledAttributes (CAObjHandle objectHandle,
                                                                        ERRORINFO *errorInfo,
                                                                        MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintwriteAnnotation (CAObjHandle objectHandle,
                                                                 ERRORINFO *errorInfo,
                                                                 LPUNKNOWN annotationSink,
                                                                 VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetselector (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             char **selector);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetfields (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           MSXML2Obj_ISchemaStringCollection *fields);

HRESULT CVIFUNC MSXML2_ISchemaIdentityConstraintGetreferencedKey (CAObjHandle objectHandle,
                                                                  ERRORINFO *errorInfo,
                                                                  MSXML2Obj_ISchemaIdentityConstraint *key);

HRESULT CVIFUNC MSXML2_ISchemaNotationGetname (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **name);

HRESULT CVIFUNC MSXML2_ISchemaNotationGetnamespaceURI (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **namespaceURI);

HRESULT CVIFUNC MSXML2_ISchemaNotationGetschema (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_ISchema *schema);

HRESULT CVIFUNC MSXML2_ISchemaNotationGetid (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, char **id);

HRESULT CVIFUNC MSXML2_ISchemaNotationGetitemType (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Type_SOMITEMTYPE *itemType);

HRESULT CVIFUNC MSXML2_ISchemaNotationGetunhandledAttributes (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              MSXML2Obj_IVBSAXAttributes *attributes);

HRESULT CVIFUNC MSXML2_ISchemaNotationwriteAnnotation (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       LPUNKNOWN annotationSink,
                                                       VBOOL *isWritten);

HRESULT CVIFUNC MSXML2_ISchemaNotationGetsystemIdentifier (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **uri);

HRESULT CVIFUNC MSXML2_ISchemaNotationGetpublicIdentifier (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **uri);

HRESULT CVIFUNC MSXML2_IXMLElementCollectionSetlength (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long p);

HRESULT CVIFUNC MSXML2_IXMLElementCollectionGetlength (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *p);

HRESULT CVIFUNC MSXML2_IXMLElementCollectionGet_newEnum (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_IXMLElementCollectionitem (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT var1, VARIANT var2,
                                                  CAObjHandle *ppDisp);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetroot (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            MSXML2Obj_IXMLElement *p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetfileSize (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetfileModifiedDate (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetfileUpdatedDate (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGeturl (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentSeturl (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, const char *p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetmimeType (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetreadyState (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo, long *pl);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetcharset (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentSetcharset (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetversion (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetdoctype (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentGetdtdURL (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLDocumentcreateElement (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT vType, VARIANT var1,
                                                  MSXML2Obj_IXMLElement *ppElem);

HRESULT CVIFUNC MSXML2_IXMLElementGettagName (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLElementSettagName (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              const char *p);

HRESULT CVIFUNC MSXML2_IXMLElementGetparent (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             MSXML2Obj_IXMLElement *ppParent);

HRESULT CVIFUNC MSXML2_IXMLElementsetAttribute (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                const char *strPropertyName,
                                                VARIANT propertyValue);

HRESULT CVIFUNC MSXML2_IXMLElementgetAttribute (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                const char *strPropertyName,
                                                VARIANT *propertyValue);

HRESULT CVIFUNC MSXML2_IXMLElementremoveAttribute (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   const char *strPropertyName);

HRESULT CVIFUNC MSXML2_IXMLElementGetchildren (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLElementCollection *pp);

HRESULT CVIFUNC MSXML2_IXMLElementGettype (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *plType);

HRESULT CVIFUNC MSXML2_IXMLElementGettext (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLElementSettext (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, const char *p);

HRESULT CVIFUNC MSXML2_IXMLElementaddChild (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo,
                                            MSXML2Obj_IXMLElement pChildElem,
                                            long lIndex, long lReserved);

HRESULT CVIFUNC MSXML2_IXMLElementremoveChild (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               MSXML2Obj_IXMLElement pChildElem);

HRESULT CVIFUNC MSXML2_IXMLElement2GettagName (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLElement2SettagName (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo,
                                               const char *p);

HRESULT CVIFUNC MSXML2_IXMLElement2Getparent (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              MSXML2Obj_IXMLElement2 *ppParent);

HRESULT CVIFUNC MSXML2_IXMLElement2setAttribute (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 const char *strPropertyName,
                                                 VARIANT propertyValue);

HRESULT CVIFUNC MSXML2_IXMLElement2getAttribute (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 const char *strPropertyName,
                                                 VARIANT *propertyValue);

HRESULT CVIFUNC MSXML2_IXMLElement2removeAttribute (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *strPropertyName);

HRESULT CVIFUNC MSXML2_IXMLElement2Getchildren (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLElementCollection *pp);

HRESULT CVIFUNC MSXML2_IXMLElement2Gettype (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *plType);

HRESULT CVIFUNC MSXML2_IXMLElement2Gettext (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, char **p);

HRESULT CVIFUNC MSXML2_IXMLElement2Settext (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, const char *p);

HRESULT CVIFUNC MSXML2_IXMLElement2addChild (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo,
                                             MSXML2Obj_IXMLElement2 pChildElem,
                                             long lIndex, long lReserved);

HRESULT CVIFUNC MSXML2_IXMLElement2removeChild (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLElement2 pChildElem);

HRESULT CVIFUNC MSXML2_IXMLElement2Getattributes (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  MSXML2Obj_IXMLElementCollection *pp);

HRESULT CVIFUNC MSXML2_IXMLAttributeGetname (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, char **n);

HRESULT CVIFUNC MSXML2_IXMLAttributeGetvalue (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, char **v);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionGetitem (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long index,
                                                MSXML2Obj_IXMLDOMNode *listItem);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionGetlength (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long *listLength);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionnextNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *nextItem);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionreset (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionGet_newEnum (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionGetexpr (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                char **expression);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionSetexpr (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                const char *expression);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionGetcontext (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode *ppNode);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionSetByRefcontext (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        MSXML2Obj_IXMLDOMNode ppNode);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionpeekNode (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 MSXML2Obj_IXMLDOMNode *ppNode);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionmatches (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo,
                                                MSXML2Obj_IXMLDOMNode pNode,
                                                MSXML2Obj_IXMLDOMNode *ppNode);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionremoveNext (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   MSXML2Obj_IXMLDOMNode *ppNode);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionremoveAll (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionclone (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              MSXML2Obj_IXMLDOMSelection *ppNode);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectiongetProperty (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *name,
                                                    VARIANT *value);

HRESULT CVIFUNC MSXML2_IXMLDOMSelectionsetProperty (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    const char *name,
                                                    VARIANT value);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestopen (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  const char *bstrMethod,
                                                  const char *bstrUrl,
                                                  VARIANT varAsync,
                                                  VARIANT bstrUser,
                                                  VARIANT bstrPassword);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestsetRequestHeader (CAObjHandle objectHandle,
                                                              ERRORINFO *errorInfo,
                                                              const char *bstrHeader,
                                                              const char *bstrValue);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestgetResponseHeader (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               const char *bstrHeader,
                                                               char **pbstrValue);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestgetAllResponseHeaders (CAObjHandle objectHandle,
                                                                   ERRORINFO *errorInfo,
                                                                   char **pbstrHeaders);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestsend (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  VARIANT varBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestabort (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestGetstatus (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *plStatus);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestGetstatusText (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           char **pbstrStatus);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestGetresponseXML (CAObjHandle objectHandle,
                                                            ERRORINFO *errorInfo,
                                                            CAObjHandle *ppBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestGetresponseText (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             char **pbstrBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestGetresponseBody (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VARIANT *pvarBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestGetresponseStream (CAObjHandle objectHandle,
                                                               ERRORINFO *errorInfo,
                                                               VARIANT *pvarBody);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestGetreadyState (CAObjHandle objectHandle,
                                                           ERRORINFO *errorInfo,
                                                           long *plState);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestSetonreadystatechange (CAObjHandle objectHandle,
                                                                   ERRORINFO *errorInfo,
                                                                   CAObjHandle newValue);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestsetTimeouts (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long resolveTimeout,
                                                         long connectTimeout,
                                                         long sendTimeout,
                                                         long receiveTimeout);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestwaitForResponse (CAObjHandle objectHandle,
                                                             ERRORINFO *errorInfo,
                                                             VARIANT timeoutInSeconds,
                                                             VBOOL *isSuccessful);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestgetOption (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Type_SERVERXMLHTTP_OPTION option,
                                                       VARIANT *value);

HRESULT CVIFUNC MSXML2_IServerXMLHTTPRequestsetOption (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       MSXML2Type_SERVERXMLHTTP_OPTION option,
                                                       VARIANT value);

HRESULT CVIFUNC MSXML2_IMXNamespacePrefixesGetitem (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    long index, char **prefix);

HRESULT CVIFUNC MSXML2_IMXNamespacePrefixesGetlength (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long *length);

HRESULT CVIFUNC MSXML2_IMXNamespacePrefixesGet_newEnum (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        LPUNKNOWN *ppUnk);

HRESULT CVIFUNC MSXML2_XMLDOMDocumentEventsRegOnondataavailable (CAObjHandle serverObject,
                                                                 XMLDOMDocumentEventsRegOnondataavailable_CallbackType callbackFunction,
                                                                 void *callbackData,
                                                                 int enableCallbacks,
                                                                 int *callbackId);

HRESULT CVIFUNC MSXML2_XMLDOMDocumentEventsRegOnonreadystatechange (CAObjHandle serverObject,
                                                                    XMLDOMDocumentEventsRegOnonreadystatechange_CallbackType callbackFunction,
                                                                    void *callbackData,
                                                                    int enableCallbacks,
                                                                    int *callbackId);
#ifdef __cplusplus
    }
#endif
#endif /* _MSXML2_H */
