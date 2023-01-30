import json
import os

import numpy as np
import pandas as pd
from numpy import genfromtxt


def open_histogram(path):

    histogram = pd\
        .read_csv(f"{path}.csv", delimiter=',', dtype=float, header=None)\
        .T.to_numpy().flatten()
    f = open(f"{path}.json")
    histogram_settings = json.load(f)


    histogram = histogram[:-histogram_settings["N_BANDS"]]
    histogram = histogram / np.max(histogram)

    return histogram, histogram_settings

def open_histograms(folder_path):


    f = open(f"{folder_path}/histogram.json")
    histogram_settings = json.load(f)
    histograms = genfromtxt(f"{folder_path}/histogram.csv", delimiter=',')
    histograms = np.delete(histograms, -1, axis=1)

    for band in range(histogram_settings['N_BANDS']):
        histograms[band] = histograms[band] / np.max(histograms[band])

    return histograms, histogram_settings