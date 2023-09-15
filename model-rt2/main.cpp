#include <omp.h>

// #define EPI_DEBUG

#include "../epiworld.hpp"

using namespace epiworld;

int main(int argc, char* argv[])
{

    // Setting up the model ----------------------------------------------------
    epimodels::ModelSEIRCONN<> model(
        "model-rt2",
        2e4,    // Population size
        0.001,  // Initial infected fraction
        2.0,    // Contact Rate
        0.3,    // Transmission prob
        7.0,    // Avg incubation
        1.0/7.0 // Recovery rate
        );

    omp_set_num_threads(4);
    
    // Adding multi-file write
    auto sav = epiworld::make_save_run<int>(
        "saves/main_out_%04li", // std::string fmt,
        true,  // bool total_hist,
        false, // bool variant_info,
        false, // bool variant_hist,
        false, // bool tool_info,
        false, // bool tool_hist,
        true , // bool transmission,
        true,  // bool transition,
        true,  // bool reproductive,
        true   // bool generation time
    );

    model.run_multiple(
        100,
        200,
        200,
        sav,
        true,
        true, 
        4
        );

    model.print();

    return 0;

}


