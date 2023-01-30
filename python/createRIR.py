import os

import numpy as np
from scipy.io.wavfile import write

from helpers import open_histograms

c = 343
max_mu = 10_000


def create_noise_sample(noise_sample_rate, t_end, volume):
    n_noise_samples = int(t_end * noise_sample_rate)
    noise_samples = np.zeros(n_noise_samples, dtype=int)
    # schroder 5.45

    c_3 = c*c*c
    pre_t_mu = 4*np.pi*c_3 / volume

    t_0 = ( (2*volume*np.log(2)) / (4*np.pi*c_3) )**(1/3)
    total_t = t_0

    dirac_pulse = 1.0

    while (total_t < t_end):
        mu = pre_t_mu * total_t*total_t
        if (mu > max_mu):
            mu = max_mu
        z = np.random.uniform()
        delta_t = (1/mu) * np.log(1/z)

        noise_sample_index = int(total_t * noise_sample_rate)
        noise_samples[noise_sample_index] = dirac_pulse
        dirac_pulse = -dirac_pulse
        total_t += delta_t


    return noise_samples


def combine_noise_sequence_and_histogram(histogram, histogram_sampling_frequency, noise_samples, noise_sample_rate):
    combined_sample = np.zeros(len(noise_samples))
    histogram_noise_sums = np.zeros(len(histogram))

    # schroder 5.47
    for k in range(len(histogram)):
        g_k = k * histogram_sampling_frequency * noise_sample_rate

        g_k_index = int(g_k)
        g_k_index_next = g_k_index +  int(histogram_sampling_frequency * noise_sample_rate)
        squared_sum = np.sum(noise_samples[g_k_index:g_k_index_next]**2)
        histogram_noise_sums[k] = squared_sum

    corrected_energy_levels = histogram / histogram_noise_sums
    corrected_energy_levels = np.nan_to_num(corrected_energy_levels, copy=False, nan=0.0, posinf=0.0, neginf=0.0)
    corrected_energy_levels = np.sqrt(corrected_energy_levels)

    for i in range(len(combined_sample)):
        k = int((i / noise_sample_rate) * (1 / histogram_sampling_frequency))
        if (k < len(histogram)):
            combined_sample[i] = noise_samples[i] * corrected_energy_levels[k]

    return combined_sample

def get_max_width_factor(w_lowest, w_highest, n_bands):
    x = np.power(w_highest / w_lowest, 1 / n_bands)
    return (x - 1) / (x + 1)

def get_G_lower(diff_p_to_frequencies, width_cross_over):
    # equation 8, raytracing, reuk wayverb
    G = np.square(np.sin(np.pi * ((diff_p_to_frequencies / width_cross_over + 1) / 2) / 2.0))
    G[np.argwhere(diff_p_to_frequencies < -width_cross_over)] = 0.0
    G[np.argwhere(diff_p_to_frequencies >= width_cross_over)] = 1.0

    return G

def get_G_higher(diff_p_to_frequencies, width_cross_over):
    # equation 8, raytracing, reuk wayverb
    G = np.square(np.cos(np.pi * ((diff_p_to_frequencies / width_cross_over + 1) / 2) / 2.0))
    G[np.argwhere(diff_p_to_frequencies < -width_cross_over)] = 1.0
    G[np.argwhere(diff_p_to_frequencies >= width_cross_over)] = 0.0

    return G

def get_G_transform(fft_frequencies, w_low, w_high, width_factor):
    lower_G = get_G_lower(fft_frequencies - w_low, w_low * width_factor)
    higher_G = get_G_higher(fft_frequencies - w_high, w_high * width_factor)

    return lower_G * higher_G


def convert_histogram_to_audio(histogram):
    num_samples = len(histogram)
    one_min_one = np.tile([1, -1], (num_samples + 1) // 2)[:num_samples]
    histogram = histogram * one_min_one

    return histogram


def run(folder_path=None, check_exists=True):

    if check_exists and os.path.exists(f'{folder_path}/histogram.wav'):
        return
    path = folder_path + "/histogram"


    histograms, histogram_settings = open_histograms(f"{folder_path}")
    samplerate = 44100

    reverb_samples = []

    for i, histogram in enumerate(histograms):
        reverb_sample = convert_histogram_to_audio(histogram)
        reverb_samples.append(reverb_sample)

    reverb_samples = np.array(reverb_samples)
    fft_frequencies = np.fft.rfftfreq(len(reverb_sample))
    reverb_samples_fft = np.fft.rfft(reverb_samples, axis=1)


    n_bands = histogram_settings['N_BANDS']


    w_lowest = 20.0 / samplerate
    w_highest = 20000.0 / samplerate
    w_edges = w_lowest * np.power(w_highest / w_lowest, np.arange(n_bands + 1) / n_bands)
    width_factor = get_max_width_factor(w_lowest, w_highest, n_bands)

    G_transforms = []

    for i in range(n_bands):
        G_transform = get_G_transform(fft_frequencies, w_edges[:-1][i], w_edges[1:][i], width_factor)
        G_transforms.append(G_transform)
        reverb_samples_fft[i] *= G_transform

    reverb_samples_conv = np.fft.irfft(reverb_samples_fft, axis=1)
    reverb_sample_conv = np.sum(reverb_samples_conv, axis=0)


    # truncate sample to be able to analyse better
    max_non_zero_index = reverb_samples.round(8).nonzero()[1].max()
    max_non_zero_index = np.ceil(max_non_zero_index / samplerate)
    max_non_zero_index = int(max_non_zero_index * samplerate)

    reverb_sample_conv = reverb_sample_conv[:max_non_zero_index]

    reverb_sample_conv = reverb_sample_conv / max(reverb_sample_conv.max(), -reverb_sample_conv.min())

    amplitude = np.iinfo(np.int16).max
    data = amplitude * reverb_sample_conv

    write(f"{path}.wav", samplerate, data.astype(np.int16))
    print(path)

if __name__ == '__main__':

    path = ""
    run(folder_path=path)

