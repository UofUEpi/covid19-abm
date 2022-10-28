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

// Simulation function for the FMCMC
epiworld::Model<> model;

// RT number in the form of {(Rt, length)} data stored by
// column.
std::vector< size_t > ndays = {10, 25, 50, 75, 100};
std::vector< double > Rts = {2.5, 2, 1.5, 1, .5};

std::vector< double > simfun(
    std::vector< epiworld_double > params,
    LFMCMC<std::vector<double>> * m
) {


    // Mapping the parameters: params is normal but needs to be between 0 and 1.
    model("Prob. of Recovery")     = 1.0/(1.0 + std::exp(-params[0u]));
    model("Infectiousness")        = 1.0/(1.0 + std::exp(-params[1u]));
    model("Death prob.")           = 1.0/(1.0 + std::exp(-params[2u]));
    model("Hospitalization prob.") = 1.0/(1.0 + std::exp(-params[3u]));
    model("Incubation period")     = 1.0/(1.0 + std::exp(-params[4u]));

    model.reset();
    model.run();
           
    std::vector< double > res(model.get_ndays(), 0.0);
    std::vector< double > res_sum(Rts.size(), 0.0);
    std::vector< double > counts(res);

    // variant source source_exposure_date rt
    auto repnum = model.get_db().reproductive_number();
    for (auto r : repnum) 
    {
        // Adding to the sum
        res[r.first.at(2)] += r.second;
        counts[r.first.at(2)] += 1.0;
    }

    for (size_t i = 0u; i < res.size(); ++i)
        if (counts[i] > .1)
        {
            res[i] /= counts[i];
        } else
            res[i] = -1.0;

    // Summarizing now based on the vector
    int j = 0;
    int days_in_count = 0;
    for (size_t i = 0u; i < res.size(); ++i) 
    {
        // Switching to the next bracket
        if (i >= ndays[j])
        {

            if (days_in_count > 0)
                res_sum[j] /= days_in_count;

            days_in_count = 0;

            printf_epiworld("%.4f, ", res_sum[j]);

            j++;

        }

        // Adding the observed Rt to the current date
        if (res[i] > -.000000001)
        {
            res_sum[j] += res[i];
            days_in_count++;
        }
        
    }

    if (days_in_count > 0)
        res_sum[j] /= days_in_count; // One last calc

    printf_epiworld("%.4f, ", res_sum[j]);
    printf_epiworld("\n");

    return res_sum;

}


void sumfun(
    std::vector< epiworld_double > & res,
    const std::vector< double > & dat,
    LFMCMC< std::vector<double> > * m
) {

    if (res.size() == 0u)
        res.resize(dat.size());

    
    for (size_t i = 0u; i < dat.size(); ++i)
        res[i] = static_cast< epiworld_double >(dat[i]);

    return;

}

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
    epiworld_fast_uint nties       = 40;

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
    model.add_status("Susceptible", epiworld::default_update_susceptible<>);
    model.add_status("Exposed", update_exposed_rt);
    model.add_status("Infected", update_infected_rt);
    model.add_status("Hospitalized", update_hospitalized_rt);
    model.add_status("Recovered");
    model.add_status("Deseased");

    model.add_param(1.0/7.0, "Incubation period");
    model.add_param(.1, "Hospitalization prob.");
    model.add_param(.3, "Death prob.");
    model.add_param(.9, "Infectiousness");
    model.add_param(.3, "Prob. of Recovery");

    // Creating the virus
    epiworld::Virus<> covid19("Covid19");
    covid19.set_prob_infecting(&model("Infectiousness"));
    covid19.set_prob_recovery(&model("Prob. of Recovery"));
    covid19.set_status(S::Exposed, S::Recovered);
    covid19.set_queue(epiworld::QueueValues::OnlySelf, -99LL);

    model.add_virus_n(covid19, preval);
    
    // Adding the population
    model.agents_smallworld(popsize, nties, false, .01);
    model.init(ndays, 2312);

    // Creating FMCMC model
    LFMCMC< std::vector< double > > lfmcmc;

    lfmcmc.set_observed_data(Rts);
    lfmcmc.set_simulation_fun(simfun);
    lfmcmc.set_summary_fun(sumfun);
    lfmcmc.set_kernel_fun(kernel_fun_gaussian<std::vector<double>>);

    model.set_backup();
    model.verbose_off();

    std::vector< epiworld_double > par0 = {.5, 5, .5, .5, .5};
    lfmcmc.run(par0, 500, .25);
    
    lfmcmc.set_par_names(
        {"Prob. of Recovery",
        "Infectiousness",
        "Death prob.",
        "Hospitalization prob.",
        "Incubation period"
        }
        );

    // lfmcmc.set_stats_names(model.get_status());

    lfmcmc.print();

    // // Adding multi-file write
    // auto sav = epiworld::make_save_run<int>(
    //     "saves/main_out_%04l", // std::string fmt,
    //     true,  // bool total_hist,
    //     false, // bool variant_info,
    //     false, // bool variant_hist,
    //     false, // bool tool_info,
    //     false, // bool tool_hist,
    //     false, // bool transmission,
    //     false, // bool transition,
    //     true   // bool reproductive
    // );

    // model.run_multiple(100, sav);
    // model.print();

    // // Getting the reproductive numbers
    // model.get_db().write_data(
    //     "", "", "", "", "total_hist.txt", "", "", "repnumber.txt"
    // );

    return 0;
}