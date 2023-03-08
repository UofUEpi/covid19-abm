library(data.table)

set.seed(7778)
# This setup yields a mean degree of 25.09
N_desired        <- 5000
N_locations      <- ceiling(N_desired/100)
N_ties2locations <- N_desired * 5

toth <- fread("../model-saltlake/data-raw/analysisData.txt")

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

# Generating the locations
pop_locations <- data.table(
  household = 1:max(households$household)
)
pop_locations[, lon := runif(.N)]
pop_locations[, lat := runif(.N)]

pop_locations <- merge(
  x = pop_locations, 
  y = households[, .(id, household)],
  all.x = FALSE, all.y = TRUE
  )

households <- merge(
  households[, .(household, ego = id)],
  households[, .(household, alter = id)], 
  allow.cartesian = TRUE
)[ego != alter]

households <- households[, .(
  ego   = fifelse(ego < alter, ego, alter),
  alter = fifelse(ego < alter, alter, ego)
  )] |> unique()

# Building ties between individuals --------------------------------------------
who <- sample.int(n = N_desired, size = N_ties2locations, replace = TRUE)
who <- data.table(id = who)
who <- who[, .(n = .N), by = "id"]
setorder(who, id)

places <- data.table(
  id = (1:N_locations) - 1,
  lon = runif(N_locations),
  lat = runif(N_locations)
)

pop_places_ties <- NULL
for (i in 1:nrow(who)) {

  w <- pop_locations[i, sqrt((places$lon - lon)^2 + (places$lat - lat)^2)]
  w <- sample.int(size = who[i, n], n = N_locations, prob = 1 / w)
  pop_places_ties <- rbind(
    pop_places_ties,
    data.table(agent = who[i, id] - 1, place = w - 1)
  )

  if (!i %% 200)
    message(i, " complete")

}

fwrite(
  households, # Will be connected through entities
  file = "population.txt",
  sep = " ", col.names = FALSE
)

fwrite(
  pop_places_ties,
  file = "agents_entities.txt",
  sep = " ", col.names = FALSE
)

fwrite(
  pop_locations,
  file = "locations_agents.txt"
)

fwrite(
  places,
  file = "locations_entities.txt"
)