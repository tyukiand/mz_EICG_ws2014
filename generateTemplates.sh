#!/bin/bash
#
# This script copies the template folder and replaces the 
# exercise number.
#
# Input parameters: start and end of the range of exercise numbers
startNum=$1
endNum=$2

# Clean up the template directory, don't copy all the generated stuff
for fileName in exercise.aux exercise.log exercise.out exercise.pdf
do 
  realFileName=$(echo $fileName | sed "s/\./_EXERCISENUMBER./g")
  rm template/tex/$realFileName
done

# Generate bunch of copies of the template directory
for i in $(seq $startNum $endNum)
do
  dirName=$(printf "exercise_%02d" $i)
  if [ -d "$dirName" ] 
  then
    echo "The directory $dirName already exists, skipping"
  else
    cp -r template $dirName
    # replace the EXERCISENUMBER-variable in some of the files
    paddedI=$(printf "%02d" $i)
    sed -i "s/EXERCISENUMBER/$i/g" $dirName/tex/exercise_EXERCISENUMBER.tex
    sed -i "s/EXERCISENUMBER/$paddedI/g" $dirName/tex/build
    # rename some of the files
    mv $dirName/tex/exercise_EXERCISENUMBER.tex $dirName/tex/exercise_${paddedI}.tex
  fi
done
