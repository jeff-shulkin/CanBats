import os
import random

dir2 = os.fsencode(input("dir?\n"))
for dir in os.listdir(dir2):
    dir = os.path.join(os.fsdecode(dir2), os.fsdecode(dir))
    # print(dir)
    for file in os.listdir(dir):
        filename = os.fsdecode(file)
        if filename.endswith(".wav"):
            filename2 = "".join(["p163_g", format(random.randint(1, 999999), '06d'),"_f",filename.split("-")[-1]])
            # print(filename2)
            os.rename(os.path.join(os.fsdecode(dir), filename), os.path.join(os.fsdecode(dir), filename2))