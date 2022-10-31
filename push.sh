#!/bin/bash
echo "=================================="
echo "pushing note to github ..."
echo "=================================="
git add .
git status

if [ ! $1 ]; then
# default opreation
git commit -m "$(date) with push.sh"
echo " "
echo "====================================="
echo "commit: $(date) with push.sh"
echo " "
echo " "
else
git commit -m "$1 | with push.sh"
echo " "
echo "====================================="
echo "commit: $1 | with push.sh"
echo " "
echo " "
fi
git push
echo " "
echo " "
echo "=================================="
echo "upload over, press any key to continue!"
read
#date=$(date)
#echo "\""$(date)"\""
