#!/bin/sh
for zip in *.zip
do
  dirname=`echo $zip | sed 's/\.zip$//'`
  if mkdir -p "../WavesW/$dirname"
  then
    if cd "../WavesW/$dirname"
    then
      unzip ../../Zipped/"$zip"
      cd "../../Zipped"
      # rm -f $zip # Uncomment to delete the original zip file
    else
      echo "Could not unpack $zip - cd failed"
    fi
  else
    echo "Could not unpack $zip - mkdir failed"
  fi
done
