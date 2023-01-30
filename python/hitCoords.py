import sys

import numpy as np
import pandas as pd
from scipy.stats import multivariate_normal
from sklearn.mixture import GaussianMixture


def run():
    hitgram_path = sys.argv[1]
    output_file = sys.argv[2]
    num_directions = int(sys.argv[3])

    hitgram = pd.read_csv(hitgram_path).to_numpy()

    n_components = 40
    if (n_components > len(hitgram)):
        n_components = len(hitgram)

    gmm = GaussianMixture(n_components=n_components, tol=1e-25, covariance_type='full', max_iter=100000000)
    gmm.fit(hitgram)
    sampled = gmm.sample(num_directions)[0]
    sampled = np.remainder(sampled, 1.0)
    scores = np.exp(gmm.score_samples(sampled))

    output_df = pd.DataFrame(np.vstack([sampled.T, scores]).T)
    output_df.to_csv(output_file, index=False, header=None)

if __name__ == '__main__':
    run()