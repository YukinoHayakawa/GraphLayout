#include <Usagi/Runtime/Runtime.hpp>

#include "GraphLayoutDemo.hpp"
#include <GraphLayout/Genetic/GeneticOptimizer.hpp>

using namespace usagi;

int main(int argc, char *argv[])
{
    GeneticOptimizer optimizer;
    optimizer.graph.nodes.emplace_back();
    optimizer.graph.nodes.emplace_back();
    optimizer.graph.nodes.emplace_back();
    optimizer.graph.nodes.emplace_back();
    optimizer.graph.nodes.emplace_back();
    optimizer.graph.links.emplace_back();
    optimizer.graph.links.back().node0 = 0;
    optimizer.graph.links.back().node1 = 1;
    optimizer.graph.links.emplace_back();
    optimizer.graph.links.back().node0 = 0;
    optimizer.graph.links.back().node1 = 2;
    optimizer.graph.links.emplace_back();
    optimizer.graph.links.back().node0 = 0;
    optimizer.graph.links.back().node1 = 3;
    optimizer.graph.links.emplace_back();
    optimizer.graph.links.back().node0 = 1;
    optimizer.graph.links.back().node1 = 2;
    optimizer.graph.links.emplace_back();
    optimizer.graph.links.back().node0 = 3;
    optimizer.graph.links.back().node1 = 2;
    optimizer.graph.links.emplace_back();
    optimizer.graph.links.back().node0 = 2;
    optimizer.graph.links.back().node1 = 4;
    optimizer.graph.links.emplace_back();
    optimizer.graph.links.back().node0 = 3;
    optimizer.graph.links.back().node1 = 4;
    optimizer.optimize();

    return 0;

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
