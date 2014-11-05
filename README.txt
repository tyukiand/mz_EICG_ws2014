#################################
### Content of this directory ###
#################################

This directory contains:
- This README.txt 
  (describes content of this directory)
- a `template` directory with a template exercise 
  (contains all the stuff that is needed to build a .pdf)
- NAMES.txt 
  (please enter your name there, the content is inserted 
  into the header of each built exercise sheet)
- generateTemplates.sh 
  (this script is used to generate template directories)
- actual exercise_XY directories
  (these are carefully renamed copies of the `template` 
   directory that can be filled with the actual solutions)

#########################################################
### How the template generation works, in more detail ###
#########################################################

This directory contains a script called `generateTemplates.sh`.
This script can generate `exercise_XY` folders from the 
`template` directory. The `template` directory should not
be copied manually, since it is easy to forget to rename some
of the files, or to replace some of the variables. 

To generate a single directory for the k-th exercise sheet 
invoke the script as follows:

me@here$ ./generateTemplates k k

To generate multiple directories for all exercises from
k to m, invoke the script as follows:

me@here$ ./generatetemplates k m

If a directory for the k-th exercise already exists, it
will not be overridden by the script.

After generating a directory for the k-th exercise,
read the README.txt in the `exercise_k` directory for
further instructions.

###############################
### What you should do once ###
###############################

Please enter you name into the NAMES.txt file.
The content of this file is used in the headers of the 
generated PDFs.

############################################
### What you should do for k-th exercise ###
############################################

Goto directory `exercise_k` then repeat until done:
- pull
- modify the exercise_k/tex/exercise_k.tex, check that it builds with
  $ ./build
  check that it looks nice with evince or okular or whatever
  pdf-reader you use.
- add your code (.hpp, .cpp), config and build files to exercise_k/code
- commit
- push


"Fragen? Anregungen?" (C) 
