#include <node.h>
#include <v8.h>
#include <string>

using namespace v8;

void LoadAssembly();

// this is a shim class which mediates between managed and unmanaged code
class tsqlftwHelper
{
protected:
    tsqlftwHelper() {};
public:
    virtual bool connect(std::string& connString, std::string& err, std::string& result) = 0;
    virtual bool query(std::string& query, std::string& err, std::string& result) = 0;
    virtual bool close(std::string& err, std::string& result) = 0;
	
    static tsqlftwHelper* New();
};
