library(data.table)
download.file(
    "https://slco.org/globalassets/1-site-files/health/programs/covid/download.zip",
    destfile = "model-saltlake/data-raw/cases.zip",
    method = "wget"
)

if (!dir.exists("model-saltlake/data-raw/cases"))
    dir.create("model-saltlake/data-raw/cases")

unzip(
    "model-saltlake/data-raw/cases.zip",
    exdir = "model-saltlake/data-raw/cases")

byday <- fread(
    "model-saltlake/data-raw/cases/Summary_Case Counts per Day.csv"
    )

hosp <- fread(
    "model-saltlake/data-raw/cases/Hospitalizations_Counts per Day.csv"
    )
