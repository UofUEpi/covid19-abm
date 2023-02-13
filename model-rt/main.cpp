#include <omp.h>

// #define EPI_DEBUG

#include "../epiworld.hpp"

using namespace epiworld;

enum S {
    Susceptible,
    Exposed,
    Infected,
    Hospitalized,
    Recovered,
    Deceased
};

// Update dynamics for exposed individuals
EPI_NEW_UPDATEFUN(update_exposed_rt, int) 
{
    // Checking if the data was assigned
    auto v = p->get_virus(0u);
    if (m->today() == (v->get_date() + 1))
    {

        // Checking the same
        if (v->get_data().size() != 3u)
            v->get_data().resize(3u);

        // Days of incubation
        v->get_data()[0u] = v->get_date() + m->rgamma(
            m->par("Gamma shape (incubation)"),
            m->par("Gamma rate (incubation)")
            );

        // Duration as Infected
        v->get_data()[1u] = v->get_date() + m->rgamma(
            m->par("Gamma shape (infected)"),
            m->par("Gamma rate (infected)")
            );

        // Prob of becoming hospitalized
        v->get_data()[2u] = (
            m->runif() < m->par("Hospitalization prob.")
            ) ? 100.0 : -100.0;


    } else if (m->today() >= v->get_data()[0u])
    {
        p->change_status(m, S::Infected, epiworld::QueueValues::Everyone);
        return;
    }

    return;

}

// Update dynamics for infected individuals
EPI_NEW_UPDATEFUN(update_infected_rt, int)
{

    // Computing probability of recovery
    auto v = p->get_virus(0u);

    if (m->today() < v->get_data()[1u])
        return;

    if (v->get_data()[2u] < 0)
    {
        
        p->rm_virus(v, m, S::Recovered, -epiworld::QueueValues::Everyone);
        return;

    }
    else
    {

        // Individual goes hospitalized
        p->change_status(m, S::Hospitalized, -epiworld::QueueValues::Everyone);
        return;

    }

}

// Update dynamics for hospitalized individuals
EPI_NEW_UPDATEFUN(update_hospitalized_rt, int)
{

    // Computing the recovery probability
    auto v = p->get_virus(0u);
    auto probs = {
        m->par("Prob. hosp. recovers"),
        m->par("Prob. hosp. dies")
        };

    int which = epiworld::roulette(probs, m);

    // Nothing happens
     if (which < 0)
        return;

    if (which == 0) // Then it recovered
    {
        p->rm_virus(v, m, S::Recovered, epiworld::QueueValues::NoOne);
        return;
    }

    // Individual dies
    p->rm_virus(v, m, S::Deceased, epiworld::QueueValues::NoOne);

    return;

}

/**
 * @brief Transmission by contact outside home
 */
EPI_NEW_GLOBALFUN(contact, int)
{
    for (auto & a : (m->get_agents()))
    {
        // Will it get it from the entities?
        if (a.get_status() == S::Susceptible)
        {

            AgentsSample<int> neighbors(m, a, m->par("N interactions"), true);

            int n_viruses = 0;
            for (auto n : neighbors) {
                if (n->get_status() == S::Infected)
                    m->array_virus_tmp[n_viruses++] = &(*n->get_virus(0u));
            }

            // Nothing to see here
            if (n_viruses == 0)
                continue;

            // Is the individual getting the infection?
            double p_infection = 1.0 - std::pow(1.0 - m->par("Infectiousness in entity"), n_viruses);

            if (m->runif() >= p_infection)
                continue;

            // Who infects the individual?
            int which = std::floor(m->runif() * n_viruses);

            a.add_virus(
                *(m->array_virus_tmp[which]), // Viruse.
                m, 
                S::Exposed,                   // New state.
                QueueValues::OnlySelf         // Change on the queue.
                ); 

        }


    }
}



int main(int argc, char* argv[])
{

    // Setting up the model ----------------------------------------------------
    Model<> model;

    model.add_status(
        "Susceptible", 
        epiworld::sampler::make_update_susceptible<int>({S::Exposed, S::Hospitalized})
        );

    model.add_status("Exposed", update_exposed_rt);
    model.add_status("Infected", update_infected_rt);
    model.add_status("Hospitalized", update_hospitalized_rt);
    model.add_status("Recovered");
    model.add_status("Deceased");

    // Reading the model parameters
    model.read_params("params.txt");

    int nthreads = static_cast<int>(model("OMP threads"));
    omp_set_num_threads(nthreads);

    // Creating the virus
    epiworld::Virus<> covid19("Covid19");
    covid19.set_prob_infecting(&model("Infectiousness"));
    covid19.set_status(S::Exposed, S::Recovered);
    covid19.set_queue(epiworld::QueueValues::OnlySelf, -99LL);
    covid19.get_data() = {0.0, 0.0F};

    model.add_virus_n(covid19, model("Prevalence"));
    
    // Adding the population
    // model.agents_from_adjlist(
    //     "population.txt",         // Filepath
    //     model("Population Size"), // Population size
    //     0,                        // Lines to skip
    //     false                     // Directed?
    //     );
    model.agents_smallworld((uint) model("Population Size"), 5, false, .05);
 
    // Adding randomly distributed entities, each one with capacity for entity_capacity
    for (size_t r = 0u; r < model("N entities"); ++r)
    {
        Entity<int> e(std::string("Location ") + std::to_string(r));
        model.add_entity_n(e, 0);
    }

    // Loading the entities
    model.load_agents_entities_ties("agents_entities.txt", 0);

    // This will act through the global
    // model.add_global_action(contact, -99);
    
    
    model.init(model("Days"), model("Seed"));

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

    model.run_multiple(model("Sim count"), sav, true, true, nthreads);
    model.print();

    return 0;
}

