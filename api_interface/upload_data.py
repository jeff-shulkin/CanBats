import sys
import json
import requests
from decouple import config

post_url = config('BASE_URL') + '/item'
get_url = config('BASE_URL') + '/allitems'

if len(sys.argv) != 2:
    print("Usage: python3 upload_data.py <filename>.csv")
    sys.exit(1)

response = requests.get(get_url)
if response.status_code != 200:
    print("Error getting table info")
    print(response.text)
    sys.exit(1)
else:
    count = len(json.loads(response.text)['items'])

with open(sys.argv[1]) as csvFile:
    for line in csvFile:
        attrs = line.split(',')
        data = {
            "id" : count,
            "time" : int(attrs[0]),
            "latitude" : float(attrs[1]),
            "longitude" : float(attrs[2]),
            "species" : attrs[3]
        }
        response = requests.post(post_url, data=data)

        if response.status_code != 200:
            print("Error uploading item:", line)
            print(response.text)
        else:
            count += 1