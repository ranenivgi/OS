#!/bin/bash
#Ranen Ivgi 208210708

#check if the aren't enough arguments given
if [ "$#" -lt 2 ]; then
    echo "Not enough parameters"
    exit 1
fi

#save the base path
base=$(pwd)

#move to the given directory
cd $1

#delete .out files
rm -f *.out

#complie the .c files that contain the word recieved as an argument
for i in $(ls)
do
    if [ "${i##*.}" == "c" ] && grep -i -q "\b$2\b" "$i" ; then
        gcc -w "$i" -o "${i%.*}.out"
    fi
done

#return to the base path
cd $base

#do the same proccess to each sub-direcory using recursion
if [ "$3" = "-r" ]; then        #if the '-r' flag was entered then:
	for j in "$1"/*; do         #for each file in the given path:
        if [ -d "$j" ]; then    #check if the file is a directory
            $0 $j $2 "-r"       #if it is, re-run the script with the new directory to repeat the process
        fi
    done
fi
