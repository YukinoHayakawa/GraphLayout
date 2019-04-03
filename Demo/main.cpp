#include <Usagi/Runtime/Runtime.hpp>

// #include "GraphLayoutDemo.hpp"
#include <GraphLayout/Genetic/Crossover.hpp>
// #include <GraphLayout/Genetic/GeneticOptimizer.hpp>
#include <fmt/printf.h>
#include <GraphLayout/Genetic/Mutation.hpp>

using namespace usagi;

int main(int argc, char *argv[])
{
    // std::string a = "aaaaaaaaaaaaaaaa";
    // std::string b = "bbbbbbbbbbbbbbbb";
    //
    // genetic::crossover::OnePointCrossover c;
    // std::random_device rd;
    // std::mt19937 gen { rd() };
    // for(int i = 0; i < 1000; ++i)
    // {
    //     c(a, b, rd);
    //     fmt::print("{} {}\n", a, b);
    // }
    //
    // std::vector<float> aa = { 0, 0, 0, 0, 0 };
    //
    // genetic::mutation::GaussianMutation<decltype(aa)> gm;
    // std::uniform_real_distribution<> dist(0, 1);
    // for(int i = 0; i < 1000; ++i)
    // {
    //     // if(dist(gen) < 0.1)
    //         gm(aa, gen);
    //     fmt::print("{} {} {} {} {}\n", aa[0], aa[1], aa[2], aa[3], aa[4]);
    // }
    //


    // return 0;
    // GeneticOptimizer optimizer;
    // optimizer.graph.nodes.emplace_back();
    // optimizer.graph.nodes.emplace_back();
    // optimizer.graph.nodes.emplace_back();
    // optimizer.graph.nodes.emplace_back();
    // optimizer.graph.nodes.emplace_back();
    // optimizer.graph.links.emplace_back();
    // optimizer.graph.links.back().node0 = 0;
    // optimizer.graph.links.back().node1 = 1;
    // optimizer.graph.links.emplace_back();
    // optimizer.graph.links.back().node0 = 0;
    // optimizer.graph.links.back().node1 = 2;
    // optimizer.graph.links.emplace_back();
    // optimizer.graph.links.back().node0 = 0;
    // optimizer.graph.links.back().node1 = 3;
    // optimizer.graph.links.emplace_back();
    // optimizer.graph.links.back().node0 = 1;
    // optimizer.graph.links.back().node1 = 2;
    // optimizer.graph.links.emplace_back();
    // optimizer.graph.links.back().node0 = 3;
    // optimizer.graph.links.back().node1 = 2;
    // optimizer.graph.links.emplace_back();
    // optimizer.graph.links.back().node0 = 2;
    // optimizer.graph.links.back().node1 = 4;
    // optimizer.graph.links.emplace_back();
    // optimizer.graph.links.back().node0 = 3;
    // optimizer.graph.links.back().node1 = 4;
    // optimizer.optimize();
    //
    // return 0;

    /*auto runtime = Runtime::create();
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
    }*/
}
