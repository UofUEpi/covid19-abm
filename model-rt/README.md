
# Description of the model

This is a variation of a SEIRD model, which includes hospitalizations.
The specific features follow:

  - Two types of network connections: families and bi-partite through
    entities.
  - Individuals are clustered in groups of five.
  - Each entity houses 100 individuals.
  - Transmission can happen between family members or between entity
    members.
  - At each step, the model draws 5 entity members per susceptible
    individual. This represents the chance of direct contact.
  - Only infected non-hospitalized individuals can transmit the disease.

The file [`params.txt`](params.txt) contains the model parameters. The
current values are:

| Parameter                |   Value |
| :----------------------- | ------: |
| Gamma shape (incubation) |     7.0 |
| Gamma rate (incubation)  |     1.0 |
| Gamma shape (infected)   |     7.0 |
| Gamma rate (infected)    |     1.0 |
| Hospitalization prob.    |     0.1 |
| Prob. hosp. recovers     |     0.5 |
| Prob. hosp. dies         |     0.5 |
| Infectiousness           |     0.8 |
| Infectiousness in entity |     0.1 |
| Days                     |    50.0 |
| Population Size          | 10000.0 |
| Prevalence               |   100.0 |
| N ties                   |     5.0 |
| Sim count                |    50.0 |
| N entities               |   200.0 |
| Seed                     |  1545.0 |
| N interactions           |    20.0 |
| OMP threads              |     4.0 |

The full program can be found in the file [main.cpp](main.cpp).

# Running the model

``` bash
./main.o
```

    ## Starting multiple runs (50) using 4 thread(s)
    ## _________________________________________________________________________
    ## _________________________________________________________________________
    ## ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| done.
    ##  done.
    ## 
    ## ________________________________________________________________________________
    ## SIMULATION STUDY
    ## 
    ## Name of the model   : (none)
    ## Population size     : 10000
    ## Number of entitites : 200
    ## Days (duration)     : 51 (of 50)
    ## Number of variants  : 1
    ## Last run elapsed t  : 0.00s
    ## Total elapsed t     : 2.00s (50 runs)
    ## Last run speed      : 2.18 million agents x day / second
    ## Average run speed   : 9.33 million agents x day / second
    ## Rewiring            : off
    ## 
    ## Virus(es):
    ##  - Covid19 (baseline prevalence: 100 seeds)
    ## 
    ## Tool(s):
    ##  (none)
    ## 
    ## Model parameters:
    ##  - Days                     : 50.0000
    ##  - Gamma rate (incubation)  : 1.0000
    ##  - Gamma rate (infected)    : 1.0000
    ##  - Gamma shape (incubation) : 7.0000
    ##  - Gamma shape (infected)   : 7.0000
    ##  - Hospitalization prob.    : 0.1000
    ##  - Infectiousness           : 0.8000
    ##  - Infectiousness in entity : 0.1000
    ##  - N entities               : 200.0000
    ##  - N interactions           : 20.0000
    ##  - N ties                   : 5.0000
    ##  - OMP threads              : 4.0000
    ##  - Population Size          : 10000.0000
    ##  - Prevalence               : 100.0000
    ##  - Prob. hosp. dies         : 0.5000
    ##  - Prob. hosp. recovers     : 0.5000
    ##  - Seed                     : 1545.0000
    ##  - Sim count                : 50.0000
    ## 
    ## Distribution of the population at time 51:
    ##  - (0) Susceptible  :  9900 -> 0
    ##  - (1) Exposed      :   100 -> 14
    ##  - (2) Infected     :     0 -> 24
    ##  - (3) Hospitalized :     0 -> 3
    ##  - (4) Recovered    :     0 -> 9470
    ##  - (5) Deceased     :     0 -> 489
    ## 
    ## Transition Probabilities:
    ##  - Susceptible   0.83  0.17  0.00  0.00  0.00  0.00
    ##  - Exposed       0.00  0.85  0.15  0.00  0.00  0.00
    ##  - Infected      0.00  0.00  0.54  0.05  0.41  0.00
    ##  - Hospitalized  0.00  0.00  0.00  0.33  0.36  0.31
    ##  - Recovered     0.00  0.00  0.00  0.00  1.00  0.00
    ##  - Deceased      0.00  0.00  0.00  0.00  0.00  1.00

# Computing reproductive number

``` r
rt <- list.files("saves", pattern = "reproductive", full.names = TRUE)
rt <- lapply(seq_along(rt), \(i) {cbind(id = i, fread(rt[i]))}) |>
    rbindlist()

# Computing for each individual
rt <- rt[, .(rt = mean(rt)), by = c("id", "source_exposure_date", "thread")]
setorder(rt, source_exposure_date)

ggplot(rt, aes(x = source_exposure_date, y = rt)) +
    geom_jitter(alpha = .1) +
    geom_smooth(method = "loess", se = TRUE)
```

    ## `geom_smooth()` using formula = 'y ~ x'

![](README_files/figure-gfm/repnum-1.png)<!-- -->

``` r
setorder(rt, id, source_exposure_date, rt)
fwrite(rt, "reproductive_numbers.csv")
```

# Epi curves

``` r
epicurves <- list.files("saves", pattern = "hist", full.names = TRUE)
epicurves <- lapply(seq_along(epicurves), \(i) {
    cbind(id = i, fread(epicurves[i]))
}) |> rbindlist()

fwrite(epicurves, "epicurves.csv")

# Samlping
epicurves[, pick := order(runif(.N)), by = .(date, nvariants)]

epicurves_sample <- epicurves[pick <= 200]

epicurves_sample[status %in% c("Exposed", "Infected", "Hospitalized")] |>
    ggplot(aes(x = date, y = counts)) +
    geom_jitter(aes(colour = status), alpha = .1)
```

![](README_files/figure-gfm/transitions-1.png)<!-- -->

``` r
epicurves_sample[!status %in% c("Exposed", "Infected", "Hospitalized")] |>
    ggplot(aes(x = date, y = counts)) +
    # geom_smooth(aes(colour = status), method = "loess", se = TRUE) +
    geom_jitter(aes(colour = status), alpha = .1)
```

![](README_files/figure-gfm/totals-1.png)<!-- -->

Status at the end of the simulation

``` r
epicurves_end <- epicurves[date == max(date)]
epicurves_end[, .(
    Avg     = mean(counts),
    `50%`   = quantile(counts, probs = .5),
    `2.5%`  = quantile(counts, probs = .025),
    `97.5%` = quantile(counts, probs = .975)
    ), by = "status"] |> knitr::kable()
```

| status       |  Avg |  50% | 2.5% | 97.5% |
| :----------- | ---: | ---: | ---: | ----: |
| Susceptible  |    0 |    0 |    0 |     0 |
| Exposed      |   14 |   14 |   14 |    14 |
| Infected     |   24 |   24 |   24 |    24 |
| Hospitalized |    3 |    3 |    3 |     3 |
| Recovered    | 9470 | 9470 | 9470 |  9470 |
| Deceased     |  489 |  489 |  489 |   489 |
