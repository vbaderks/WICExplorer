//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
module;

#include "Macros.h"

export module CodeGenerator;

import <strsafe.h>;
import <std.h>;

export struct ICodeGenerator
{
    virtual ~ICodeGenerator() = default;

    virtual void BeginVariableScope(std::wstring_view varType, std::wstring_view varBaseName, std::wstring_view varInitValue) = 0;
    virtual void EndVariableScope() = 0;
    virtual void CallFunction(const wchar_t* func, ...) = 0;
    virtual const std::wstring& GetLastVariableName() noexcept = 0;
    virtual std::wstring GenerateCode() = 0;

protected:
    ICodeGenerator() = default;
    ICodeGenerator(const ICodeGenerator&) = default;
    ICodeGenerator(ICodeGenerator&&) = default;
    ICodeGenerator& operator=(const ICodeGenerator&) = default;
    ICodeGenerator& operator=(ICodeGenerator&&) = default;
};

export class CSimpleCodeGenerator final : public ICodeGenerator
{
public:
    CSimpleCodeGenerator() noexcept(false)
    {
        BeginVariable(L"IWICImagingFactory*", L"imagingFactory", L"nullptr");
        CallFunction(L"CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*) &imagingFactory)");
    }

    void BeginVariableScope(std::wstring_view varType, std::wstring_view varBaseName, std::wstring_view varInitValue) override
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

    void CallFunction(const wchar_t* func, ...) override
    {
        constexpr size_t maxCallLength = 1024;
        wchar_t call[maxCallLength];

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

    const std::wstring& GetLastVariableName() noexcept override
    {
        return m_lastVarName;
    }

    std::wstring GenerateCode() override
    {
        std::wstring out;
        for (size_t l = 0; l < m_lines.size(); l++)
        {
            out += m_lines[l].c_str();
            out += L"\n";
        }

        return out;
    }

private:
    void BeginVariable(std::wstring_view varType, std::wstring_view varBaseName, std::wstring_view varInitValue)
    {
#pragma warning(push)
#pragma warning(disable : 4296) // '<': expression is always false (known defect in MSVC compiler 2022 17.4 Preview 1.0)
        AddLine(std::format(L"{} {} = {}", varType, varBaseName, varInitValue));
#pragma warning(pop)

        m_lastVarName = varBaseName;
    }

    void AddLine(const std::wstring& line)
    {
        m_lines.push_back(std::wstring(m_indent, L' ') + line);
    }

    static constexpr size_t INDENT_SPACES{4};

    std::vector<std::wstring> m_lines;
    std::wstring m_lastVarName;
    size_t m_indent{};
};
