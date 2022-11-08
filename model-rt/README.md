
``` r
library(data.table)
library(ggplot2)
```

# Running the model

``` bash
./main.o
```

    ## Starting multiple runs (100)
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
    ## Number of entitites : 0
    ## Days (duration)     : 100 (of 100)
    ## Number of variants  : 1
    ## Last run elapsed t  : 0.00s
    ## Total elapsed t     : 8.00s (100 runs)
    ## Last run speed      : 10.83 million agents x day / second
    ## Average run speed   : 12.34 million agents x day / second
    ## Rewiring            : off
    ## 
    ## Virus(es):
    ##  - Covid19 (baseline prevalence: 20 seeds)
    ## 
    ## Tool(s):
    ##  (none)
    ## 
    ## Model parameters:
    ##  - Death prob.           : 0.3000
    ##  - Hospitalization prob. : 0.1000
    ##  - Incubation period     : 0.1429
    ##  - Infectiousness        : 0.9000
    ##  - Prob. of Recovery     : 0.1429
    ## 
    ## Distribution of the population at time 100:
    ##  - (0) Susceptible  :  9980 -> 0
    ##  - (1) Exposed      :    20 -> 34
    ##  - (2) Infected     :     0 -> 42
    ##  - (3) Hospitalized :     0 -> 16
    ##  - (4) Recovered    :     0 -> 7033
    ##  - (5) Deseased     :     0 -> 2875
    ## 
    ## Transition Probabilities:
    ##  - Susceptible   0.91  0.09  0.00  0.00  0.00  0.00
    ##  - Exposed       0.00  0.86  0.14  0.00  0.00  0.00
    ##  - Infected      0.00  0.00  0.78  0.09  0.13  0.00
    ##  - Hospitalized  0.00  0.00  0.00  0.64  0.11  0.26
    ##  - Recovered     0.00  0.00  0.00  0.00  1.00  0.00
    ##  - Deseased      0.00  0.00  0.00  0.00  0.00  1.00

# Computing reproductive number

``` r
rt <- list.files("saves", pattern = "reproductive", full.names = TRUE)
rt <- lapply(seq_along(rt), \(i) {cbind(id = i, fread(rt[i]))}) |>
    rbindlist()

rt <- rt[, .(rt = mean(rt)), by = c("id", "source_exposure_date")]
setorder(rt, source_exposure_date)


ggplot(rt, aes(x = source_exposure_date, y = rt)) +
    geom_jitter() +
    scale_y_log10()
```

    ## Warning: Transformation introduced infinite values in continuous y-axis

    ## Warning: Removed 4 rows containing missing values (geom_point).

![](README_files/figure-gfm/repnum-1.png)<!-- -->

# Epi curves

``` r
epicurves <- list.files("saves", pattern = "hist", full.names = TRUE)
epicurves <- lapply(seq_along(epicurves), \(i) {
    cbind(id = i, fread(epicurves[i]))
}) |> rbindlist()

ggplot(epicurves, aes(x = date, y = counts)) +
    geom_jitter(aes(colour = status))
```

![](README_files/figure-gfm/unnamed-chunk-2-1.png)<!-- -->
