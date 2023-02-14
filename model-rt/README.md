
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
| Infectiousness           |     0.9 |
| Infectiousness in entity |     0.9 |
| Days                     |   100.0 |
| Population Size          | 10000.0 |
| Prevalence               |   100.0 |
| N ties                   |     5.0 |
| Sim count                |    50.0 |
| N entities               |   100.0 |
| Seed                     |  1545.0 |
| N interactions           |     5.0 |
| OMP threads              |     1.0 |

The full program can be found in the file [main.cpp](main.cpp).

# Running the model

``` bash
./main.o
```

    ## _________________________________________________________________________
    ## Starting multiple runs (50) using 1 thread(s)
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
    ## Last run elapsed t  : 0.00s
    ## Total elapsed t     : 4.00s (50 runs)
    ## Last run speed      : 20.57 million agents x day / second
    ## Average run speed   : 11.46 million agents x day / second
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
    ##  - Infectiousness in entity : 0.9000
    ##  - N entities               : 100.0000
    ##  - N interactions           : 5.0000
    ##  - N ties                   : 5.0000
    ##  - OMP threads              : 1.0000
    ##  - Population Size          : 10000.0000
    ##  - Prevalence               : 100.0000
    ##  - Prob. hosp. dies         : 0.5000
    ##  - Prob. hosp. recovers     : 0.5000
    ##  - Seed                     : 1545.0000
    ##  - Sim count                : 50.0000
    ## 
    ## Distribution of the population at time 101:
    ##  - (0) Susceptible  :  9900 -> 0
    ##  - (1) Exposed      :   100 -> 0
    ##  - (2) Infected     :     0 -> 0
    ##  - (3) Hospitalized :     0 -> 8
    ##  - (4) Recovered    :     0 -> 9473
    ##  - (5) Deceased     :     0 -> 519
    ## 
    ## Transition Probabilities:
    ##  - Susceptible   1.00  0.00  0.00  0.00  0.00  0.00
    ##  - Exposed       0.00  0.87  0.13  0.00  0.00  0.00
    ##  - Infected      0.00  0.00  0.52  0.05  0.43  0.00
    ##  - Hospitalized  0.00  0.00  0.00  0.31  0.34  0.35
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

    ## Warning: Removed 161 rows containing non-finite values (`stat_smooth()`).

    ## Warning: Removed 256 rows containing missing values (`geom_point()`).

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
    geom_smooth(aes(colour = status)) # +
```

    ## `geom_smooth()` using method = 'gam' and formula = 'y ~ s(x, bs = "cs")'

![](README_files/figure-gfm/transitions-1.png)<!-- -->

``` r
    # geom_jitter(aes(colour = status), alpha = .1)
```

``` r
epicurves[!status %in% c("Exposed", "Infected", "Hospitalized")] |>
    ggplot(aes(x = date, y = counts)) +
    geom_smooth(aes(colour = status)) # +
```

    ## `geom_smooth()` using method = 'gam' and formula = 'y ~ s(x, bs = "cs")'

![](README_files/figure-gfm/totals-1.png)<!-- -->

``` r
    # geom_jitter(aes(colour = status), alpha = .1)
```

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

| status       |        Avg |  50% | 2.5% |   97.5% |
| :----------- | ---------: | ---: | ---: | ------: |
| Susceptible  | 1160.65098 |    0 |    0 | 6358.00 |
| Exposed      |  218.37412 |    0 |    0 | 2970.20 |
| Infected     |  111.96471 |    0 |    0 | 1256.75 |
| Hospitalized |   15.47373 |    4 |    1 |   97.40 |
| Recovered    | 8035.22510 | 9435 | 3133 | 9488.00 |
| Deceased     |  458.31137 |  540 |  166 |  582.00 |
