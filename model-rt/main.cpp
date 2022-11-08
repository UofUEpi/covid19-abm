#include "../epiworld.hpp"

using namespace epiworld;

enum S {
    Susceptible,
    Exposed,
    Infected,
    Hospitalized,
    Recovered,
    Deseased
};

// Updating exposed
EPI_NEW_UPDATEFUN(update_exposed_rt, int) 
{
    if (m->runif() < *m->p0)
    {
        p->change_status(S::Infected, epiworld::QueueValues::Everyone);
        return;
    }

    return;
}

// Updating infected
EPI_NEW_UPDATEFUN(update_infected_rt, int)
{

    auto v = p->get_virus(0u);
    auto probs = {
        v->get_prob_recovery(),
        (*m->p1)
        };
    int which = epiworld::roulette(probs, m);

    if (which < 0)
        return;

    if (which == 0) // Then it recovered
    {
        p->rm_virus(v, S::Recovered, -epiworld::QueueValues::Everyone);
        return;
    }

    p->change_status(S::Hospitalized, -epiworld::QueueValues::Everyone);
    return;

}

EPI_NEW_UPDATEFUN(update_hospitalized_rt, int)
{

    auto v = p->get_virus(0u);
    auto probs = {
        v->get_prob_recovery(),
        (*m->p2)
        };
    int which = epiworld::roulette(probs, m);

     if (which < 0)
        return;

    if (which == 0) // Then it recovered
    {
        p->rm_virus(v, S::Recovered, epiworld::QueueValues::NoOne);
        return;
    }

    p->rm_virus(v, S::Deseased, epiworld::QueueValues::NoOne);
    return;

}


int main(int argc, char* argv[]) {

    // Getting the parameters --------------------------------------------------
    epiworld_fast_uint ndays       = 100;
    epiworld_fast_uint popsize     = 10000;
    epiworld_fast_uint preval      = 20;
    epiworld_fast_uint nties       = 100;

    if (argc == 5)
    {
        ndays    = strtol(argv[1], nullptr, 0);
        popsize  = strtol(argv[2], nullptr, 0);
        preval   = strtol(argv[3], nullptr, 0);
        nties    = strtol(argv[4], nullptr, 0);
    }
    else if (argc != 1)
        std::logic_error("Either no arguments or four (ndays, popsize, preval, and nties.)");

    // Setting up the model ----------------------------------------------------
    epiworld::Model<> model;

    model.add_status(
        "Susceptible", 
        epiworld::sampler::make_update_susceptible<int>({S::Exposed, S::Hospitalized})
        );

    model.add_status("Exposed", update_exposed_rt);
    model.add_status("Infected", update_infected_rt);
    model.add_status("Hospitalized", update_hospitalized_rt);
    model.add_status("Recovered");
    model.add_status("Deseased");

    model.add_param(1.0/7.0, "Incubation period");
    model.add_param(.1, "Hospitalization prob.");
    model.add_param(.1, "Death prob.");
    model.add_param(.02, "Infectiousness"); 
    model.add_param(1.0/7.0, "Prob. of Recovery");

    // Creating the virus
    epiworld::Virus<> covid19("Covid19");
    covid19.set_prob_infecting(&model("Infectiousness"));
    covid19.set_prob_recovery(&model("Prob. of Recovery"));
    covid19.set_status(S::Exposed, S::Recovered);
    covid19.set_queue(epiworld::QueueValues::OnlySelf, -99LL);

    model.add_virus_n(covid19, preval);
    
    // Adding the population
    model.agents_smallworld(popsize, nties, false, .3);
    model.init(ndays, 2312);
    
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

    model.run_multiple(100, sav);
    model.print();

    return 0;
}