#include <Usagi/Runtime/Runtime.hpp>

#include "GraphLayoutDemo.hpp"

using namespace usagi;

int main(int argc, char *argv[])
{
    auto runtime = Runtime::create();
    runtime->enableCrashHandler("GraphLayoutDemoCrashDump");
    try
    {
        GraphLayoutDemo demo(runtime);
        demo.mainLoop();
    }
    catch(const std::exception &e)
    {
        runtime->displayErrorDialog(e.what());
        throw;
    }
}
