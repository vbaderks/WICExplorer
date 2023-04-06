﻿// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

#include "ImportMsXml2.h"

#include <atlstr.h>

export module MetadataTranslator;


export class CMetadataTranslator final
{
public:
    static CMetadataTranslator &Inst()
    {
        static CMetadataTranslator inst;
        static bool inited = false;

        if (!inited)
        {
            // We do not fail if there was an error loading the dictionary
            inst.LoadTranslations();

            inited = true;
        }

        return inst;
    }

    HRESULT Translate(const GUID &format, PROPVARIANT *pv, CString &out) const;

private:
    struct Key final
    {
        Key(PCWSTR guidStr, PCWSTR idStr) noexcept;
        Key() = default;

        GUID m_format{};
        int m_id{};

        bool operator == (const Key &other) const noexcept
        {
            return other.m_id == m_id && other.m_format == m_format;
        }
    };

    CMetadataTranslator() noexcept = default;

    static HRESULT ReadPropVariantInteger(PROPVARIANT *pv, int &out) noexcept;
    HRESULT LoadFormat(MSXML2::IXMLDOMNode* formatNodeArg);
    HRESULT LoadTranslations();

    CSimpleMap<Key, CString> m_dictionary;
};

