import sys
import json
import requests
from decouple import config

def main():
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

    index = 0
    with open(sys.argv[1]) as csvFile:
        for line in csvFile:
            attrs = [item.strip() for item in line.split(',')]
            data = {
                "id" : count,
                "otime" : int(attrs[0]),
                "latitude" : float(attrs[1]),
                "longitude" : float(attrs[2]),
                "species" : attrs[3]
            }
            response = requests.post(post_url, data=data)

            if response.status_code != 200:
                print(f"Error uploading item {index}:", line.strip())
                print(response.text)
            else:
                count += 1
            index += 1

if __name__ == "__main__":
    main()