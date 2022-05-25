library(data.table)

set.seed(7778)
N_desired       <- 10000
N_locations     <- ceiling(N_desired/200)
N_avg_locations <- 1

toth <- fread("model-saltlake/data-raw/analysisData.txt")

# - `n`: total number of people in household
# - `a`: number who were antibody tested
# - `s`: number who responded to the survey but were not antibody tested
# - `aPP`: number who reported a prior positive test result and received a positive antibody test
# - `aPN`: number who reported a prior positive test result and received a negative antibody test
# - `aNP`: number who reported no prior positive test result and received a positive antibody test
# - `sP`: number who were surveyed, reported a prior positive test result, and did not receive an antibody test

setnames(
  toth,
  colnames(toth),
  c("household_size", "antibody_tested", "responded_but_not_tested", 
    "prior_pos_test_and_test_pos",
    "prior_pos_test_and_test_neg",
    "prior_neg_test_and_test_pos",
    "surveyed_prior_pos_test_and_not_tested",
    "freq"
    )
)

toth[, sum(household_size * freq)] # Total population

# Building the synthetic population --------------------------------------------
sizes <- toth[, sample(household_size, size = N_desired, replace = TRUE, prob = freq)]
sizes <- data.table(sizes, cum_size = cumsum(sizes))
sizes <- sizes[cum_size <= N_desired]
sizes[, household := 1L:.N]
sizes <- sizes[, .(household = rep(household, sizes))]
sizes[, id := (1L:.N) - 1]

# Adding the reminder
if (nrow(sizes) < N_desired)
  sizes <- rbind(
    sizes,
    data.table(
      household = max(sizes$household),
      id        = max(sizes$id) + 1L:(N_desired - nrow(sizes))
    ))

# Building ties between individuals
loc_size <- rpois(N_desired, N_avg_locations)

locations <- data.table(
  id  = rep(sizes$id, loc_size), 
  loc = sample.int(N_locations, sum(loc_size), replace = TRUE)
) |> unique()

locations <- merge(
  locations[,.(ego = id, loc)],
  locations[,.(alter = id, loc)],
  by = "loc", allow.cartesian = TRUE
)

locations <- locations[, .(
  ego   = fifelse(ego > alter, ego, alter),
  alter = fifelse(ego > alter, alter, ego)
  )] |> unique()


locations[, .(n = .N), by = .(ego)][, hist(n)]
locations[, .(n = .N), by = .(ego)][, quantile(n, prob = c(.025,.5,.975))]
locations[, .(n = .N), by = .(ego)][, mean(n)]
