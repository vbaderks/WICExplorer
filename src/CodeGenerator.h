//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#pragma once

#include "Macros.h"

struct ICodeGenerator
{
    virtual ~ICodeGenerator() = default;

    ICodeGenerator() = default;
    ICodeGenerator(const ICodeGenerator&) = default;
    ICodeGenerator(ICodeGenerator&&) = default;
    ICodeGenerator& operator=(const ICodeGenerator&) = default;
    ICodeGenerator& operator=(ICodeGenerator&&) = default;

    virtual void BeginVariableScope(LPCWSTR varType, LPCWSTR varBaseName, LPCWSTR varInitValue) = 0;
    virtual void EndVariableScope() = 0;
    virtual void CallFunction(LPCWSTR func, ...) = 0;

    virtual LPCWSTR GetLastVariableName() noexcept = 0;

    virtual void GenerateCode(CString &out) = 0;
};

class CSimpleCodeGenerator final : public ICodeGenerator
{
public:
    CSimpleCodeGenerator() noexcept(false)
    {
        BeginVariable(L"IWICImagingFactory*", L"imagingFactory", L"nullptr");
        CallFunction(L"CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*) &imagingFactory)");
    }

    void BeginVariableScope(const LPCWSTR varType, const LPCWSTR varBaseName, const LPCWSTR varInitValue) override
    {
        AddLine(L"{");

        m_indent += INDENT_SPACES;

        BeginVariable(varType, varBaseName, varInitValue);
    }

    void EndVariableScope() override
    {
        if (m_indent >= INDENT_SPACES)
        {
            m_indent -= INDENT_SPACES;

            AddLine(L"}");
        }
    }

    void CallFunction(LPCWSTR func, ...) override
    {
        constexpr size_t maxCallLength = 1024;
        WCHAR call[maxCallLength];

        WARNING_SUPPRESS_NEXT_LINE(26826) //  Don't use C-style variable arguments (f.55).
        va_list args;

        WARNING_SUPPRESS_NEXT_LINE(26826) //  Don't use C-style variable arguments (f.55).
        WARNING_SUPPRESS_NEXT_LINE(26492) // Don't use const_cast to cast away const or volatile (type.3).
        va_start(args, func);

        StringCchVPrintf(call, maxCallLength, func, args);

        WARNING_SUPPRESS_NEXT_LINE(26826) //  Don't use C-style variable arguments (f.55).
        va_end(args);

        StringCchCatW(call, maxCallLength, L";");

        AddLine(call);
    }

    LPCWSTR GetLastVariableName() noexcept override
    {
        return m_lastVarName;
    }

    void GenerateCode(CString &out) override
    {
        out = L"";
        for (int l = 0; l < m_lines.GetSize(); l++)
        {
            out += m_lines[l];
            out += L"\n";
        }
    }

private:
    void BeginVariable(const LPCWSTR varType, const LPCWSTR varBaseName, const LPCWSTR varInitValue)
    {
        CString decl;
        decl.Format(L"%s %s = %s;", varType, varBaseName, varInitValue);

        AddLine(decl);

        m_lastVarName = varBaseName;
    }

    void AddLine(const LPCWSTR line)
    {
        m_lines.Add(CString(L' ', m_indent) + line);
    }

    enum { INDENT_SPACES = 4 };

    CSimpleArray<CString> m_lines;
    CString m_lastVarName;
    int m_indent{};
};
