from spectrogram import Spectrogram
from pathlib import Path

import os
import random

paths = []

sp = Spectrogram()

file = os.listdir("./waves")
file = file[random.randint(0, len(file)-1)]
file = os.path.join(os.fsdecode("./waves"), os.fsdecode(file))
print(file)

spdata = sp.process_file(file)
print("File Processed")
# For each pulse within file...

for i, m in enumerate(spdata.metadata):
    # ...create a place to put the spectrogram.
    path = './images/t_{}.png'.format(m.offset)
    Path('./images'.format()).mkdir(parents=True, exist_ok=True)

    # ...create a spectrogram image surrounding the pulse and save to disk.
    img = sp.make_spectrogram(m.window, spdata.sample_rate)
    img.save(path)
    # print(path)

print("Files Saved")