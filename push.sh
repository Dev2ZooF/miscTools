#!/bin/bash

#
# CONFIG VARS
#

# The path (relative to this script) to the version file this script will update before pushing
VERSIONFILE='version'
# The git branch name where this script will push
BRANCH='master'

#
# BEGIN
#

# Go to push.sh dir
currentDir=$(pwd)
cd $(dirname $0)

# Retrieving current version
currentVersion=$(cat $VERSIONFILE | grep version | tr -d \"\':,[:space:][:alpha:])

if [[ -z $currentVersion ]]
then
	echo Error : cannot retrieve the version number.
	exit -1
fi

currMaj=$(echo $currentVersion | cut -d"." -f1)
currMin=$(echo $currentVersion | cut -d"." -f2)
currPatch=$(echo $currentVersion | cut -d"." -f3)


# Asking user for kind of update
echo Current version : $currentVersion
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

# Updating version number in $VERSIONFILE
if [[ -n $userInput ]] && [[ $userInput = "M" ]]
then
	currMaj=$((currMaj+1))
	currMin=0
	currPatch=0
elif [[ -n $userInput ]] && [[ $userInput = "m" ]]
then
    currMin=$((currMin+1))
	currPatch=0
else
    currPatch=$((currPatch+1))
fi
nextVersion="$currMaj.$currMin.$currPatch"
sed -i "s/$currentVersion/$nextVersion/g" $VERSIONFILE

# Push to git repo
echo Pushing version $nextVersion
git add .
git commit -a -m "Pushing version $nextVersion"
git push origin $BRANCH
echo Version $nextVersion successfuly pushed!

cd $currentDir

#
# END
#