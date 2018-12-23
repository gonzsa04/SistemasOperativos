# poner antes chmod +x ./script.txt en la consola para dar permisos para ejecutar el script

SRC="./src"
TEMP="./temp"
MOUNT="./mount-point"

FILE1="myFS.h"
INVISIBLEFILE=".invisibleFile.txt"

# borramos temp si lo hubiera y lo volvemos a crear
rmdir $TEMP 
mkdir $TEMP 

# copiamos myFS.h y fuseLib.h a tmp y mount
cp $SRC/$FILE1 $TEMP/$FILE1	
cp $SRC/$FILE1 $MOUNT/$FILE1

./my-fsck virtual-disk

# comparamos los archivos de mount y tmp
DIFF_FILE1=$(diff $SRC/$FILE1 $MOUNT/$FILE1)

if [ "$DIFF_FILE1" == "" ]; 
then echo "El primer archivo es IGUAL en ambas"
else echo "El primer archivo es DIFERENTE entre ambas"
fi

mv $SRC/$FILE1 $MOUNT/$FILE1

./my-fsck virtual-disk

DIFF_FILE1_RENAMED=$(diff $SRC/$FILE1 $MOUNT/$FILE1)

if [ "$DIFF_FILE1_RENAMED" == "" ]; 
then echo "El primer archivo renombrado es IGUAL que el original"
else echo "El primer archivo renombrado es DIFERENTE al original"
fi

cp Makefile $MOUNT/$Makefile1

mv $MOUNT/Makefile1 $MOUNT/INVISIBLEFILE

ls

./my-fsck virtual-disk

ls
