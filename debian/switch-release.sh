#! /bin/bash

helpcall() {
    echo "     Usage: switch-release.sh -d distribution-name [-v vtk-major-version]"
    exit 1
}

ARGC=$#
if [ "$#" -lt 2 ]; then
    helpcall
fi

vtk=5

while getopts "v:d:" opt; do
    case "$opt" in
	v)
            vtk=$OPTARG
            ;;
	d)  distrib=$OPTARG
            ;;
    esac
done    
echo $distrib
if [ -z $distrib ];then 
     helpcall
fi
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd $DIR
echo $DIR
cp -f changelog changelog.bck
cp -f control control.bck
#cp -f ../CMakeLists.txt ../CMakeLists.txt.bck
#if [ $vtk == 6 ]; then
#sed -i 's|find_package( VTK REQUIRED )|find_package( VTK REQUIRED )\n  find_package(Qt5Widgets REQUIRED) \n  find_package(Qt5OpenGL REQUIRED)|g' ../CMakeLists.txt
#fi
vtk=libvtk$vtk-dev
echo $vtk
sed -i "s|libvtk5-dev|$vtk|g" control
cd ../
dch -r -D $distrib
debuild -S -I
cd debian
mv changelog.bck changelog
mv control.bck control
#mv ../CMakeLists.txt.bck ../CMakeLists.txt

