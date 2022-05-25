# Data used for the simulations

The data generated for the simulations is located in the folder [`data`](../data)

## analysisData.txt

Directly extrated from the paper by [Toth *et al.* (PLOS ONE, 2021)](https://doi.org/10.1371/journal.pone.0259097)

The data are represented as follows. For each household in the dataset, we captured the following 7 values from the data:

- `n`: total number of people in household

- `a`: number who were antibody tested

- `s`: number who responded to the survey but were not antibody tested

- `aPP`: number who reported a prior positive test result and received a positive antibody test

- `aPN`: number who reported a prior positive test result and received a negative antibody test

- `aNP`: number who reported no prior positive test result and received a positive antibody test

- `sP`: number who were surveyed, reported a prior positive test result, and did not receive an antibody test

Those surveyed participants who reported no prior positive test result includes
both those who had never been tested and those who had been tested but received
no positive results. We did not have sufficient information to properly distinguish
those two groups, nor to determine the circumstances of any prior negative tests
that might affect the inferred probability of true prior infection.

