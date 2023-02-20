#!/bin/bash
echo "Total arguments : $#"
if [[ $# -ne 3 ]]; then
    echo "Wrong number of arguments"
    exit
fi
echo "Input File = $1"
echo "Input Directory = $2"
echo "Number of Files per Directory = $3"
inputFile=$1
input_dir=$2
numFiles=$3

[[ -d $input_dir ]] && echo $input_dir already exists, aborting && exit
mkdir $input_dir            #make input directory
op=$PWD                     #original path

#store countries in array cArray
cCount=0
cArray=()
while read line; do

    cArray[cCount]=$line
    ((cCount++))
done < "countries.txt"

#create directories and .txt files for each country
cd $input_dir
counter=1
let numf=$numFiles+1
c=0
while [ $c -lt $cCount ]; do

    mkdir ${cArray[c]}
    cd ${cArray[c]}
    counter=1
    while [ $counter -lt $numf ]; do 

        touch ${cArray[c]}-$counter.txt
        ((counter++))
    done
    cd ..
    ((c++))
done

#create a RoundRobin Array that holds the number of the .txt file that is the next to be edited (START: all values set to 1) 
counter=0
RRarray=()
while [ $counter -lt $cCount ]; do

    RRarray[counter]=1
    ((counter++))
done

i=0
while read line; do

    arr=($line)
    country=${arr[3]}
    #save the index(i) of the country in the cArray
    for c in "${!cArray[@]}"; do
        if [[ "${cArray[$c]}" = "${country}" ]]; then
            i=$c
            break
        fi
    done

    cd ${cArray[$i]}
    #write the record in the .txt file with number equal to the value stored in index i of the Round Robin array
    echo $line >> $country-${RRarray[$i]}.txt
    let RRarray[$i]=RRarray[$i]+1               #increment value in index i of the Round Robin array
    let lim=$numFiles+1
    if [[ "${RRarray[$i]}" -eq lim ]]; then       #if value is equal to numFiles+1, set it to 1
        RRarray[$i]=1
    fi
    cd ..
done < $op/$inputFile
cd ..

echo "DONE!"
