#!/bin/bash

# The path (relative to this script) to the version file this script will update before pushing
VERSIONFILES='version'

# The git branch name where this script will push
BRANCH='master'

CURRENTDIR=$(pwd)
cd $(dirname $0)

VER=$(cat $VERSIONFILES | grep "version:" |  cut -d"'" -f2)

echo Current version : $VER
echo ---
echo Which type of new version to push ?
echo Major [M] ? Minor [m] ? Patch [Anything else] ?
echo Type [A] to abort push.
read userInput

if [[ -n $userInput ]] && ([[ $userInput = "A" ]] || [[ $userInput = "a" ]])
then
	echo Push aborted by user.
	exit 0
fi

IFS="." read -r -a ARRVER <<< "$VER"

if [[ -n $userInput ]] && [[ $userInput = "M" ]]
then
    ARRVER[0]=$((${ARRVER[0]} + 1))
    ARRVER[1]=0
    ARRVER[2]=0
elif [[ -n $userInput ]] && [[ $userInput = "m" ]]
then
    ARRVER[1]=$((${ARRVER[1]} + 1))
    ARRVER[2]=0
else
    ARRVER[2]=$((${ARRVER[2]} + 1))
fi
NVER="${ARRVER[0]}.${ARRVER[1]}.${ARRVER[2]}"
sed -i "s/$VER/$NVER/g" $VERSIONFILES

echo Pushing version $NVER
git add .
git commit -a -m "Pushing version $NVER"
git push origin $BRANCH
echo Version $NVER successfuly pushed!

cd $CURRENTDIR