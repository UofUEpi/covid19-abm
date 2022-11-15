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

/**
 * @brief Transmission by contact outside home
 */
EPI_NEW_GLOBALFUN(contact, int)
{
    for (auto & a : *(m->get_agents()))
    {
        // Will it get it from the entities?
        if (a.get_status() == S::Susceptible)
        {

            AgentsSample<int> neighbors(a, 5, true);

            int n_viruses = -1;
            for (auto n : neighbors) {
                if (n->get_status() == S::Infected)
                    m->array_virus_tmp[++n_viruses] = &(*n->get_virus(0u));
            }

            // Nothing to see here
            if (n_viruses == -1)
                continue;

            double p_infection = 1.0 - std::pow(1.0 - *m->p4, n_viruses);

            if (m->runif() >= p_infection)
                continue;

            int which = std::floor(m->runif() * n_viruses);

            a.add_virus(*(m->array_virus_tmp[which]), S::Exposed, QueueValues::OnlySelf); 

        }


    }
}



int main(int argc, char* argv[]) {

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

    // Reading the model parameters
    model.read_params("params.txt");

    // Creating the virus
    epiworld::Virus<> covid19("Covid19");
    covid19.set_prob_infecting(&model("Infectiousness"));
    covid19.set_prob_recovery(&model("Prob. of Recovery"));
    covid19.set_status(S::Exposed, S::Recovered);
    covid19.set_queue(epiworld::QueueValues::OnlySelf, -99LL);

    model.add_virus_n(covid19, model("Prevalence"));
    
    // Adding the population
    model.agents_from_adjlist(
        epiworld::rgraph_blocked(
            model("Population Size"),
            model("N ties"),
            1,
            model
            )
        );

    model.init(model("Days"), model("Seed"));

    // Adding randomly distributed entities, each one with capacity for entity_capacity
    size_t n_entities = std::ceil(model("Population Size")/model("Entity size"));
    for (size_t r = 0u; r < n_entities; ++r)
    {
        Entity<int> e(std::string("Location ") + std::to_string(r));
        model.add_entity_n(e, model("Entity size"));
    }

    // This will act through the global
    model.add_global_action(contact, -99);
    
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

    model.run_multiple(model("Sim count"), sav);
    model.print();

    return 0;
}