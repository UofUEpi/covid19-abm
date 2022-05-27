library(data.table)

set.seed(7778)
# This setup yields a mean degree of 25.09
N_desired       <- 10000
N_locations     <- ceiling(N_desired/100)
N_avg_locations <- .5

expected_deg <- function(n, m, v) {
  (n - 1) * (1 - (2*(1 - 1/m)^v - (1 - 1/m)^(2 * v)) ^ m)
} 

expected_deg(N_desired, N_locations, N_avg_locations)

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
households <- toth[, sample(household_size, size = N_desired, replace = TRUE, prob = freq)]
households <- data.table(households, cum_size = cumsum(households))
households <- households[cum_size <= N_desired]
households[, household := 1L:.N]
households <- households[, .(household = rep(household, households))]
households[, id := (1L:.N) - 1]

# Adding the reminder
if (nrow(households) < N_desired)
  households <- rbind(
    households,
    data.table(
      household = max(households$household),
      id        = max(households$id) + 1L:(N_desired - nrow(households))
    ))

households <- merge(
  households[,.(household, ego = id)],
  households[,.(household, alter = id)], 
  allow.cartesian = TRUE
)[ego != alter]

households <- households[, .(ego = fifelse(ego < alter, ego, alter), alter = fifelse(ego < alter, alter, ego))]
households <- unique(households)

# Building ties between individuals --------------------------------------------
p_visit <- 1 - (1 - 1/N_locations)^N_avg_locations

# Sampling visits
visits <- which(matrix(
  runif(N_locations * N_desired) < p_visit, ncol = N_desired
), arr.ind = TRUE)

visits <- data.table(
  id  = visits[,1] - 1L,
  loc = visits[,2]
)

# Creating the bipartite graph  
visits <- merge(
  visits[, .(ego = id, loc)],
  visits[, .(alter = id, loc)],
  by = "loc", allow.cartesian = TRUE, all = TRUE
)[, loc:=NULL] |> unique()

# Retrieving the edgelist
visits <- visits[, .(ego = fifelse(ego > alter, alter, ego), alter = fifelse(ego > alter, ego, alter))]
visits <- unique(visits)[ego != alter,]

mean(table(visits[, c(ego, alter)])) # 22.2 as expected
hist(table(visits[, c(ego, alter)]))

saveRDS(
  list(households = households, outsize_contacts = visits),
  file = "model-saltlake/data/population.rds"
)

fwrite(
  unique(rbind(households, visits)),
  file = "model-saltlake/data/population.txt",
  sep = " ", col.names = FALSE
)
