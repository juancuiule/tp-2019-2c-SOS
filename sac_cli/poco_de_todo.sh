#!/bin/bash

counter=1
_counter=1
while [ $counter -le 10 ]
do
	VAR1="Archivo_$counter"
	dd if=/dev/urandom of=$VAR1 bs=1024 count=10 status=none
	echo "Creando $VAR1 de 10KiB"
	((counter++))
done
sleep 3 
clear
ls -lS --color=auto

truncate=6

echo "Truncando archivos..."
while [ $truncate -le 10 ]
do
	VAR1="Archivo_$truncate"
	truncate -s 0 $VAR1
	((truncate++))
done 

ls -lS --color=auto

sleep 4
clear

echo "Crear 2 directorios"
while [ $_counter -le 2 ]
do
	VAR1="DIR_$_counter"
	mkdir $VAR1
	sleep 0.5
	((_counter++))
done 

ls -lS --color=auto
sleep 3
clear
echo "Moviendo archivo a DIR2"
files=5
while [ $files -le 10 ]
do
	VAR1="Archivo_$files"
    	VAR2="DIR_2/"
	mv $VAR1 $VAR2
	((files++))
done 

sleep 3
clear
tree

sleep 3
clear

echo "Mover DIR_2 a DIR_1"
mv DIR_2 DIR_1
sleep 3

tree

sleep 4
clear
echo "Append Data_file_10"
sleep 2
echo "Andó." >> Data_file_10
echo "leer Data_file_10"
sleep 2
cat Data_file_10
sleep 3