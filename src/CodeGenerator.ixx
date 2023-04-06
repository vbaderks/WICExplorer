﻿// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

export module CodeGenerator;

import ICodeGenerator;

import <std.h>;


export class CSimpleCodeGenerator final : public ICodeGenerator
{
public:
    CSimpleCodeGenerator() noexcept(false)
    {
        BeginVariable(L"IWICImagingFactory*", L"imagingFactory", L"nullptr");
        CallFunction(L"CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*) &imagingFactory);");
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

    void CallFunction(const std::wstring& func) override
    {
        AddLine(func);
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
        AddLine(std::format(L"{} {} = {};", varType, varBaseName, varInitValue));
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
