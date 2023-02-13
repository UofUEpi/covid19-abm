
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

| Parameter                |    Value |
| :----------------------- | -------: |
| Gamma shape (incubation) |     7.00 |
| Gamma rate (incubation)  |     1.00 |
| Gamma shape (infected)   |     7.00 |
| Gamma rate (infected)    |     1.00 |
| Hospitalization prob.    |     0.10 |
| Prob. hosp. recovers     |     0.50 |
| Prob. hosp. dies         |     0.50 |
| Infectiousness           |     0.90 |
| Infectiousness in entity |     0.25 |
| Days                     |   100.00 |
| Population Size          | 10000.00 |
| Prevalence               |   100.00 |
| N ties                   |     5.00 |
| Sim count                |   100.00 |
| N entities               |   100.00 |
| Seed                     |  1545.00 |
| N interactions           |     5.00 |
| OMP threads              |     4.00 |

The full program can be found in the file [main.cpp](main.cpp).

# Running the model

``` bash
./main.o
```

    ## _________________________________________________________________________
    ## Starting multiple runs (100) using 4 thread(s)
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
    ## Number of entitites : 100
    ## Days (duration)     : 101 (of 100)
    ## Number of variants  : 1
    ## Last run elapsed t  : 8.00ms
    ## Total elapsed t     : 289.00ms (100 runs)
    ## Last run speed      : 117.61 million agents x day / second
    ## Average run speed   : 344.89 million agents x day / second
    ## Rewiring            : off
    ## 
    ## Virus(es):
    ##  - Covid19 (baseline prevalence: 100 seeds)
    ## 
    ## Tool(s):
    ##  (none)
    ## 
    ## Model parameters:
    ##  - Days                     : 100.0000
    ##  - Gamma rate (incubation)  : 1.0000
    ##  - Gamma rate (infected)    : 1.0000
    ##  - Gamma shape (incubation) : 7.0000
    ##  - Gamma shape (infected)   : 7.0000
    ##  - Hospitalization prob.    : 0.1000
    ##  - Infectiousness           : 0.9000
    ##  - Infectiousness in entity : 0.2500
    ##  - N entities               : 100.0000
    ##  - N interactions           : 5.0000
    ##  - N ties                   : 5.0000
    ##  - OMP threads              : 4.0000
    ##  - Population Size          : 10000.0000
    ##  - Prevalence               : 100.0000
    ##  - Prob. hosp. dies         : 0.5000
    ##  - Prob. hosp. recovers     : 0.5000
    ##  - Seed                     : 1545.0000
    ##  - Sim count                : 100.0000
    ## 
    ## Distribution of the population at time 101:
    ##  - (0) Susceptible  :  9900 -> 6129
    ##  - (1) Exposed      :   100 -> 279
    ##  - (2) Infected     :     0 -> 48
    ##  - (3) Hospitalized :     0 -> 6
    ##  - (4) Recovered    :     0 -> 3344
    ##  - (5) Deceased     :     0 -> 194
    ## 
    ## Transition Probabilities:
    ##  - Susceptible   1.00  0.00  0.00  0.00  0.00  0.00
    ##  - Exposed       0.00  0.87  0.13  0.00  0.00  0.00
    ##  - Infected      0.00  0.00  0.52  0.05  0.42  0.00
    ##  - Hospitalized  0.00  0.00  0.00  0.31  0.37  0.31
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
    geom_jitter(aes(colour = as.factor(thread == 0)), alpha = .1) +
    geom_smooth() +
    lims(y = c(0, 5))
```

    ## `geom_smooth()` using method = 'gam' and formula = 'y ~ s(x, bs = "cs")'

    ## Warning: Removed 181 rows containing missing values (`geom_point()`).

    ## Warning: Removed 2 rows containing missing values (`geom_smooth()`).

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

epicurves[status %in% c("Exposed", "Infected", "Hospitalized")] |>
    ggplot(aes(x = date, y = counts, shape = as.factor(thread == 0))) +
    geom_smooth(aes(colour = status)) +
    geom_jitter(aes(colour = status), alpha = .1)
```

    ## `geom_smooth()` using method = 'gam' and formula = 'y ~ s(x, bs = "cs")'

![](README_files/figure-gfm/transitions-1.png)<!-- -->

``` r
epicurves[!status %in% c("Exposed", "Infected", "Hospitalized")] |>
    ggplot(aes(x = date, y = counts)) +
    geom_smooth(aes(colour = status)) +
    geom_jitter(aes(colour = status), alpha = .1)
```

    ## `geom_smooth()` using method = 'gam' and formula = 'y ~ s(x, bs = "cs")'

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

| status       |         Avg |  50% | 2.5% | 97.5% |
| :----------- | ----------: | ---: | ---: | ----: |
| Susceptible  | 6128.556154 | 6133 | 5833 |  6408 |
| Exposed      |  286.474615 |  289 |  245 |   317 |
| Infected     |   61.707692 |   61 |   45 |    79 |
| Hospitalized |    6.938461 |    6 |    3 |    16 |
| Recovered    | 3332.380769 | 3340 | 3083 |  3565 |
| Deceased     |  183.942308 |  183 |  152 |   207 |
