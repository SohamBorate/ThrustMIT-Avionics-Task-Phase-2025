import pandas as pd

df = pd.read_csv("Agniastra-Flight-Data.csv")
to_be_smoothed = "pressure"

# http://www.statistics4u.info/fundstat_eng/cc_savgol_coeff.html
window = 25
h = 5175
coeffs = [-253, -138, -33, 62, 147, 222, 287, 343, 387, 422, 447, 462, 467,
    462, 447, 422, 387, 343, 287, 222, 147, 62, -33, -138, -253
]

# Apply the formula: (Sum of (data * coeff)) / h
df["smoothed_" + to_be_smoothed] = (
    df[to_be_smoothed]
    .rolling(window = window, center = True)
    .apply(lambda x: sum(x * coeffs) / h)
)

df.to_csv("Agniastra-Flight-Data.csv", index = False)
