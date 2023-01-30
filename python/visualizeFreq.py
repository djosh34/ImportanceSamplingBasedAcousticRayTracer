import os

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

from scipy.io import wavfile
from scipy import signal

# https://gist.github.com/HudsonHuang/fbdf8e9af7993fe2a91620d3fb86a182
def pcm2float(sig):
    return (sig.astype('float64')) / max(sig.max(), -sig.min())


def calcflatness(response):
    response = abs(response) ** 2

    gmean = np.exp(np.mean(np.log(response)))
    amean = np.mean(response)
    flatness = (gmean / amean)
    return flatness

def return_noise(data):
    # noise
    data = np.random.normal(0, 1, len(data))
    data_too_high = np.argwhere(data > 1.0)
    data_too_low = np.argwhere(data < -1.0)
    data[data_too_high] = 1.0
    data[data_too_low] = -1.0

    return data

def frequency_and_flatness(folder_path, check_exists=True):
    if check_exists and os.path.exists(f'{folder_path}/frequency.png'):
        return

    flatness = frequency_and_flatness_plot(folder_path)

    plt.savefig(f'{folder_path}/frequency.png', dpi=300)
    plt.show()

    with open(f'{folder_path}/flatness.txt', "w") as f:
        f.write(str(flatness))
        f.close()

    print(f'{folder_path}/frequency.png')

response_list = []

def frequency_and_flatness_plot(folder_path, ax=None, label=None, title=None, lim=None, response_list=None):


    # Read in the .wav file
    rate, data = wavfile.read(f"{folder_path}/histogram.wav")
    name = os.path.basename(folder_path)

    min_freq = 1000
    max_freq = 20000

    data = pcm2float(data)




    # Compute the frequency response
    f = np.fft.rfftfreq(len(data), d=1./rate)
    response = abs(np.fft.rfft(data))

    min_freq_index = np.argwhere(f > min_freq)[0][0]
    f, response = f[min_freq_index:], response[min_freq_index:]

    max_freq_index = np.argwhere(f > max_freq)[0][0]
    f, response = f[:max_freq_index], response[:max_freq_index]

    # Smooth the frequency response using a moving average
    window = signal.windows.hann(1000)
    response_smooth = signal.convolve(response, window, mode='same')


    window = signal.windows.hann(1000)
    response_smooth_flatness = signal.convolve(response, window, mode='same')
    flatness = calcflatness(response_smooth_flatness)


    response_smooth_norm = response_smooth / response_smooth.max()

    # Plot the frequency response
    if (ax is None):
        fig, ax = plt.subplots()

    response_smooth_norm_db = 20 * np.log10(response_smooth_norm)
    ax.semilogx(f, response_smooth_norm_db, label=label)


    plt.xscale('log')

    ticks = [1000, 2000,5000,10000,20000]
    ax.xaxis.set_minor_formatter(ticker.NullFormatter())

    ax.xaxis.set_ticks(ticks)
    ax.xaxis.set_major_formatter(ticker.ScalarFormatter())
    ax.xaxis.set_major_formatter(ticker.FormatStrFormatter("%d"))
    ax.xaxis.set_minor_formatter(ticker.NullFormatter())
    ax.set_ylim(-13,1)
    if lim is not None:
        ax.set_ylim(lim, 4)



    if (ax is None):
        plt.title(f'{name} with flatness: {flatness}')
    ax.set_title(title, fontsize=11)


    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Amplitude (dB)")
    ax.grid(True)

    plt.xlim(min_freq,max_freq)


    return flatness



if __name__ == '__main__':

    path = ""

    for folder in os.listdir(path):
        folder_path = f"{path}/{folder}"
        if os.path.isdir(folder_path) and ("diff" not in os.path.basename(folder_path)):
            frequency_and_flatness(folder_path)
