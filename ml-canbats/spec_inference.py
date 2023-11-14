from spectrogram import Spectrogram
#from pathlib import Path

print("Spectrogram Loaded")

import os
import random

import numpy as np
# from PIL import Image
from tensorflow import lite

print("Interpreter Loaded")

interpreter = lite.Interpreter("./models_lite/m-1.tflite")

print("Model Loaded")

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
    num = m.offset
    # ...create a place to put the spectrogram.
    # path = './images/t_{}.png'.format(m.offset)
    # Path('./images'.format()).mkdir(parents=True, exist_ok=True)

    # ...create a spectrogram image surrounding the pulse and save to disk.
    temp = sp.make_spectrogram(m.window, spdata.sample_rate)
    # img.save(path)
    # print(path)
    img = np.array(temp)
    temp.close()

    img = img[..., :3].astype('float32')
    img /= 255.0
    # print(img)
    # print(img.shape)

    input = np.expand_dims(img, axis=0)

    # Load the TFLite model in TFLite Interpreter
    # There is only 1 signature defined in the model,
    # so it will return it by default.
    # If there are multiple signatures then we can pass the name.
    my_signature = interpreter.get_signature_runner()

    # print(my_signature.get_input_details())

    # my_signature is callable with input as arguments.
    output = my_signature(input_1=input)
    # 'output' is dictionary with all outputs from the inference.
    # In this case we have single output 'result'.
    # print(output)
    # print(output['dense_3'])
    print(f"{num} = {output['dense_3'][0]}")

    # plt.plot(output['dense_3'][0])
    # plt.show()

print("Interpretations Done")