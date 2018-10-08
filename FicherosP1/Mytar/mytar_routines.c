#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int copynFile(FILE * origin, FILE * destination, int nBytes)
{
	if (origin == NULL) return -1;     // si el origen no existe damos error

	int c = getc(origin);             // c guarda el primer dato del origen
	int i = 0;

	while (i < nBytes && c != EOF)     // mientras no hayamos copiado los nBytes y c no haya llegado al final
    { 
        putc(c, destination);         // depositamos el contenido de c en el destino
        c = getc(origin);             // c guarda el siguiente dato del origen
        i++;
    }

    if(i < nBytes)return -1;           // si no se han llegado a copiar todos los bytes error
    return i;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char* loadstr(FILE * file)
{
	int longitudNombre = 0;
    char *nombre;                                          // cadena de char que representa el nombre del archivo
    char c;
	                                                       
    while((c = getc(file) != '\0')) longitudNombre++;      // calculamos la longitud del nombre

    nombre =  malloc(sizeof(char) * (longitudNombre + 1)); // reservamos espacio en memoria para el nombre
    fseek(file, -longitudNombre - 1, SEEK_CUR);            // movemos el puntero desde el actual devuelto por malloc
	                                                       // hasta el principio del archivo (- su longitud - 1)

    for (int i = 0; i < longitudNombre + 1; i++) {         // guardamos el nombre del archivo en nombre,
        nombre[i] = getc(file);                            // que ya tiene memoria reservada por malloc
    }    

	return nombre;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry* readHeader(FILE * tarFile, int *nFiles)
{
	int tam = 0;
    stHeaderEntry *header = NULL;                       // array que contendra tamaño y nombre de cada archivo

	fread(nFiles, sizeof(int), 1, tarFile);             // leemos el numero de archivos de tarFile

    header = malloc(sizeof(stHeaderEntry)*(*nFiles));   // reservamos memoria para header (lo que ocupe el struct
                                                        // x todos los archivos que haya)
    for (int i = 0; i < nFiles; i++) {                  // para cada archivo guardamos en su struct correspondiente
        fread(&tam, sizeof(int), 1, tarFile);           // su tamaño y nombre
        header[i].size = tam;
		header[i].name = loadstr(tarFile);
    }

	return header;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int createTar(int nFiles, char *fileNames[], char tarName[])
{
	FILE * entrada;                                  // leemos
    FILE * salida;                                   // escribimos

    int bytes = 0, headerBytes = 0;                  // tamaño que ocupara cada archivo y y el cabecero del principio
    stHeaderEntry *header; 

    header = malloc(sizeof(stHeaderEntry) * nFiles); // reservamos memoria para el header

	// DETERMINAMOS CUANTO OCUPARA LA CABECERA
    headerBytes += sizeof(int);                      // lo que ocupa nFiles (size de un int)
    headerBytes += nFiles*sizeof(int);               // mas el tamaño de cada archivo (size de un int)

    for (int i=0; i < nFiles; i++) {                 // mas el nombre de cada archivo:
        headerBytes+=strlen(fileNames[i])+1;         // size del nombre de cada archivo + '\0'
    }

	// RELLENAMOS HEADER CON NOMBRES Y TAMAÑOS Y ESCRIBIMOS LOS DATOS DE LOS ARCHIVOS EN LA SECCION DATA
    salida =  fopen(tarName, "w");                   // creamos un archivo vacio (w->writing) con el nombre deseado
    fseek(salida, headerBytes, SEEK_SET);            // hacemos que el puntero de salida apunte a la seccion
	                                                 // de datos, avanzando desde el principio lo que vaya a ocupar el cabecero

    for (int i = 0; i < nFiles; i++) {               // para cada archivo copiamos en bytes el maximo que podamos
		entrada = fopen(fileNames[i], "r");
        bytes = copynFile(entrada, salida, INT_MAX); // asi nos aseguramos de haber escrito todo el contenido en salida
        header[i].size = bytes;                      // ademas nos devuelve su tamaño
        header[i].name = malloc(sizeof(fileNames[i]) + 1); // hacemos hueco para el nombre con lo que ocupe + '\0'
        strcpy(header[i].name, fileNames[i]);        // copiamos el nombre del archivo
    
    if (fclose(entrada) == EOF) return EXIT_FAILURE; // si no es posible cerrar el archivo devolvemos error
    }
	
	// ESCRIBIMOS LA CABECERA
	fseek(salida, 0, SEEK_SET);                      // movemos el puntero de salida al principio otra vez
    fwrite(&nFiles, sizeof(int), 1, salida);         // escribimos el numero de archivos
    
	// para cada archivo escribimos su nombre y tamaño
    for (int i = 0; i < nFiles; i++) {
        fwrite(header[i].name, strlen(header[i].name)+1, 1, salida);
        fwrite(&header[i].size, sizeof(unsigned int), 1, salida); 
    }
    
	// LIBERAMOS Y CERRAMOS	
	// liberamos los nombres de los archivos de header
    for (int i = 0; i < nFiles; i++) {
        free(header[i].name); 
    }

    free(header);          // liberamos header

    if (fclose(salida) == EOF) return (EXIT_FAILURE); // si no podemos cerrar el archivo devolvemos error

    return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int extractTar(char tarName[])
{
	FILE *tarFile = NULL; 
    FILE *final = NULL;
    stHeaderEntry *header;
    int nFiles = 0, bytes = 0;                      // numero de archivos y tamaño que ocupara cada uno

	tarFile = fopen(tarName, "r");
	header = readHeader(tarFile, &nFiles);          // obtenemos header y nFiles

    for (int i = 0; i < nFiles; i++) {
        if ((final = fopen(header[i].name, "w")) == NULL) return EXIT_FAILURE; // si no podemos crear el archivo devolvemos error
        else bytes = copynFile(tarFile, final, header[i].size);    // copiamos todo el contenido de cada header en final
		if (bytes == -1) return EXIT_FAILURE;                      // si bytes es -1 significaba error
        if(fclose(final) != 0) return EXIT_FAILURE;                // si no podemos cerrar el archivo, devolvemos error
    }

	// liberamos los nombres de los archivos de header
    for (int i = 0; i <nFiles; i++) {
        free(header[i].name);
    }

    free(header);     // liberamos header

    if (fclose(tarFile) == EOF) return (EXIT_FAILURE); // si no podemos cerrar el archivo devolvemos error

    return (EXIT_SUCCESS);
}
