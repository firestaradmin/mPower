#!/bin/bash
echo "=================================="
echo "pushing note to github ..."
echo "=================================="
git add .
git status
git commit -m "$(date) with push.sh"
git push
echo " "
echo " "
echo "=================================="
echo "upload over, press any key to continue!"
read
#date=$(date)
#echo "\""$(date)"\""
