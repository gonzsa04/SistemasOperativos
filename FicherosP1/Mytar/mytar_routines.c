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
	if (origin == NULL) return -1;

	int c = fgetc(origin);
	int i = 0;

	while (i < nBytes && c != EOF) 
    { 
        fputc(c, destination); 
        c = fgetc(origin); 
        i++;
    }

    if(i < nBytes)return -1;
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
    char *nombre;
    char c;

    while((c = getc(file) != '\0')) { //hasta que llegue al final del nombre
        longitudNombre++; //aumentamos la longitud
    }

    nombre =  malloc(sizeof(char) * (longitudNombre + 1)); //reserva espacio en memoria para el nombre
    fseek(file, -(longitudNombre + 1), SEEK_CUR); //apunta al principio del nombre

    for (int i = 0; i < longitudNombre+1; i++) {
        nombre[i] = getc(file); //guardamos el nombre del archivo en nombre
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
    stHeaderEntry *header=NULL; 

    header=malloc(sizeof(stHeaderEntry)*(*nFiles)); //reservamos memoria para header
   
    for (int i = 0; i < nFiles; i++) {
        fread(&tam, sizeof(int), 1, tarFile); 
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
	FILE * entrada; //Creacion de variables
    FILE * salida;

    int bytes = 0, headerBytes = 0;
    stHeaderEntry *header; 

    header = malloc(sizeof(stHeaderEntry) * nFiles); // reserva memoria para el header
    headerBytes += sizeof(int); 
    headerBytes += nFiles*sizeof(unsigned int); 

    for (int i=0; i < nFiles; i++) {
        headerBytes+=strlen(fileNames[i])+1; 
    }

    salida =  fopen(tarName, "w"); // Abrimos el archivo
    fseek(salida, headerBytes, SEEK_SET); //hacemos que el puntero apunte al principio del archivo

    for (int i=0; i < nFiles; i++) {
        bytes = copynFile(entrada, salida, INT_MAX);
        header[i].size = bytes; 
        header[i].name = malloc(sizeof(fileNames[i]) + 1); 
        strcpy(header[i].name, fileNames[i]);  
    
    if (fclose(entrada) == EOF) return EXIT_FAILURE; // si no es posible cerrar el archivo devolvemos error
    }
	
    fwrite(&nFiles, sizeof(int), 1, salida); 
    

    for (int i = 0; i < nFiles; i++) {
        fwrite(header[i].name, strlen(header[i].name)+1, 1, salida);
        fwrite(&header[i].size, sizeof(unsigned int), 1, salida); 
    }
    
    for (int i=0; i < nFiles; i++) {
        free(header[i].name); 
    }

    free(header);

    if (fclose(salida) == EOF) { return (EXIT_FAILURE); } //intentamos cerrar elarchivo, si falla devolvemos error

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
    int nFiles = 0, bytes = 0;

    for (int i = 0; i < nFiles; i++) {

        if ((final = fopen(header[i].name, "w")) == NULL) { return EXIT_FAILURE; } // si no podemos crear el archivo devolvemos error
        else {
            bytes = copynFile(tarFile, final, header[i].size);     
    }  
        
        if(fclose(final) != 0) { return EXIT_FAILURE; } // si no podemos cerrar el archivo, devolvemos error
    }

    for (int i = 0; i <nFiles; i++) {
        free(header[i].name);
    }

    free(header);
    if (fclose(tarFile) == EOF) { return (EXIT_FAILURE); }

    return (EXIT_SUCCESS);
}
