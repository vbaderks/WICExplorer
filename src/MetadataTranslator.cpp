// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

#include "Macros.h"
#include "ImportMsXml2.h"

module MetadataTranslator;


CMetadataTranslator::Key::Key(const PCWSTR guidStr, const PCWSTR idStr) noexcept :
    m_id{_wtoi(idStr)}
{
    VERIFY(SUCCEEDED(CLSIDFromString(guidStr, &m_format)));
}

HRESULT CMetadataTranslator::ReadPropVariantInteger(PROPVARIANT* pv, int& out) noexcept
{
    HRESULT result{E_INVALIDARG};

    ASSERT(pv);
    if (pv)
    {
        switch (pv->vt)
        {
        case VT_I1:
            out = pv->cVal;
            result = S_OK;
            break;
        case VT_UI1:
            out = pv->bVal;
            result = S_OK;
            break;
        case VT_I2:
            out = pv->iVal;
            result = S_OK;
            break;
        case VT_UI2:
            out = pv->uiVal;
            result = S_OK;
            break;
        case VT_I4:
            out = pv->lVal;
            result = S_OK;
            break;
        case VT_UI4:
            out = pv->ulVal;
            result = S_OK;
            break;
        case VT_INT:
            out = pv->intVal;
            result = S_OK;
            break;
        case VT_UINT:
            out = pv->uintVal;
            result = S_OK;
            break;
        default:
            result = E_INVALIDARG;
            break;
        }
    }

    return result;
}

HRESULT CMetadataTranslator::Translate(const GUID& format, PROPVARIANT* pv, std::wstring& out) const
{
    HRESULT result;

    int id{};

    // For now, pv must be one of the integers
    IFC(ReadPropVariantInteger(pv, id));

    // Lookup the string
    Key k;
    k.m_format = format;
    k.m_id = id;
    const auto item{std::find_if(m_dictionary.cbegin(), m_dictionary.cend(),
        [&k](const std::pair<Key, std::wstring>& m) noexcept -> bool { return m.first == k; })};

    if (item != m_dictionary.cend())
    {
        // Found one!
        out = item->second;
    }
    else
    {
        // No translation found
        result = E_FAIL;
    }

    return result;
}

HRESULT CMetadataTranslator::LoadFormat(MSXML2::IXMLDOMNode* formatNodeArg)
{
    // Get the format
    _bstr_t formatGuidStr;
    MSXML2::IXMLDOMNodePtr formatNode{formatNodeArg};
    const MSXML2::IXMLDOMNodePtr formatGuidNode = formatNode->attributes->getNamedItem(L"guid");
    if (formatGuidNode)
    {
        WARNING_SUPPRESS_NEXT_LINE(33005) // VARIANT '' was provided as an _In_ or _InOut_ parameter but was not initialized
            formatGuidStr = formatGuidNode->nodeValue;
    }

    // Verify that the GUID has some text
    if (formatGuidStr.length() < 1)
    {
        return E_UNEXPECTED;
    }

    // Load each entry
    MSXML2::IXMLDOMNodeListPtr entryNodes = formatNode->childNodes;
    for (int ei = 0; ei < entryNodes->length; ei++)
    {
        if (_wcsicmp(entryNodes->item[ei]->nodeName, TEXT("entry")) == 0)
        {
            const MSXML2::IXMLDOMNodePtr entryIdNode = entryNodes->item[ei]->attributes->getNamedItem(L"id");
            const MSXML2::IXMLDOMNodePtr entryValueNode = entryNodes->item[ei]->attributes->getNamedItem(L"value");

            if (entryIdNode && entryValueNode)
            {
                WARNING_SUPPRESS_NEXT_LINE(33005) // VARIANT '' was provided as an _In_ or _InOut_ parameter but was not initialized
                    _bstr_t idStr{entryIdNode->nodeValue};

                WARNING_SUPPRESS_NEXT_LINE(33005) // VARIANT '' was provided as an _In_ or _InOut_ parameter but was not initialized
                    _bstr_t valueStr{entryValueNode->nodeValue};

                // Finally, we can add this entry
                m_dictionary.push_back(std::make_pair(Key(formatGuidStr, idStr), static_cast<PCWSTR>(valueStr)));
            }
        }
    }

    return S_OK;
}

HRESULT CMetadataTranslator::LoadTranslations()
{
    HRESULT result = S_OK;

    // Create the XML document object
    MSXML2::IXMLDOMDocumentPtr xmlDoc;
    IFC(xmlDoc.CreateInstance(__uuidof(MSXML2::DOMDocument)));

    // Do not allow asynchronous download
    xmlDoc->async = VARIANT_FALSE;

    // Load the translations file
    _variant_t dictFilename(L"MetadataDictionary.xml");
    _variant_t br(true);
    br = xmlDoc->load(dictFilename);
    if (!static_cast<bool>(br))
    {
        return E_FAIL;
    }

    // Locate each of the Metadata formats, and process them.
    // <dictionary> -> <format /> <format /> ... </dictionary>
    MSXML2::IXMLDOMNodeListPtr dictNodes = xmlDoc->childNodes;
    for (int di = 0; di < dictNodes->length; di++)
    {
        if (_wcsicmp(dictNodes->item[di]->nodeName, TEXT("dictionary")) == 0)
        {
            MSXML2::IXMLDOMNodeListPtr formatNodes = dictNodes->item[di]->childNodes;
            for (int fi = 0; fi < formatNodes->length; fi++)
            {
                if (_wcsicmp(formatNodes->item[fi]->nodeName, TEXT("format")) == 0)
                {
                    LoadFormat(formatNodes->item[fi]);
                }
            }
        }
    }

    return result;
}
