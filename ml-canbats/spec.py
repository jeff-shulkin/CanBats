from spectrogram import Spectrogram
from pathlib import Path

import os
import random

paths = []

sp = Spectrogram()

SourceDir = "/media/usb/WavesUnused2"
DoneDir = "/media/usb/Waves_Done2"
ImagesDir = "/home/canbats3/images"
os.makedirs(DoneDir, exist_ok=True)

sourcelist = os.listdir(SourceDir)
files = random.sample(sourcelist, min(len(sourcelist), 1))
#files = os.listdir(SourceDir)
for file2 in files:
    file = os.path.join(os.fsdecode(SourceDir), os.fsdecode(file2))
    print(file)
    print(file2)
    file2 = file2.split(".wav")[0]

    spdata = sp.process_file(file)
    print("File Processed")
    # For each pulse within file...

    for i, m in enumerate(spdata.metadata):
        # ...create a place to put the spectrogram.
        
        path = '/'.join([ImagesDir, '{}/t_{}.png'.format(file2, m.offset)])
        Path('/'.join([ImagesDir, '{}'.format(file2)])).mkdir(parents=True, exist_ok=True)

        # ...create a spectrogram image surrounding the pulse and save to disk.
        img = sp.make_spectrogram(m.window, spdata.sample_rate)
        img.save(path)
        #print(path)

    print("Files Saved")

    file3 = os.path.join(os.fsdecode(DoneDir), os.fsdecode('.'.join([file2, "wav"])))
    os.rename(file, file3)
