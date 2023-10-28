import requests
import json
import pytz
from datetime import datetime
from decouple import config

all_url = config('BASE_URL') + '/allitems'
query_url = config('BASE_URL') + '/query'

def get_unix_time(year, month, day):
    timezone = pytz.timezone('US/Eastern')

    noon = timezone.localize(datetime(2000 + year, month, day, 12, 0, 0))
    
    unix_timestamp = noon.timestamp()
    
    return int(unix_timestamp)

def get_all_items():
    response = requests.get(all_url)

    if response.status_code != 200:
        print("Error retrieving items")
        print(response.text)
        return
    
    with open(f'{datetime.now()} ALL DATA.csv', 'w') as of:
        for item in json.loads(response.text)['items']:
            of.write(f"{item['otime']},{item['latitude']},{item['longitude']},{item['species']}\n")
    print(f"Wrote file \"{datetime.now()} ALL DATA.csv\"")

def get_filtered_items(filters, ea_vals):
    filter = '(' + ') AND ('.join(filters) + ')'

    params =  {
        'FilterExpression' : filter,
        'ExpressionAttributeValues' : json.dumps(ea_vals)
    }

    response = requests.get(query_url, params=params)

    if response.status_code != 200:
        print("Error retrieving items")
        print(response.text)
        return
    
    with open(f'{datetime.now()} FILTERED DATA.csv', 'w') as of:
        for item in json.loads(response.text):
            of.write(f"{int(item['otime'])},{item['latitude']},{item['longitude']},{item['species']}\n")
    print(f"Wrote file \"{datetime.now()} ALL_DATA.csv\"")

def main():
    
    filters = []
    ea_vals = {}
    prompt = "Add filter species (s), location (l), or time period (t), OR run query (r): "

    while 1:
        addFilter = input(prompt)[0].lower()

        if addFilter == 'r':
            if not len(filters):
                get_all_items()
            else:
                get_filtered_items(filters, ea_vals)
            break
        
        elif addFilter == 's':
            species = input("List all species to include (space-separated): ").split()
            new_filter = "species = :s0"
            ea_vals[':s0'] = species[0]
            for i in range(1, len(species)):
                ea_vals[':s'+str(i)] = species[i]
                new_filter += f" OR species = :s{i}"
            filters.append(new_filter)
        
        elif addFilter == 't':
            in_date = input("Enter the date of the starting night (YY MM DD): ").split()
            start_time = get_unix_time(int(in_date[0]), int(in_date[1]), int(in_date[2]))
            in_date = input("Enter the date of the ending morning (YY MM DD): ").split()
            end_time = get_unix_time(int(in_date[0]), int(in_date[1]), int(in_date[2]))
            ea_vals[':t0'] = start_time
            ea_vals[':t1'] = end_time
            filters.append("otime BETWEEN :t0 AND :t1")

        elif addFilter == 'l':
            lat = float(input("Enter the latitude: "))
            lon = float(input("Enter the longitude: "))
            ea_vals[':lat'] = lat
            ea_vals[':lon'] = lon
            filters.append("latitude = :lat AND longitude = :lon")

if __name__ == "__main__":
    main()