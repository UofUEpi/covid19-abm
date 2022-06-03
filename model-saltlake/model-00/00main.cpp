// #define EPI_DEBUG
#include "../../epiworld.hpp"

enum Status {
    Susceptible,
    Exposed,
    InfectedSymp,
    InfectedAsymp,
    Hospitalized,
    Recovered,
    Removed
};

using namespace epiworld;

// Exposed individuals become infected by day 7
EPI_NEW_UPDATEFUN(update_exposed, int)
{

    auto virus = p->get_virus(0u);
    int days_since = m->today() - virus->get_date();
    
    // If no days have passed, then sample the
    // number of days needed
    if (days_since <= 1)
        virus->get_data()[0u] = m->rgamma(7, 1);
    else if (days_since >= virus->get_data()[0u])
        p->change_status(Status::InfectedSymp);

}

EPI_NEW_UPDATEFUN(update_infected_symp, int)
{
    auto v = p->get_virus(0);

    m->array_double_tmp[0u] = v->get_prob_recovery() * p->get_recovery_enhancer(v);
    m->array_double_tmp[0u] = m->par("Prob. Hospitalization");

    int which = roulette(2, m);

    if (which < 0)
        return;

    if (which == 0u)
        p->rm_virus(v);
    else
        p->change_status(Status::Hospitalized);
        

    return;


}

EPI_NEW_UPDATEFUN(update_hospitalized, int)
{

    auto virus = p->get_virus(0u);
    
    // Evaluating probabilities
    m->array_double_tmp[0u] = virus->get_prob_death() * p->get_death_reduction(virus);
    m->array_double_tmp[1u] = virus->get_prob_recovery() * p->get_recovery_enhancer(virus);

    int which = roulette(2, m);

    if (which < 0)
        return;

    if (which == 0u)
    {
        p->rm_virus(virus);
        p->change_status(Status::Removed);
    }
    else
        p->rm_virus(virus);

}

int main()
{

    int nreplicates = 1000;

    // Baseline Configuration
    Model<> model;
    model.add_status("Susceptible", default_update_susceptible<>);
    model.add_status("Exposed", update_exposed);
    model.add_status("Infected Asymptomatic", default_update_exposed<>);
    model.add_status("Infected Symptomatic", update_infected_symp);
    model.add_status("Hospitalized", update_hospitalized);
    model.add_status("Recovered");
    model.add_status("Removed");

    // Reading in the population
    model.agents_from_adjlist("../data/population.txt", 10000, 0, false);
    model.write_edgelist("../data/population-model-written.txt");

    // Setting up the parameters
    model.add_param(.9, "Prob. Infecting");
    model.add_param(1.0/7.0, "Prob. Recovery");
    model.add_param(.01, "Prob. death");
    model.add_param(.05, "Prob. Hospitalization");

    Virus<> omicron("Omicron");
    omicron.set_status(1, 3, 4);

    omicron.get_data().resize(1u);

    omicron.set_prob_infecting(&model("Prob. Infecting"));
    omicron.set_prob_recovery(&model("Prob. Recovery"));
    omicron.set_prob_death(&model("Prob. death"));

    model.add_virus(omicron, .05);

    model.init(100, 223); 

    // Running multiple simulations. The results will be stored in the folder
    // "results/", with each replicate named "0000_total_hist.csv"
    model.run_multiple(
        nreplicates, 
        save_run<>("results/%04lu")
        );

    model.get_db().reproductive_number("reproductive_number.txt");

    // Printing the results
    model.print();

}