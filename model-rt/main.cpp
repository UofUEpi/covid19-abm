#include <omp.h>

// #define EPI_DEBUG

#include "../epiworld.hpp"

using namespace epiworld;

int main(int argc, char* argv[])
{

    // Setting up the model ----------------------------------------------------
    epimodels::ModelSEIRD<> model("params.txt", "Covid19");

    int nthreads = static_cast<int>(model("OMP threads"));
    #ifdef __OPENMP
    omp_set_num_threads(nthreads);
    #else
    nthreads = 1;
    #endif

    // Adding the population
    model.agents_from_adjlist(
         "population.txt",         // Filepath
         model("Population Size"), // Population size
         0,                        // Lines to skip
         false                     // Directed?
         );

    // Loading the entities
    model.load_agents_entities_ties("agents_entities.txt", 0);

    // Adding multi-file write
    auto sav = epiworld::make_save_run<int>(
        "saves/main_out_%04li", // std::string fmt,
        true,  // bool total_hist,
        false, // bool variant_info,
        false, // bool variant_hist,
        false, // bool tool_info,
        false, // bool tool_hist,
        true , // bool transmission,
        false, // bool transition,
        true   // bool reproductive
    );

    model.run_multiple(model("Days"), model("Sim count"), model("Seed"), sav, true, true, nthreads);

    model.print();

    return 0;

}

