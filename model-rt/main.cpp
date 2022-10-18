#include "../epiworld.hpp"

enum S {
    Susceptible,
    Exposed,
    Infected,
    Recovered
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
};


int main(int argc, char* argv[]) {

    // Getting the parameters --------------------------------------------------
    epiworld_fast_uint ndays       = 100;
    epiworld_fast_uint popsize     = 10000;
    epiworld_fast_uint preval      = 10;

    if (argc == 4)
    {
        ndays    = strtol(argv[1], nullptr, 0);
        popsize  = strtol(argv[2], nullptr, 0);
        preval   = strtol(argv[3], nullptr, 0);
    }
    else if (argc != 1)
        std::logic_error("Either no arguments or three (ndays, popsize, preval.)");

    // Setting up the model ----------------------------------------------------
    epiworld::Model<> model;

    model.add_status("Susceptible", epiworld::default_update_susceptible<>);
    model.add_status("Exposed", update_exposed_rt);
    model.add_status("Infected", epiworld::default_update_exposed<>);
    model.add_status("Removed");

    model.add_param(1.0/7.0, "Incubation period");

    // Creating the virus
    epiworld::Virus<> covid19("Covid19");
    covid19.set_prob_infecting(1.0);
    covid19.set_prob_recovery(1.0/5.0);
    covid19.set_status(S::Exposed, S::Recovered);
    covid19.set_queue(epiworld::QueueValues::OnlySelf, -99LL);

    model.add_virus_n(covid19, preval);
    // Adding the population
    model.agents_smallworld(popsize, 20, false, .01);

    model.init(ndays, 2312);
    model.run();
    model.print();

    // Getting the reproductive numbers
    model.get_db().write_data(
        "", "", "", "", "total_hist.txt", "", "", "repnumber.txt"
    );

    return 0;
}