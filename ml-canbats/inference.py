import numpy as np
from PIL import Image
# import matplotlib.pyplot as plt
import os

def chunk_list(l, n):
    for i in range(0, len(l), n):
        yield l[i:i + n]

from tensorflow import lite

import csv

species_list = ['ANPA',
 'COTO',
 'EPFU',
 'EUMA',
 'EUPE',
 'LABL',
 'LABO',
 'LACI',
 'LAIN',
 'LANO',
 'LASE',
 'MYAU',
 'MYCA',
 'MYCI',
 'MYEV',
 'MYGR',
 'MYLE',
 'MYLU',
 'MYSE',
 'MYSO',
 'MYTH',
 'MYVO',
 'MYYU',
 'NOISE',
 'NYHU',
 'NYMA',
 'PAHE',
 'PESU',
 'TABR']

print("Interpreter Loaded")

interpreter = lite.Interpreter(model_path="./models_lite/m-1.tflite")
my_signature = interpreter.get_signature_runner()

print("Model Loaded")

# os.makedirs('./media/usb', exist_ok=True)
out_file = open("./outgoing_data.csv", 'w', newline='')
out_writer = csv.writer(out_file)

# time = 12

for waves in os.listdir("./images"):
    time = 12 #TODO change to wave timestamp thing
    wave_path = os.path.join(os.fsdecode("./images"), os.fsdecode(waves))
    print(wave_path)

    conf_species_dict = {}#dict[str, int]

    file_batches = chunk_list(os.listdir(wave_path), 32)
    for batch in file_batches:
        input = np.empty((len(batch), 100, 100, 3), dtype=np.float32)
        i = 0
        for path in batch:
            num = path.split('_')[-1].split('.')[0]
            file = os.path.join(os.fsdecode(wave_path), os.fsdecode(path))

            temp = Image.open(file)
            img = np.array(temp)
            temp.close()

            img = img[..., :3].astype('float32')
            input[i] = img / 255.0
            i += 1
            # print(img)
            
            os.remove(file)

        # my_signature is callable with input as arguments.
        output = my_signature(input_1=input)
        # 'output' is dictionary with all outputs from the inference.
        # In this case we have single output 'result'.
        # print(output)
        #print(output['dense_3'])
        # print(f"{num} = {output['dense_3'][0]}")

        for out in output['dense_3']:
            conf_species = max((c, species_list[idx]) for idx, c in enumerate(out))
            if conf_species == None:
                continue
            
            # print(conf_species[::-1])
            # out_writer.writerow([time, conf_species[1], conf_species[0], wave_path])
            # time += 1
            if conf_species[0] > 0 and conf_species[1] != "NOISE":
                # conf_species_dict.setdefault(conf_species[1], 0)
                # conf_species_dict[conf_species[1]] += 1
                temp = conf_species_dict.get(conf_species[1], (0, 0))
                temp = (temp[0] + 1, max(temp[1],conf_species[0]))
                conf_species_dict[conf_species[1]] = temp

    for species, count in conf_species_dict.items():
        out_writer.writerow([time, species, count[1], count[0], wave_path])
    # time += 1

    os.rmdir(wave_path)

print("Interpretations Done")