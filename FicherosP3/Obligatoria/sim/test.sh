directorio="resultados"

# Preguntamos al usuario por un archivo valido para la simulacion

Archivo=""
valido=false

while [ "$valido" == "false" ]
do
	echo "Archivo: "
	read nombre

	Archivo=("./examples/"$nombre".txt")
	
	if [ -f "$Archivo" ]
	then
	valido=true
	else
		echo "$Archivo no encontrado"
	fi
done

# Creamos el directorio de los resultados

if [ -d "$directorio" ]; then
	# Lo destruimos si ya existe
	rm -rf -- $directorio  
fi

mkdir $directorio # Creamos uno vacio
 

./sim $Archivo $directorio"/""sim.log"

cd "../gantt"

./generate_gantt_chart "../sim/"$directorio"/""sim.log"

exit 0