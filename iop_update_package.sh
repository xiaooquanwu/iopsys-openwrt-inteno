cd $1
NEW_HASH=`git log -n 1 master --pretty=format:"%H"`
echo $NEW_HASH
cd ../../..
sed -r -i  "s/^PKG_SOURCE_VERSION:=.*/PKG_SOURCE_VERSION:=$NEW_HASH/g" $3
git commit $3 -m "$2: update package"
