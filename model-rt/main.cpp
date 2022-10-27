#include "../epiworld.hpp"

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
    epiworld_fast_uint preval      = 10;
    epiworld_fast_uint nties       = 40;

    if (argc == 5)
    {
        ndays    = strtol(argv[1], nullptr, 0);
        popsize  = strtol(argv[2], nullptr, 0);
        preval   = strtol(argv[3], nullptr, 0);
        nties    = strtol(argv[4], nullptr, 0);
    }
    else if (argc != 1)
        std::logic_error("Either no arguments or three (ndays, popsize, preval.)");

    // Setting up the model ----------------------------------------------------
    epiworld::Model<> model;

    model.add_status("Susceptible", epiworld::default_update_susceptible<>);
    model.add_status("Exposed", update_exposed_rt);
    model.add_status("Infected", update_infected_rt);
    model.add_status("Hospitalized", update_hospitalized_rt);
    model.add_status("Recovered");
    model.add_status("Deseased");

    model.add_param(1.0/7.0, "Incubation period");
    model.add_param(.1, "Hospitalization prob.");
    model.add_param(.3, "Death prob.");

    // Creating the virus
    epiworld::Virus<> covid19("Covid19");
    covid19.set_prob_infecting(1.0);
    covid19.set_prob_recovery(1.0/5.0);
    covid19.set_status(S::Exposed, S::Recovered);
    covid19.set_queue(epiworld::QueueValues::OnlySelf, -99LL);

    model.add_virus_n(covid19, preval);
    
    // Adding the population
    model.agents_smallworld(popsize, nties, false, .01);

    // Adding multi-file write
    auto sav = epiworld::make_save_run<int>(
        "saves/main_out_%04l", // std::string fmt,
        true,// bool total_hist,
        false, // bool variant_info,
        false, // bool variant_hist,
        false, // bool tool_info,
        false, // bool tool_hist,
        false, // bool transmission,
        false, // bool transition,
        true// bool reproductive
    );

    model.init(ndays, 2312);
    model.run_multiple(100, sav);
    model.print();

    // Getting the reproductive numbers
    model.get_db().write_data(
        "", "", "", "", "total_hist.txt", "", "", "repnumber.txt"
    );

    return 0;
}