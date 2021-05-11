#!/bin/bash
set -e
set -x
test -d ../AriaCodaDocs
GITBRANCH=`git branch --show-current`
GITCOMMIT=`git log --pretty=reference -n 1 | cut -f 1 -d ' '`
make doc DOXYGEN_OUTDIR=..\\/AriaCodaDocs
cd ../AriaCodaDocs
git add --all
git commit -m "Regenerate documentation from AriaCoda $GITBRANCH $GITCOMMIT"
git push
