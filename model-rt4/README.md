
# Description of the model[^1]

This is a piece of R code that sets up a model for an infectious disease
using the SEIR (Susceptible, Exposed, Infectious, Recovered) model with
connections.

The first part of the code defines several variables:

- `n` is the total population size, set to 20,000.
- `preval` is the initial fraction of the population that is infected,
  set to 0.001.
- `crate` is the contact rate, set to 2.0. This is the average number of
  contacts per person per time.
- `trate` is the transmission probability, set to 0.3. This is the
  probability that the disease transmits between people in contact.
- `incu` is the average number of days of incubation, set to 7.0.
- `rrate` is the recovery rate, set to 1/7. This is the rate at which
  infected individuals recover and become immune.

The second part of the code creates an instance of the `ModelSEIRCONN`
class, named “SEIR with test”. This model is initialized with the
parameters defined above.

# Running the model

In the following example, we change the transmission rate to be a
function of whether an individual becomes isolated. To do this, at the
beginning, we randomly assign a 0/1 value and define the transmission
rate as a function of this value.

``` r
# Reading in parameters
n      <- 2e4      # Population size
preval <- 0.001    # Initial infected fraction
crate  <- 2.0      # Contact Rate
trate  <- 0.3      # Transmission prob
incu   <- 7.0      # Avg incubation
rrate  <- 1.0/7.0  # Recovery rate
asymrate <- .3     # Fraction of asymptomatic

mymodel <- ModelSEIRCONN(
  name              = "SEIR with test",
  n                 = n,
  prevalence        = preval,
  contact_rate      = crate,
  transmission_rate = trate,
  incubation_days   = incu,
  recovery_rate     = rrate
  )

# Editing the virus to make the isolated population less infectious
set.seed(331)
isolate <- cbind(1, as.numeric(runif(n) < (1 - asymrate)))
set_agents_data(mymodel, isolate)

# Creating the function for setting the probability of transmission
fun <- virus_fun_logit(
  vars = c(0L, 1L),
  coefs = c(-10, dlogis(trate)),
  mymodel
  )

# Changing the transmission probability to be a function of the isolated
# population
get_virus(mymodel, 0) |>
  set_prob_infecting_fun(mymodel, fun)
```

``` r
saver <- make_saver(
  "total_hist",
  "transmission",
  "transition",
  "reproductive",
  "generation"
)

set.seed(331)
run_multiple(mymodel, ndays = 100, nsims = 200, saver = saver, nthreads = 8)
```

    Starting multiple runs (200) using 8 thread(s)
    _________________________________________________________________________
    _________________________________________________________________________
    ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| done.
     done.

Looking into the results

``` r
summary(mymodel)
```

    ________________________________________________________________________________
    ________________________________________________________________________________
    SIMULATION STUDY

    Name of the model   : Susceptible-Exposed-Infected-Removed (SEIR) (connected)
    Population size     : 20000
    Agents' data loaded : yes (2 columns/features)
    Number of entities  : 0
    Days (duration)     : 100 (of 100)
    Number of viruses   : 1
    Last run elapsed t  : 0.00s
    Total elapsed t     : 15.00s (200 runs)
    Last run speed      : 3.77 million agents x day / second
    Average run speed   : 25.84 million agents x day / second
    Rewiring            : off

    Global actions:
     (none)

    Virus(es):
     - SEIR with test (baseline prevalence: 0.10%)

    Tool(s):
     (none)

    Model parameters:
     - Avg. Incubation days : 7.0000
     - Contact rate         : 2.0000
     - Prob. Recovery       : 0.1429
     - Prob. Transmission   : 0.3000

    Distribution of the population at time 100:
      - (0) Susceptible : 19980 -> 19980
      - (1) Exposed     :    20 -> 0
      - (2) Infected    :     0 -> 0
      - (3) Recovered   :     0 -> 20

    Transition Probabilities:
     - Susceptible  1.00  0.00  0.00  0.00
     - Exposed      0.00  0.81  0.19  0.00
     - Infected     0.00  0.00  0.88  0.12
     - Recovered    0.00  0.00  0.00  1.00

``` r
# Retrieving the results
res <- run_multiple_get_results(mymodel)
```

# Computing reproductive number

``` r
rt <- data.table(res$reproductive)

# Renaming
setnames(rt, "sim_num", "id")

rt <- rt[, .(rt = mean(rt)), by = c("id", "source_exposure_date")]
setorder(rt, source_exposure_date)

rt[, pick := order(runif(.N)), by = .(source_exposure_date)]
rt_sample <- rt[pick <= 20]
rt[, pick := NULL]

ggplot(rt_sample, aes(x = source_exposure_date, y = rt)) +
    geom_jitter(alpha = .1, height = 0) +
    geom_smooth(method = "loess", se = TRUE) +
    lims(y = c(0, 10))
```

    `geom_smooth()` using formula = 'y ~ x'

    Warning in simpleLoess(y, x, w, span, degree = degree, parametric = parametric,
    : span too small.  fewer data values than degrees of freedom.

    Warning in simpleLoess(y, x, w, span, degree = degree, parametric = parametric,
    : pseudoinverse used at -0.07

    Warning in simpleLoess(y, x, w, span, degree = degree, parametric = parametric,
    : neighborhood radius 11.07

    Warning in simpleLoess(y, x, w, span, degree = degree, parametric = parametric,
    : reciprocal condition number 0

    Warning in simpleLoess(y, x, w, span, degree = degree, parametric = parametric,
    : There are other near singularities as well. 197.96

    Warning in predLoess(object$y, object$x, newx = if (is.null(newdata)) object$x
    else if (is.data.frame(newdata))
    as.matrix(model.frame(delete.response(terms(object)), : span too small.  fewer
    data values than degrees of freedom.

    Warning in predLoess(object$y, object$x, newx = if (is.null(newdata)) object$x
    else if (is.data.frame(newdata))
    as.matrix(model.frame(delete.response(terms(object)), : pseudoinverse used at
    -0.07

    Warning in predLoess(object$y, object$x, newx = if (is.null(newdata)) object$x
    else if (is.data.frame(newdata))
    as.matrix(model.frame(delete.response(terms(object)), : neighborhood radius
    11.07

    Warning in predLoess(object$y, object$x, newx = if (is.null(newdata)) object$x
    else if (is.data.frame(newdata))
    as.matrix(model.frame(delete.response(terms(object)), : reciprocal condition
    number 0

    Warning in predLoess(object$y, object$x, newx = if (is.null(newdata)) object$x
    else if (is.data.frame(newdata))
    as.matrix(model.frame(delete.response(terms(object)), : There are other near
    singularities as well. 197.96

![](README_files/figure-commonmark/repnum-1.png)

``` r
setorder(rt, id, source_exposure_date, rt)
fwrite(rt, "reproductive_numbers.gz", )
```

# Generation time

``` r
gentime <- data.table(res$generation)
setnames(gentime, "sim_num", "id")

# Removing negative values
gentime <- gentime[gentime > 0]
gentime <- gentime[, .(gtime = mean(gentime)), by = .(id, source_exposure_date)]

# Sampling
gentime[, pick := order(runif(.N)), by = .(source_exposure_date)]
gentime_sample <- gentime[pick <= 20]
gentime[, pick := NULL]

ggplot(gentime_sample, aes(x = source_exposure_date, y = gtime)) +
    geom_jitter(alpha = .1, height = 0) +
    geom_smooth(method = "loess", se = TRUE) +
    lims(y = c(0, 10))
```

![](README_files/figure-commonmark/gentime-1.png)

``` r
fwrite(gentime, "generation_time.gz")
```

# New daily cases

Daily cases can be informed through the transition matrix.

``` r
transition <- data.table(res$transition)
setnames(transition, "sim_num", "id")

transition[, ttrans := fifelse(
    from == "Susceptible" & to == "Exposed",
    "New exposures",
    fifelse(
        from == "Exposed" & to == "Infected",
        "New infections",
        fifelse(
            (from == "Infected" & to == "Recovered") | (from == "Hospitalized" & to == "Recovered"),
            "New recovered",
            fifelse(
                from == "Infected" & to == "Hospitalized",
                "New hospitalized",
                fifelse(
                    from == "Hospitalized" & to == "Deceased",
                    "New deaths",
                    "(other)"
                )
            )
        )
    )
    )]

# File with transitions
transition_file <- transition[from != to]
transition_file <- 
    transition_file[, transition := paste0(from, "_", to)] |>
        dcast(id + date ~ transition, value.var = "counts")

fwrite(transition_file, "transitions.gz")

transition <- transition[ttrans != "(other)"]

# sampling
transition[, pick := order(runif(.N)), by = .(date, ttrans)]
transition <- transition[pick <= 10]
transition |>
    ggplot(aes(x = date, y = counts)) +
    geom_jitter(aes(colour = ttrans), alpha = .1) + 
    geom_smooth(aes(colour = ttrans), method="loess", se = TRUE)
```

    `geom_smooth()` using formula = 'y ~ x'

![](README_files/figure-commonmark/transi-1.png)

# Epi curves

``` r
epicurves <- data.table(res$total_hist)
setnames(epicurves, "sim_num", "id")

fwrite(epicurves, "epicurves.gz")

# Samlping
epicurves[, pick := order(runif(.N)), by = .(date, nviruses)]

epicurves_sample <- epicurves[pick <= 200]

epicurves_sample[state %in% c("Exposed", "Infected", "Hospitalized")] |>
    ggplot(aes(x = date, y = counts)) +
    geom_jitter(aes(colour = state), alpha = .1) + 
    geom_smooth(aes(colour = state), method="loess", se = TRUE)
```

    `geom_smooth()` using formula = 'y ~ x'

![](README_files/figure-commonmark/transitions-1.png)

``` r
epicurves_sample[!state %in% c("Exposed", "Infected", "Hospitalized")] |>
    ggplot(aes(x = date, y = counts)) +
    geom_smooth(aes(colour = state), method = "loess", se = TRUE)
```

    `geom_smooth()` using formula = 'y ~ x'

![](README_files/figure-commonmark/totals-1.png)

``` r
    # geom_jitter(aes(colour = status), alpha = .1)
```

States at the end of the simulation

``` r
epicurves_end <- epicurves[date == max(date)]
epicurves_end[, .(
    Avg     = mean(counts),
    `50%`   = quantile(counts, probs = .5),
    `2.5%`  = quantile(counts, probs = .025),
    `97.5%` = quantile(counts, probs = .975)
    ), by = "state"] |> knitr::kable()
```

| state       |      Avg |   50% |  2.5% | 97.5% |
|:------------|---------:|------:|------:|------:|
| Susceptible | 19979.99 | 19980 | 19980 | 19980 |
| Exposed     |     0.00 |     0 |     0 |     0 |
| Infected    |     0.00 |     0 |     0 |     0 |
| Recovered   |    20.01 |    20 |    20 |    20 |

[^1]: The description of the model was generated by GitHub copilot
