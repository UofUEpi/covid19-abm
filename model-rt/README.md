
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
| Infectiousness           |     0.50 |
| Infectiousness in entity |     0.25 |
| Days                     |   100.00 |
| Population Size          | 10000.00 |
| Prevalence               |    10.00 |
| N ties                   |     5.00 |
| Sim count                |    50.00 |
| N entities               |   100.00 |
| Seed                     |  1545.00 |
| N interactions           |     5.00 |

The full program can be found in the file [main.cpp](main.cpp).

# Running the model

``` bash
./main.o
```

    ## Starting multiple runs (50)
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
    ## Days (duration)     : 100 (of 100)
    ## Number of variants  : 1
    ## Last run elapsed t  : 0.00s
    ## Total elapsed t     : 8.00s (50 runs)
    ## Last run speed      : 4.12 million agents x day / second
    ## Average run speed   : 5.73 million agents x day / second
    ## Rewiring            : off
    ## 
    ## Virus(es):
    ##  - Covid19 (baseline prevalence: 10 seeds)
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
    ##  - Infectiousness           : 0.5000
    ##  - Infectiousness in entity : 0.2500
    ##  - N entities               : 100.0000
    ##  - N interactions           : 5.0000
    ##  - N ties                   : 5.0000
    ##  - Population Size          : 10000.0000
    ##  - Prevalence               : 10.0000
    ##  - Prob. hosp. dies         : 0.5000
    ##  - Prob. hosp. recovers     : 0.5000
    ##  - Seed                     : 1545.0000
    ##  - Sim count                : 50.0000
    ## 
    ## Distribution of the population at time 100:
    ##  - (0) Susceptible  :  9990 -> 155
    ##  - (1) Exposed      :    10 -> 0
    ##  - (2) Infected     :     0 -> 0
    ##  - (3) Hospitalized :     0 -> 0
    ##  - (4) Recovered    :     0 -> 9364
    ##  - (5) Deceased     :     0 -> 481
    ## 
    ## Transition Probabilities:
    ##  - Susceptible   0.96  0.04  0.00  0.00  0.00  0.00
    ##  - Exposed       0.00  0.84  0.16  0.00  0.00  0.00
    ##  - Infected      0.00  0.00  0.53  0.06  0.42  0.00
    ##  - Hospitalized  0.00  0.00  0.00  0.29  0.35  0.36
    ##  - Recovered     0.00  0.00  0.00  0.00  1.00  0.00
    ##  - Deceased      0.00  0.00  0.00  0.00  0.00  1.00

# Computing reproductive number

``` r
rt <- list.files("saves", pattern = "reproductive", full.names = TRUE)
rt <- lapply(seq_along(rt), \(i) {cbind(id = i, fread(rt[i]))}) |>
    rbindlist()

# Computing for each individual
rt <- rt[, .(rt = mean(rt)), by = c("id", "source_exposure_date")]
setorder(rt, source_exposure_date)

ggplot(rt, aes(x = source_exposure_date, y = rt)) +
    geom_jitter(alpha = .1) +
    geom_smooth() +
    lims(y = c(0, 5))
```

    ## `geom_smooth()` using method = 'gam' and formula = 'y ~ s(x, bs = "cs")'

    ## Warning: Removed 29 rows containing non-finite values (`stat_smooth()`).

    ## Warning: Removed 249 rows containing missing values (`geom_point()`).

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
    ggplot(aes(x = date, y = counts)) +
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

| status       |     Avg |    50% |     2.5% |    97.5% |
| :----------- | ------: | -----: | -------: | -------: |
| Susceptible  |  167.90 |  167.5 |  143.000 |  192.875 |
| Exposed      |    0.00 |    0.0 |    0.000 |    0.000 |
| Infected     |    0.00 |    0.0 |    0.000 |    0.000 |
| Hospitalized |    0.00 |    0.0 |    0.000 |    0.000 |
| Recovered    | 9335.72 | 9336.5 | 9284.450 | 9391.750 |
| Deceased     |  496.38 |  497.0 |  457.225 |  541.325 |
