
``` r
library(data.table)
library(ggplot2)
```

# Running the model

``` bash
./main.o
```

    ## Starting multiple runs (20)
    ## _________________________________________________________________________
    ## _________________________________________________________________________
    ## ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| done.
    ##  done.
    ## 
    ## ________________________________________________________________________________
    ## SIMULATION STUDY
    ## 
    ## Name of the model   : (none)
    ## Population size     : 50000
    ## Number of entitites : 0
    ## Days (duration)     : 200 (of 200)
    ## Number of variants  : 1
    ## Last run elapsed t  : 0.00s
    ## Total elapsed t     : 14.00s (20 runs)
    ## Last run speed      : 11.04 million agents x day / second
    ## Average run speed   : 13.90 million agents x day / second
    ## Rewiring            : off
    ## 
    ## Virus(es):
    ##  - Covid19 (baseline prevalence: 50 seeds)
    ## 
    ## Tool(s):
    ##  (none)
    ## 
    ## Model parameters:
    ##  - Death prob.           : 0.1000
    ##  - Hospitalization prob. : 0.1000
    ##  - Incubation period     : 0.1429
    ##  - Infectiousness        : 0.5000
    ##  - Prob. of Recovery     : 0.1429
    ## 
    ## Distribution of the population at time 200:
    ##  - (0) Susceptible  : 49950 -> 37635
    ##  - (1) Exposed      :    50 -> 311
    ##  - (2) Infected     :     0 -> 196
    ##  - (3) Hospitalized :     0 -> 64
    ##  - (4) Recovered    :     0 -> 9891
    ##  - (5) Deseased     :     0 -> 1903
    ## 
    ## Transition Probabilities:
    ##  - Susceptible   1.00  0.00  0.00  0.00  0.00  0.00
    ##  - Exposed       0.00  0.86  0.14  0.00  0.00  0.00
    ##  - Infected      0.00  0.00  0.78  0.09  0.13  0.00
    ##  - Hospitalized  0.00  0.00  0.00  0.78  0.13  0.09
    ##  - Recovered     0.00  0.00  0.00  0.00  1.00  0.00
    ##  - Deseased      0.00  0.00  0.00  0.00  0.00  1.00

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

    ## `geom_smooth()` using method = 'gam' and formula 'y ~ s(x, bs = "cs")'

    ## Warning: Removed 20 rows containing non-finite values (stat_smooth).

    ## Warning: Removed 41 rows containing missing values (geom_point).

![](README_files/figure-gfm/repnum-1.png)<!-- -->

# Epi curves

``` r
epicurves <- list.files("saves", pattern = "hist", full.names = TRUE)
epicurves <- lapply(seq_along(epicurves), \(i) {
    cbind(id = i, fread(epicurves[i]))
}) |> rbindlist()

epicurves[status %in% c("Exposed", "Infected", "Hospitalized")] |>
    ggplot(aes(x = date, y = counts)) +
    geom_smooth(aes(colour = status)) +
    geom_jitter(aes(colour = status), alpha = .1)
```

    ## `geom_smooth()` using method = 'gam' and formula 'y ~ s(x, bs = "cs")'

![](README_files/figure-gfm/unnamed-chunk-2-1.png)<!-- -->
