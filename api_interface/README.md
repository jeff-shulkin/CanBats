#API Interface
##How to upload data  
Ensure your data is formatted as a csv file with no header line. Proper formatting is:  
[unix time],[latitude],[longitude],[species]  
To upload, run the following command:  
python3 upload_data.py [filename].csv