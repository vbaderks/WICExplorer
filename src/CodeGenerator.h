//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#pragma once

class ICodeGenerator
{
public:
    virtual ~ICodeGenerator() = default;

    virtual void BeginVariableScope(LPCWSTR varType, LPCWSTR varBaseName, LPCWSTR varInitValue) = 0;
    virtual void EndVariableScope() = 0;
    virtual void CallFunction(LPCWSTR func, ...) = 0;

    virtual LPCWSTR GetLastVariableName() = 0;

    virtual void GenerateCode(CString &out) = 0;
};

class CSimpleCodeGenerator final : public ICodeGenerator
{
public:
    CSimpleCodeGenerator()
    {
        BeginVariable(L"IWICImagingFactory*", L"imagingFactory", L"NULL");
        CallFunction(L"CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*) &imagingFactory)");
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
        const size_t maxCallLength = 1024;
        WCHAR call[maxCallLength];

        va_list args;
        va_start(args, func);

        StringCchVPrintf(call, maxCallLength, func, args);

        va_end(args);

        StringCchCatW(call, maxCallLength, L";");

        AddLine(call);
    }

    LPCWSTR GetLastVariableName() override
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
    void BeginVariable(LPCWSTR varType, LPCWSTR varBaseName, LPCWSTR varInitValue)
    {
        CString decl;
        decl.Format(L"%s %s = %s;", varType, varBaseName, varInitValue);

        AddLine(decl);

        m_lastVarName = varBaseName;
    }

    void AddLine(LPCWSTR line)
    {
        m_lines.Add(CString(L' ', m_indent) + line);
    }

    enum { INDENT_SPACES = 4 };

    CSimpleArray<CString> m_lines;
    CString m_lastVarName;
    int m_indent{};
};
