export module ICodeGenerator;

import <std.h>;

export struct ICodeGenerator
{
    virtual ~ICodeGenerator() = default;

    virtual void BeginVariableScope(std::wstring_view varType, std::wstring_view varBaseName, std::wstring_view varInitValue) = 0;
    virtual void EndVariableScope() = 0;
    virtual void CallFunction(const std::wstring& func) = 0;
    virtual const std::wstring& GetLastVariableName() noexcept = 0;
    virtual std::wstring GenerateCode() = 0;

protected:
    ICodeGenerator() = default;
    ICodeGenerator(const ICodeGenerator&) = default;
    ICodeGenerator(ICodeGenerator&&) = default;
    ICodeGenerator& operator=(const ICodeGenerator&) = default;
    ICodeGenerator& operator=(ICodeGenerator&&) = default;
};
