import sqlite3
import os
from sqlite3 import Error
import numpy as np
import io
import csv
import random
from collections import namedtuple
from shutil import copyfile

# Build an sqlLite DB to store file, pulse, species, and prediction information.


class NABat_DB:

    def __init__(self, p='db0',):

        self.path = p
        try:
            os.mkdir(self.path)
        except:
            pass

        self.db_name = '{}/NABatAcoustics.v1.sqlite'.format(self.path)
        if os.path.isfile(self.db_name):
            pass
        else:
            copyfile('base_db/NABatAcoustics.v1.sqlite', self.db_name)

        self.conn = self._create_connection(self.db_name)
        self.conn.row_factory = namedtuple_factory
        self.cursor = self.conn.cursor()
        self.cursor.execute("PRAGMA journal_mode=WAL")

        self.fastCon = self._create_connection(self.db_name)
        self.fastCursor = self.fastCon.cursor()

    def _create_connection(self, db_file):
        conn = None
        try:
            conn = sqlite3.connect(
                db_file, 60, detect_types=sqlite3.PARSE_DECLTYPES, check_same_thread=False)
            return conn
        except Error as e:
            print(e)

        return conn

    def to_csv(self, table):
        names = self.query('PRAGMA table_info({});'.format(table))
        data = self.query('select * from {};'.format(table))
        with open('{}/{}.csv'.format(self.path, table), 'w') as f:
            writer = csv.writer(f)
            writer.writerow([n[1] for n in names])
            writer.writerows(data)

    def get_files(self, file_id=None, file_name=None):
        if file_id is not None:
            return self.query(
                "SELECT id FROM file where id = ?;", (file_id,))
        elif file_name is not None:
            return self.query(
                "SELECT id FROM file where name = ?;", (file_name,))
        else:
            return self.fastQuery(
                "SELECT name FROM file;")

    def get_pulses(self, file_id=None):
        if file_id is not None:
            return self.query(
                "SELECT * FROM pulse where file_id = ?;", (file_id,))
        else:
            return self.fastQuery(
                "SELECT p.*, draw FROM pulse p join file f on f.id = p.file_id;")

    def add_file(self, name, duration, sample_rate, manual_id, grts_id, draw=None):

        if draw == None:
            r = random.random()
            if r <= 0.80:
                draw = 'train'
            elif r <= 0.90:
                draw = 'test'
            else:
                draw = 'validate'

        file_id = self.insert(
            "INSERT INTO file (draw, name, duration, sample_rate, manual_id, grts_id) VALUES (?,?,?,?,?,?);", (draw, name, duration, sample_rate, manual_id, grts_id))
        return file_id, draw

    def add_pulse(self, file_id, frequency, amplitude, sig_noise, offset, time,  window, path):
        return self.insert(
            "INSERT INTO pulse (file_id, frequency, amplitude, sig_noise, offset, time, window, path) VALUES (?,?,?,?,?,?,?,?);", (file_id, frequency, amplitude, sig_noise, offset, time, window, path))

    def add_predictions(self, data):
        self.conn.executemany(
            "insert into prediction (model_name, pulse_id, confidence, species_id) values (?,?,?,?);", data)
        self.conn.execute('commit;')

    def query(self, query, args={}):
        try:
            self.cursor.execute(query, args)
            return self.cursor.fetchall()
        except Error as e:
            print(e)
            return None

    def fastQuery(self, query, args={}):
        try:
            self.fastCursor.execute(query, args)
            return self.fastCursor.fetchall()
        except Error as e:
            print(e)
            return None

    def insert(self, query, args={}):

        try:
            self.cursor.execute(query, args)
            self.conn.commit()
            return self.cursor.lastrowid
        except Error as e:
            print(e)
            return None


def adapt_array(arr):
    """
    http://stackoverflow.com/a/31312102/190597 (SoulNibbler)
    """
    out = io.BytesIO()
    np.save(out, arr)
    out.seek(0)
    return sqlite3.Binary(out.read())


def convert_array(text):
    out = io.BytesIO(text)
    out.seek(0)
    return np.load(out)


def namedtuple_factory(cursor, row):
    """Returns sqlite rows as named tuples."""
    fields = [col[0] for col in cursor.description]
    Row = namedtuple("Row", fields)
    return Row(*row)


# Converts np.array to TEXT when inserting
sqlite3.register_adapter(np.ndarray, adapt_array)

# Converts TEXT to np.array when selecting
sqlite3.register_converter("array", convert_array)


# Species frequency ranges

# http://www.sonobat.com/download/WesternUS_Acoustic_Table_Mar2011.pdf
# http://www.sonobat.com/download/EasternUS_Acoustic_Table_Mar2011.pdf
# http://www.sonobat.com/download/AZ_Acoustic_Table-Mar08.pdf

SPECIES_FREQUENCY_RANGES = {
    'ANPA': [27000, 51000],
    'ARJA': [4999, 99999],
    'BRCA': [4999, 99999],
    'CHME': [4999, 99999],
    'CORA': [4999, 99999],
    'COTO': [22000, 41000],
    'COTOVI': [4999, 99999],
    'DIEC': [4999, 99999],
    'EPFU': [25000, 52000],
    'EUFL': [10000, 25000],
    'EUMA': [10000, 17000],
    'EUPE': [10000, 19000],
    'EUUN': [4999, 99999],
    'IDPH': [14000, 18000],
    'LABL': [37000, 61000],
    'LABO': [29000, 73000],
    'LACI': [17000, 49000],
    'LAEG': [4999, 99999],
    'LAIN': [25000, 41000],
    'LAMI': [4999, 99999],
    'LANO': [24000, 44000],
    'LASE': [35000, 52000],
    'LAXA': [28000, 56000],
    'LENI': [4999, 99999],
    'LEYE': [4999, 99999],
    'MACA': [28000, 55000],
    'MOME': [4999, 99999],
    'MOMO': [4999, 99999],
    'MYAR': [33000, 45000],
    'MYAU': [42000, 65000],
    'MYCA': [45000, 95000],
    'MYCI': [40000, 71000],
    'MYEV': [31000, 71000],
    'MYGR': [41000, 85000],
    'MYKE': [4999, 99999],
    'MYLE': [40000, 71000],
    'MYLU': [38000, 73000],
    'MYOC': [4999, 99999],
    'MYSE': [37000, 95000],
    'MYSO': [37000, 70000],
    'MYTH': [24000, 50000],
    'MYVE': [41000, 49000],
    'MYVO': [39000, 89000],
    'MYYU': [46000, 91000],
    'NOISE': [5000, 120000],
    'NOLE': [4999, 99999],
    'NYFE': [10000, 41000],
    'NYHU': [32000, 48000],
    'NYMA': [12000, 30000],
    'PAHE': [41000, 70000],
    'PESU': [36000, 50000],
    'STRU': [4999, 99999],
    'TABR': [18000, 46000]
}
