// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

#include "ImportMsXml2.h"

export module MetadataTranslator;

import <std.h>;

export class CMetadataTranslator final
{
public:
    static CMetadataTranslator &Inst()
    {
        static CMetadataTranslator inst;
        static bool initialized;

        if (!initialized)
        {
            // We do not fail if there was an error loading the dictionary
            inst.LoadTranslations();

            initialized = true;
        }

        return inst;
    }

    HRESULT Translate(const GUID &format, const PROPVARIANT *pv, std::wstring &out) const;

private:
    struct Key final
    {
        Key() = default;
        Key(PCWSTR guidStr, PCWSTR idStr) noexcept;

        GUID m_format{};
        int m_id{};

        bool operator == (const Key &other) const noexcept
        {
            return other.m_id == m_id && other.m_format == m_format;
        }
    };

    CMetadataTranslator() noexcept = default;

    static HRESULT ReadPropVariantInteger(const PROPVARIANT *pv, int &out) noexcept;
    HRESULT LoadFormat(MSXML2::IXMLDOMNode* formatNodeArg);
    HRESULT LoadTranslations();

    std::vector<std::pair<Key, std::wstring>> m_dictionary;
};
