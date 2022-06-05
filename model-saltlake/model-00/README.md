
# Model 0 for Salt Lake City

This simulation model features the following:

1.  Five different states: Susceptible, Exposed, Infected, Recovered,
    and Removed.

2.  The population size is 10,000 individuals, all connected at the
    household level, and between households.

3.  A baseline probability of infection is .9.

4.  Daily recovery rate for an infected agent is `1/7`.

5.  Daily death rate for an infected agent is .01.

6.  There is one virus that starts with 5% of the population exposed.

7.  The simulation lasts 100 days, and replicated a thousand times.

The implementation details can be found in [00main.cpp](00main.cpp).
After compilation, the execution of the program results in the following
output:

``` bash
./00main.o
```

    ## Starting multiple runs (1000)
    ## _________________________________________________________________________
    ## _________________________________________________________________________
    ## ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| done.
    ##  done.
    ## 
    ## ________________________________________________________________________________
    ## SIMULATION STUDY
    ## 
    ## Population size    : 10000
    ## Days (duration)    : 60 (of 60)
    ## Number of variants : 1
    ## Last run elapsed t : 0.00s
    ## Total elapsed t    : 21.00s (1000 runs)
    ## Rewiring           : off
    ## 
    ## Virus(es):
    ##  - Omicron (baseline prevalence: 5.00%)
    ## 
    ## Tool(s):
    ##  - Mask (baseline prevalence: 10.00%)
    ##  - Vaccine (baseline prevalence: 60.00%)
    ## 
    ## Model parameters:
    ##  - Mask redux transmission   : 0.8000
    ##  - Prob. Dev. Symptoms       : 0.7000
    ##  - Prob. Hospitalization     : 0.0500
    ##  - Prob. Infecting           : 0.9000
    ##  - Prob. Recovery            : 0.1429
    ##  - Prob. death               : 0.3000
    ##  - Vax Death redux           : 0.9500
    ##  - Vax Efficacy              : 0.9000
    ##  - Vax Efficacy decay        : 0.5000
    ##  - Vax Prob. Hospitalization : 0.0100
    ##  - Vax Recovery enhance      : 0.5000
    ## 
    ## Distribution of the population at time 60:
    ##  - (0) Susceptible           :  9500 -> 8115
    ##  - (1) Exposed               :   500 -> 0
    ##  - (2) Infected Symptomatic  :     0 -> 268
    ##  - (3) Infected Asymptomatic :     0 -> 0
    ##  - (4) Hospitalized          :     0 -> 15
    ##  - (5) Recovered             :     0 -> 1153
    ##  - (6) Removed               :     0 -> 449
    ## 
    ## Transition Probabilities:
    ##  -           Susceptible  1.00  0.00  0.00  0.00  0.00  0.00  0.00
    ##  -               Exposed  0.00  0.71  0.18  0.11  0.00  0.00  0.00
    ##  -  Infected Symptomatic  0.00  0.00  0.97  0.00  0.02  0.01  0.00
    ##  - Infected Asymptomatic  0.00  0.00  0.00  0.82  0.00  0.18  0.00
    ##  -          Hospitalized  0.00  0.00  0.00  0.00  0.63  0.12  0.25
    ##  -             Recovered  0.00  0.00  0.00  0.00  0.00  1.00  0.00
    ##  -               Removed  0.00  0.00  0.00  0.00  0.00  0.00  1.00

Statuses’ trajectories have the following distribution:

![](README_files/figure-gfm/posterior-dist-1.svg)<!-- -->
