# API Interface
## Required Packages
The following packages must be installed through pip to upload and download data:  
```requests, json, pytz, datetime, python-decouple```  
  
  You must also have a .env file in the same directory you are running the .py files from containing the API url under the name 'BASE_URL'. For access to this url, please contant Ali Kassem (kassemal@umich.edu).

## How to Upload Data  
Ensure your data is formatted as a csv file with no header line. Proper formatting is:  
```<unix time>,<latitude>,<longitude>,<species>```

To upload, run the following command:  
```python3 upload_data.py <filename>.csv```

## How to Download Data
Run the following command:  
```python3 download_data.py```  
You can then filter by time, location, or species, or use no filter at all. Be sure to add no more than one filter for each available filter option. The filtered data will be output as "\<Time> \<FILTERED/ALL> DATA.csv"