#!/bin/bash

FILES=*.jpg

SIZES=(256 512 1024 1536 2048)
for SIZE in "${SIZES[@]}" 
do
    echo "**** Creating images of size ${SIZE} ****"
    i=1
    for f in $FILES
    do
      echo "Processing $f file... $i"
      convert $f -resize ${SIZE}x${SIZE}^ tmp.bmp
      #convert tmp.bmp -gravity center -extent ${SIZE}x${SIZE} ${i}_${SIZE}.bmp
      convert tmp.bmp -gravity center -extent 256x256 ${i}_${SIZE}.bmp
      ((i+=1))
    done
done

rm tmp.bmp

echo $(date) > initialized.txt