# poner antes chmod +x ./script.txt en la consola para dar permisos para ejecutar el script

SRC="./src"
TEMP="./temp"
MOUNT="./mount-point"

FILE1="myFS.h"
FILE2="fuseLib.c"
FILE3="MyFileSystem.c"

# borramos temp si lo hubiera y lo volvemos a crear
rmdir $TEMP 
mkdir $TEMP 

# copiamos myFS.h y fuseLib.h a tmp y mount
cp $SRC/$FILE1 $TEMP/$FILE1	
cp $SRC/$FILE1 $MOUNT/$FILE1

cp $SRC/$FILE2 $TEMP/$FILE2	
cp $SRC/$FILE2 $MOUNT/$FILE2

./my-fsck virtual-disk

# comparamos los archivos de mount y tmp
DIFF_FILE1=$(diff $MOUNT/$FILE1 $TEMP/$FILE1)
DIFF_FILE2=$(diff $MOUNT/$FILE2 $TEMP/$FILE2)

if [ "$DIFF_FILE1" == "" ]; 
then echo "El primer archivo es IGUAL en ambas"
else echo "El primer archivo es DIFERENTE entre ambas"
fi

if [ "$DIFF_FILE2" == "" ]; 
then echo "El segundo archivo es IGUAL en ambas"
else echo "El segundo archivo es DIFERENTE entre ambas"
fi

# truncamos el archivo 1 y volvemos a comparar
truncate --size=-4096 $TEMP/$FILE1
truncate --size=-4096 $MOUNT/$FILE1

./my-fsck virtual-disk

DIFF_FILE1_TRUNCATED=$(diff $SRC/$FILE1 $MOUNT/$FILE1)

if [ "$DIFF_FILE1_TRUNCATED" == "" ]; 
then echo "El primer archivo truncado es IGUAL que el original"
else echo "El primer archivo truncado es DIFERENTE al original"
fi

cp $SRC/$FILE3 $MOUNT/$FILE3 # copiamos el tercer archivo en mount

./my-fsck virtual-disk

DIFF_FILE3_COPIED=$(diff $SRC/$FILE3 $MOUNT/$FILE3)

if [ "$DIFF_FILE3_COPIED" == "" ]; 
then echo "El tercer archivo es IGUAL que el original"
else echo "El tercer archivo es DIFERENTE al original"
fi

# truncamos el segundo archivo y lo comparamos
truncate --size=+4096 $TEMP/$FILE2
truncate --size=+4096 $MOUNT/$FILE2

./my-fsck virtual-disk

DIFF_MOUNT_TRUNCATED_FILE2=$(diff $SRC/$FILE2 $MOUNT/$FILE2) 
DIFF_TEMP_TRUNCATED_FILE2=$(diff $SRC/$FILE2 $TEMP/$FILE2)

if [ "$DIFF_MOUNT_TRUNCATED_FILE2" == "" ]; 
then echo "El segundo archivo truncado en mount es IGUAL que el original"
else echo "El segundo archivo truncado en mount es DIFERENTE al original"
fi

if [ "$DIFF_TEMP_TRUNCATED_FILE2" == "" ]; 
then echo "El segundo archivo truncado en tmp es IGUAL que el original"
else echo "El segundo archivo truncado en tmp es DIFERENTE al original"
fi
