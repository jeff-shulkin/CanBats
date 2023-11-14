import numpy as np
from PIL import Image
# import matplotlib.pyplot as plt
import os

from tensorflow import lite

print("Interpreter Loaded")

interpreter = lite.Interpreter("./models_lite/m-1.tflite")

print("Model Loaded")

for path in os.listdir("./images"):
    num = path.split('_')[-1].split('.')[0]
    file = os.path.join(os.fsdecode("./images"), os.fsdecode(path))

    temp = Image.open(file)
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