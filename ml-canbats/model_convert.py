import tensorflow as tf
import glob
import pprint
import numpy as np
from db import NABat_DB
from PIL import Image
import os


# Keep this connection open for the life of this model training run.
db = NABat_DB()

# Point to a directory of species codes containing .wav files. 
# Example "../v1.1.0/data/wav/ANPA/p163_g89522_f28390444.wav"
directory = './WavesW'


# Make sure there are at least 3 example files for each class we want to include.
sample_classes = []
x = [c.split('/')[-1] for c in glob.glob('{}/*'.format(directory), recursive=True)]
for c in x:
    size = len(glob.glob('{}/{}/*'.format(directory, c), recursive=True))
    if size > 40:
        sample_classes.append(c)
        
# Alphibitize.
sample_classes.sort()

# Print the classes we will include.
pprint.pprint(sample_classes)

# Return a list of files belonging to a dataset. 'train', 'test', or 'validate'.
def get_files(draw):
    return db.query('select id, name, grts_id from file where draw = ? and grts_id != 0 order by id',(draw,))

# Return list of bat pulses that originated from a given recording file, by file id.
def get_pulses(file_id):
    return db.query('select * from pulse where file_id = ? order by id',(file_id,))

# Yield a spectrogram image and the class it belongs to. 
def image_generator(draw):
    try:
        draw = draw.decode("utf-8")
    except:
        pass
    
    # Get list of files.
    files = get_files(draw)
    # print(files)
    for f in files:
        species = f.name.split('/')[-2]
        if species in sample_classes:
            species_id = sample_classes.index(species)
            
            # Get a list of pulses (and path to associated spectrogram image on disk) belonging to file.
            metadata = get_pulses(f.id)
            
            for i, m in enumerate(metadata):
                # Normalize the image so that each pixel value
                # is scaled between 0 and 1.
                image = Image.open(m.path)
                img = np.array(image)
                img = img[..., :3].astype('float32')
                img /= 255.0
                image.close()
                yield {"input_1": img}, species_id

train_dataset = tf.data.Dataset.from_generator(
    image_generator, args=['train'],
    output_types=({"input_1": tf.float16}, tf.int32),
    output_shapes=({"input_1": (100,100,3)}, () )                                      
    ).batch(32).prefetch(1000)

def rep_data():
    for data in train_dataset.take(32):
        # print({"input_1": tf.cast(data[0]["input_1"], dtype=tf.float32)})
        yield {"input_1": tf.cast(data[0]["input_1"], dtype=tf.float32)}
        # yield [data[0]["input_1"]]

converter = tf.lite.TFLiteConverter.from_saved_model("./models/m-1")
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = rep_data

# print('go')

tflite_quant_model = converter.convert()

# print('done')

os.makedirs("./models_lite", exist_ok=True)
with open("./models_lite/m-1.tflite", 'wb') as out:
    out.write(tflite_quant_model)