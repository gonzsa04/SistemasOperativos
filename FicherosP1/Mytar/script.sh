# poner antes chmod +x ./script.txt en la consola para dar permisos para ejecutar el script

if [ ! -e ./mytar ]; then   # miramos si mytar existe y es ejecutable
    echo "Mytar no encontrado"
    exit 1

elif [ ! -x ./mytar ]; then
	echo "Mytar no es ejecutable"
    exit 1
fi

if [ -d "tmp" ]; then
	rm -rf -- tmp           # borramos el directorio tmp si lo encontramos
fi

mkdir tmp                   # creamos nuevo directorio tmp
cd tmp	                    # nos metemos dentro de el

if [ ! -e file1.txt ]; then
	touch file1.txt 
	echo "Hello World!" > file1.txt  # escribimos Hello World! y se lo metemos al file1
fi
if [ ! -e file2.txt ]; then
	touch file2.txt 
	head -10 /etc/passwd > file2.txt # copiamos las 10 primeras líneas del fichero /etc/passwd.
fi
if [ ! -e file3.dat ]; then
	touch file3.dat                  # ponemos un contenido binario aleatorio de 1024 bytes, 
	head -c 1024 /dev/urandom > file3.dat # tomado del dispositivo/dev/urandom
fi

./../mytar -c -f mytar.mtar file1.txt file2.txt file3.dat #creamos el comprimido con los tres archivos

if [ ! -d out ]; then
	mkdir out                        # creamos out en tmp
fi
cp ./mytar.mtar ./out/mytar.mtar     # y copiamos el comprimido en el

cd out
./../../mytar -x -f mytar.mtar       # extraemos el contenido

# comparamos si los nuevos archivos extraidos son iguales que los originales
if diff ../file1.txt file1.txt >/dev/null ; then
	if diff ../file2.txt file2.txt >/dev/null ; then
		if diff ../file3.dat file3.dat >/dev/null ; then
  			../..
  			echo "Correct"           # si todos estan correctos
  			exit 0
  		else                         # si falla uno de ellos
  			../..
  			echo "Uno o mas archivos son diferentes"
			exit 1
		fi
	else
  			../..
  			echo "Uno o mas archivos son diferentes"
			exit 1
	fi	

else
	../..
  	echo "Uno o mas archivos son diferentes"
	exit 1
fi
