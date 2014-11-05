Each exercise-folder contains three subfolders:
- `tex`: contains the actual TeX-Files
- `code`: contains the c++/OpenGL code
- `scripts`: bunch of bash/scala/python scripts that have been used to generate the
  actual tex/c++ code

The .pdf can be build by cd'ing into the `tex` directory and calling

user@hostname$ ./build

The template `tex` directory contains a document with examples of formulas and 
images.

Note that there is a file `definitions.tex` that contains many useful 
abbreviations like for example:
- `\rPar`                  for round brackets that always scale with the content
- `\sPar`, `\cPar`         same for square and curved parentheses
- `\setPredicate`          for expressions like {x|P(x)} where all the brackets and
                           separating bars scale with the content
- `\norm`                   vertical double bars that scale with the argument
- `\scalar`                scalar products
- `\Real`, `\Quaternion`   for nice symbols for R and H
etc.
