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

    if (days_since >= virus->get_data()[0u])
    {

        if (m->par("Prob. Dev. Symptoms") > m->runif())
            p->change_status(Status::InfectedSymp);
        else
            p->change_status(Status::InfectedAsymp);

    }

}

EPI_NEW_UPDATEFUN(update_infected_symp, int)
{
    auto v = p->get_virus(0);

    m->array_double_tmp[0u] = v->get_prob_recovery() * p->get_recovery_enhancer(v);

    if (p->has_tool("Vaccine"))
        m->array_double_tmp[1u] = m->par("Vax Prob. Hospitalization");
    else
        m->array_double_tmp[1u] = m->par("Prob. Hospitalization");

    int which = roulette(2, m);

    if (which < 0)
        return;

    if (which == 0u)
        p->rm_virus(v, Status::Recovered);
    else
        p->change_status(Status::Hospitalized);
        

    return;


}

EPI_NEW_UPDATEFUN(update_infected_asymp, int)
{
    auto v = p->get_virus(0);

    epiworld_double prec = 1.0 - 
        (1.0 - v->get_prob_recovery()) * 
        (1.0 - p->get_recovery_enhancer(v));

    if (m->runif() < prec)
        p->rm_virus(v, Status::Recovered);
    
    return;

}



EPI_NEW_UPDATEFUN(update_hospitalized, int)
{

    auto virus = p->get_virus(0u);
    
    // Evaluating probabilities
    m->array_double_tmp[0u] =
        m->par("Prob. death") *
        (1.0 - p->get_death_reduction(virus));

    m->array_double_tmp[1u] = 1.0 - 
        (1.0 - virus->get_prob_recovery()) * 
        (1.0 - p->get_recovery_enhancer(virus));

    int which = roulette(2, m);

    if (which < 0)
        return;

    if (which == 0u)
        p->rm_virus(virus, Status::Removed);
    else
        p->rm_virus(virus, Status::Recovered);

}

// Vaccine efficacy decays through time
EPI_NEW_TOOL(vax_efficacy, int)
{

    epiworld_double days = m->today() - t.get_date();
    return 
        m->par("Vax Efficacy") *
            std::pow(1/days, m->par("Vax Efficacy decay"));
}

// Vaccine improved recovery decays also
EPI_NEW_TOOL(vax_recovery, int)
{
    epiworld_double days = m->today() - t.get_date();
    return 
        m->par("Vax Recovery enhance") *
            std::pow(1/days, m->par("Vax Efficacy decay"));
    
}

// Vaccine and so does dying
EPI_NEW_TOOL(vax_death, int)
{
    epiworld_double days = m->today() - t.get_date();
    return 
        m->par("Vax Death redux") *
            std::pow(1/days, m->par("Vax Efficacy decay"));
}

int main()
{

    int nreplicates = 1000;

    // Baseline Configuration
    Model<> model;
    model.add_status("Susceptible", default_update_susceptible<>);
    model.add_status("Exposed", update_exposed);
    model.add_status("Infected Symptomatic", update_infected_symp);
    model.add_status("Infected Asymptomatic", update_infected_asymp);
    model.add_status("Hospitalized", update_hospitalized);
    model.add_status("Recovered");
    model.add_status("Removed");

    // Reading in the population
    model.agents_from_adjlist("../data/population.txt", 10000, 0, false);
    model.write_edgelist("../data/population-model-written.txt");

    // Setting up the parameters
    model.add_param(.9, "Prob. Infecting");
    model.add_param(.7, "Prob. Dev. Symptoms");
    model.add_param(1.0/7.0, "Prob. Recovery");
    model.add_param(.05, "Prob. Hospitalization");
    model.add_param(.30, "Prob. death");
    model.add_param(.8, "Mask redux transmission");
    model.add_param(.9, "Vax Efficacy");
    model.add_param(.5, "Vax Efficacy decay");
    model.add_param(.95, "Vax Death redux");
    model.add_param(.5, "Vax Recovery enhance");
    model.add_param(.01, "Vax Prob. Hospitalization");

    // Designing virus
    Virus<> omicron("Omicron");
    omicron.set_status(Status::Exposed, Status::Recovered, Status::Removed);
    omicron.get_data().resize(1u);
    omicron.set_prob_infecting(&model("Prob. Infecting"));
    omicron.set_prob_recovery(&model("Prob. Recovery"));
    omicron.set_prob_death(&model("Prob. death"));
    model.add_virus(omicron, .05);

    // Designing Mask wearing
    Tool<> mask("Mask");
    mask.set_transmission_reduction(&model("Mask redux transmission"));
    model.add_tool(mask, .1);

    // Designing Vaccine
    Tool<> vax("Vaccine");
    vax.set_susceptibility_reduction_fun(vax_efficacy);
    vax.set_recovery_enhancer_fun(vax_recovery);
    vax.set_death_reduction_fun(vax_death);
    
    model.add_tool(vax, .6);

    model.init(60, 223); 

    // Running multiple simulations. The results will be stored in the folder
    // "results/", with each replicate named "0000_total_hist.csv"
    model.run_multiple(
        nreplicates, 
        save_run<>("results/%04lu",
            true,  // History
            false,
            false,
            false,
            false,
            false,
            true   // Transitions
            )
        );

    model.get_db().reproductive_number("reproductive_number.txt");

    // Printing the results
    model.print();

    // Printing transition probabilities
    auto tprob = model.get_db().transition_probability(true);
    
    return 0;

}