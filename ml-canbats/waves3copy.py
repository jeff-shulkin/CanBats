import os
import random
import shutil

dir = "WavesWfull"
outdir = "WavesW"

if not os.path.exists(dir):
    print("bad dir")
    quit()

for species in os.listdir(dir):
    s = os.path.join(dir, species)
    o = os.path.join(outdir, species)
    os.makedirs(o, exist_ok=True)

    temp = os.listdir(s)
    # print(len(temp))
    # continue
    if(len(temp) < 150):
        continue
    waves = random.sample(temp, 150)

    files = [os.path.join(s, file) for file in waves]
    outs = [os.path.join(o, file) for file in waves]

    for file, out in zip(files, outs):
        # print(file, out)
        shutil.copyfile(file, out)
