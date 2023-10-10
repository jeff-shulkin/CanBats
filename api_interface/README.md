# API Interface
## How to upload data  
Ensure your data is formatted as a csv file with no header line. Proper formatting is:  
```<unix time>,<latitude>,<longitude>,<species>```

To upload, run the following command:  
```python3 upload_data.py <filename>.csv```
## How to download data
Run the following command:  
```python3 download_data.csv```  
You can then filter by time, location, or species, or use no filter at all. Be sure to add no more than one filter for each available filter option. The filtered data will be output as "\<Time> \<FILTERED/ALL> DATA.csv"