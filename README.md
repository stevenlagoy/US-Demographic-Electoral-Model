# Computational Model of Demographic Archetypes and Electoral Outcomes in the United States
## Summary
I propose to create a computational model of U.S. regionalism based on mixtures of demographic archetype “groups,” and to analyze how accurately these archetypical categorizations predict historical electoral outcomes using methods in data science. The objective of this project is to apply modern computer modeling and machine learning techniques to give insight into identity-informed voting behavior in the United States.
## Introduction
The United States has historically exhibited complex relationships between demographic composition and voting behavior on a precinct level. While researchers often analyze individual predictors such as income, race, or education, less work has been focused on understanding composite demographic archetypes across the United States, or on how those archetypes impact or are impacted by local understandings of regionalism. This project proposes a computational modeling framework to intelligently cluster regions (groups of Counties, States, etc) into overlapping demographic “groups” and then to analyze how effectively these groups predict voting outcomes in the United States based on real historical data. This study explores the intersection of fields including computer science, demography, information theory, and political science.
## Research Goals
1. Collection: Group Discovery
  a. Collect and analyze U.S. Census data and public research information
  b. Use regression modeling and [Monte Carlo](https://en.wikipedia.org/wiki/Monte_Carlo_method) [simulated annealing](https://en.wikipedia.org/wiki/Simulated_annealing) to optimize a pattern of grouping within the data
  c. Develop a system representing each region as a mixture of demographic groups, descriptors, or archetypes based on the model
2. Analysis: Voting Predictability
  a. Identify whether specific demographic groups strongly predict electoral outcomes
  b. Quantify predictive strength using concepts in information theory, including [entropy-based analysis](https://en.wikipedia.org/wiki/Entropy_(information_theory)
  c. Highlight groups which display stable versus volatile voting behavior
3. Interpretation: Examination and Impact
  a. Use real-world interpretation of group voting behavior to explain trends in data
  b. Analyze optimal group clustering to explain understandings of regionalism in the United States
  c. Examine how high-entropy groups might correspond to swing regions or other emergent voter behavior
## Methods
### Data Collection
Demographic Data will be gathered from public data sources like the United States Census Bureau. Data will be collected across several layers: national data, state-wide data, county-level data. The raw scraped information will be parsed for important sorted data, which will be stored for each area of interest.
Historical Voting Data will be collected from existing datasets from the US Elections Project, the MIT Election Lab, and others. A sufficient window of time to collect enough data but to avoid introducing external complications will be chosen. The electoral outcome data from that time period will be stored in connection to defined precincts, like states, congressional districts, and counties. The data will be analyzed against known historical trends to derive the most important values like voter turnout, party preference, and result stability.

### Modeling Approach
* Represent county demographics as a nonnegative linear combination of group profiles, “descriptors”. Each descriptor will map identifying demographic traits (“Asian”, “Age 60-70”, “Healthcare Industry”) to a percentage effect value. For any geographic entity, the demographics of that entity will be approximated by the summation of the effects of each assigned descriptor for each identified demographic group.
* Use methods such as nonnegative matrix factorization (NMF), topic modeling, or simulated annealing for optimization. To find absolute maximums in the data, use a temperature value specifying the probability that a “worse” change is accepted. Throughout simulation, gradually lower this temperature to make finding absolute maximum more likely.
* Score a model state based on three metrics: Accuracy, Parsimony, Specificity. Prioritize the most accurate models. Attempt to use as few descriptors as possible. Penalize models which use too many groups (AIC/BIC/MDL) to favor parsimonious explanations.
Test with several allowed numbers of unique descriptors (between 100 and 1500) and graph maximum observed accuracy against number of allowed descriptors to find the best overall model.

### Voting Analysis
* Using collected data and assigned descriptors, fit logistic/linear models to link archetypes to turnout and party preference in historical elections.
* For each demographic, find overall predictive strength for that demographic alone. High predictive strength values indicate strong turnout and bellwether status, while low predictive strength correlates to low participation and/or irregular voting preference.
* Compute entropy within each group’s voting distribution to measure stability of predictive strength. High entropy indicates that voting results are strongly affected against expectations by the presence of a group or demographic, while low entropy indicates that group or demographic members vote in-line with general populations.
* Compare predictive strength and entropy for descriptors, demographics, and regions to explore the dynamics of swing regions. Regions with no clear party preference (high cumulative entropy) are expected to align with known swing states and counties in the United States.

### Validation, Visualization, and Interpretation
* Create publicly-accessible tools (web application) for visualization and exploration of the results.
* Examine existing interpretations of identity politics and regional voting behavior to interpret quantitative results.
### Tools and Technologies
* Version control on GitHub Repository
* [BeautifulSoup4](https://pypi.org/project/beautifulsoup4/) in Python for data scraping, [json](https://docs.python.org/3/library/json.html) intermediate storage
[SQLite](https://docs.python.org/3/library/sqlite3.html) database for querying gathered census data
C++ with [OMP](https://www.openmp.org/) parallelism for archetype discovery and model simulation
HTML and JS/TS for the [visualization website](https://stevenlagoy.github.io/us-demographics-viewer/demographics_viewer.html), Hosting on [GitHub Pages](https://stevenlagoy.github.io/)
## Expected Contributions
### Artifacts created
* Collected and standardized database of U.S. Census data
* Computationally-derived model of “Descriptors” assigning geographic entities to demographic archetypes
* Visualization system (web app) for visualizing this data
* Standardized database of historical electoral outcomes
* Mapping of archetype model to electoral prediction
* Final research journal paper, poster, and presentation
## Contributions to Computational Political Science
* A novel mixed-membership demographic grouping system for U.S. regions
* A framework for quantifying predictability vs. volatility in electoral behavior using entropy
## Significance
This work bridges computational modeling, demography, data science, and political science. The research addresses a core question in political science and data science: how do composite demographic archetypes, rather than isolated demographic variables, shape electoral outcomes in the United States? This study proposes a novel computational framework to capture the complexity of identity-informed voting behavior. The significance of this work includes: introducing a mixed-membership modeling approach to reflect real-world complexity; providing an advanced tool for understanding regional identities in the United States; quantifying predictive strength and entropy of demographic archetypes to improve election forecasting models, and find the limits of demographic determinism in electoral behavior; and integrate information-theoretic measures into political behavior research, an innovative methodological contribution.

## References and Sources
Wikipedia: Monte Carlo method – https://en.wikipedia.org/wiki/Monte_Carlo_method
Wikipedia: Simulated Annealing – https://en.wikipedia.org/wiki/Simulated_annealing
World Population Review - https://worldpopulationreview.com/
The Williams Institute (UCLA School of Law) – https://williamsinstitute.law.ucla.edu/
The Demographic Statistical Atlas of the United States – https://statisticalatlas.com/
United States Census Bureau – https://www.census.gov/
American Community Survey (ACS) – https://www.census.gov/programs-surveys/acs.html
CIA World Factbook – https://www.cia.gov/the-world-factbook/
CDC National Center for Health Statistics – https://www.cdc.gov/nchs/index.html
US Elections Project – https://www.electproject.org/election-data
MIT Election Lab – https://electionlab.mit.edu/data
Beautiful Soup – https://pypi.org/project/beautifulsoup4/, https://www.crummy.com/software/BeautifulSoup/bs4/doc/
OpenMP – https://www.openmp.org/
