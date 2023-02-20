#!/bin/bash
echo "Total arguments : $#"
echo "viruses = $1"
echo "countries = $2"
echo "numLines = $3"
echo "duplicates allowed = $4"
vfile=$1
cfile=$2
fnfile="firstNames.txt"
lnfile="lastNames.txt"
numLines=$3
duplicates=$4

cCount=0
cArray=()
while read line; do

    cArray[cCount]=$line
    ((cCount++))
done < $cfile

vCount=0
vArray=()
while read line; do

    vArray[vCount]=$line
    ((vCount++))
done < $vfile

fnCount=0
fnArray=()
while read line; do

    fnArray[fnCount]=$line
    ((fnCount++))
done < $fnfile

lnCount=0
lnArray=()
while read line; do

    lnArray[lnCount]=$line
    ((lnCount++))
done < $lnfile

touch inputFile.txt

# BASED ON WHETHER DUPLICATES ARE TURNED ON OR NOT, FILL THE INPUTFILE.TXT FILE WITH RECORDS OF DUPLICATE OR NON-DUPLICATE IDS.
# WHEN DUPLICATES ARE ON, DUPLICATE RECORDS WILL HAVE THE SAME ID, AGE, FNAME, LASTNAME, COUNTRY AND POSSIBLY DIFFERENT VIRUS, VACCINATED, ETC. 
# IN ORDER TO AVOID EXCESSIVE TIME COMPLEXITY, I ONLY CALL $RANDOM 3 TIMES PER REPETITION, SO THERE MIGHT BE SIMILARITIES IN THE CITIZENS' TRAITS.
# WHEN DUPLICATES ARE OFF, I CREATE AN ARRAY OF IDS 0-9999 AND SHUFFLE IT. THEN, I CREATE A NEW RECORD WITH EACH ID IN THE ARRAY.


if [[ $duplicates -eq 1 ]]
then
    count=0
    while [ "$count" -lt $numLines ]; do

            number=$RANDOM
            number1=$RANDOM
            number2=$RANDOM

            let citizenID=`expr $number % 10000`
            let age=`expr $citizenID % 120`
            let fname=`expr $citizenID % $fnCount`
            let lname=`expr $citizenID % $lnCount`
            let country=`expr $citizenID % $cCount`
            let virus=`expr $number1 % $vCount`
            let vaccinated=`expr $number2 % 2`
            let dVaccinated=`expr $number1 % 30`
            ((dVaccinated++))
            let mVaccinated=`expr $number2 % 12`
            ((mVaccinated++))
            let yVaccinated=`expr $number1 % 30`
            let yVaccinated=yVaccinated+1990
            if [[ vaccinated -eq 1 ]]
            then
                echo "$citizenID ${fnArray[fname]} ${lnArray[lname]} ${cArray[$country]} $age ${vArray[virus]} YES $dVaccinated-$mVaccinated-$yVaccinated"  >> "inputFile.txt"
            else
                echo "$citizenID ${fnArray[fname]} ${lnArray[lname]} ${cArray[$country]} $age ${vArray[virus]} NO"  >> "inputFile.txt"
            fi
            ((count++))
    done
else
    count=0
    while [ "$count" -lt $numLines ]; do

        citizenArray[count]=$count
        ((count++))
    done
    citizenArray=( $(shuf -e "${citizenArray[@]}") )
    
    count=0
    while [ "$count" -lt $numLines ]; do

        number=$RANDOM
        number1=$RANDOM

        citizenID=${citizenArray[count]}
        let age=`expr $number % 120`
        let fname=`expr $number1 % $fnCount`
        let lname=`expr $number % $lnCount`
        let country=`expr $number1 % $cCount`
        let virus=`expr $number % $vCount`
        let vaccinated=`expr $number1 % 2`
        let dVaccinated=`expr $number % 30`
        ((dVaccinated++))
        let mVaccinated=`expr $number1 % 12`
        ((mVaccinated++))
        let yVaccinated=`expr $number % 30`
        let yVaccinated=yVaccinated+1990
        if [[ vaccinated -eq 1 ]]
        then
            echo "$citizenID ${fnArray[fname]} ${lnArray[lname]} ${cArray[$country]} $age ${vArray[virus]} YES $dVaccinated-$mVaccinated-$yVaccinated"  >> "inputFile.txt"
        else
            echo "$citizenID ${fnArray[fname]} ${lnArray[lname]} ${cArray[$country]} $age ${vArray[virus]} NO"  >> "inputFile.txt"
        fi
        ((count++))
    done
fi

echo "Created inputFile.txt"
