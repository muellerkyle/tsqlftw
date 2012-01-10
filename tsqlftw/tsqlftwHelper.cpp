#include "tsqlftwHelper.h"
#using <mscorlib.dll>
//#using <C:\Git\node.net\tsqlftwLib\bin\Debug\tsqlftwLib.dll>
#using <tsqlftwLib.dll>
#include <gcroot.h>

using namespace System::Runtime::InteropServices;
using namespace System::Reflection;
using namespace tsqlftw;

// we need to look for the assembly in the current directory
// node will search for it next to the node.exe binary
System::Reflection::Assembly ^OnAssemblyResolve(System::Object ^obj, System::ResolveEventArgs ^args)
{
    System::String ^path = System::Environment::CurrentDirectory;
    array<System::String^>^ assemblies =
        System::IO::Directory::GetFiles(path, "*.dll");
    for (long ii = 0; ii < assemblies->Length; ii++) {
        AssemblyName ^name = AssemblyName::GetAssemblyName(assemblies[ii]);
        if (AssemblyName::ReferenceMatchesDefinition(gcnew AssemblyName(args->Name), name)) {
            return Assembly::Load(name);
        }
    }
    return nullptr;
}

// register a custom assembly load handler
void LoadAssembly()
{
    System::AppDomain::CurrentDomain->AssemblyResolve +=
        gcnew System::ResolveEventHandler(OnAssemblyResolve);
}

class tsqlftwWrapper : public tsqlftwHelper {
private:
    gcroot<tsqlftwClass^> _tsqlftwClass;

public:
    tsqlftwWrapper()
    {
        _tsqlftwClass = gcnew tsqlftwClass();
    }

    virtual bool connect(std::string& connString, std::string& err, std::string& result)
    {
        try
        {
			System::String^ data = "false";
            if (_tsqlftwClass->Connect(gcnew System::String(connString.c_str()))) data = "true";

            System::IntPtr p = Marshal::StringToHGlobalAnsi(data);
            result = static_cast<char*>(p.ToPointer());
            Marshal::FreeHGlobal(p);
            return false;
        }
        catch(System::Exception^ e)
        {
            System::IntPtr p = Marshal::StringToHGlobalAnsi(e->Message);
            err = static_cast<char*>(p.ToPointer());
            Marshal::FreeHGlobal(p);
            return true;
        }
    }


    virtual bool query(std::string& query, std::string& err, std::string& result)
    {
        try
        {
            System::String^ data =  _tsqlftwClass->Query(gcnew System::String(query.c_str()));

            System::IntPtr p = Marshal::StringToHGlobalAnsi(data);
            result = static_cast<char*>(p.ToPointer());
            Marshal::FreeHGlobal(p);
            return false;
        }
        catch(System::Exception^ e)
        {
            System::IntPtr p = Marshal::StringToHGlobalAnsi(e->Message);
            err = static_cast<char*>(p.ToPointer());
            Marshal::FreeHGlobal(p);
            return true;
        }
    }

    virtual bool close(std::string& err, std::string& result)
    {
        try
        {
			System::String^ data = "false";
            if (_tsqlftwClass->Close()) data = "true";

            System::IntPtr p = Marshal::StringToHGlobalAnsi(data);
            result = static_cast<char*>(p.ToPointer());
            Marshal::FreeHGlobal(p);
            return false;
        }
        catch(System::Exception^ e)
        {
            System::IntPtr p = Marshal::StringToHGlobalAnsi(e->Message);
            err = static_cast<char*>(p.ToPointer());
            Marshal::FreeHGlobal(p);
            return true;
        }
    }


};

tsqlftwHelper* tsqlftwHelper::New()
{
    return new tsqlftwWrapper();
}

