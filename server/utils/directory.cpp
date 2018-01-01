#include <directory.h>

void saveFile(char *filename)
{
	int counter;
	FILE *ptr_myfile;
	ptr_myfile=fopen(filename,"wb");
	if (!ptr_myfile)
	{
		printf("Unable to open file!");
		return;
	}
	for ( counter=0; counter < 100; counter++)
	{
		printf("writing %d\n", counter);
		fwrite(&counter, sizeof(int), 1, ptr_myfile);
	}
	fclose(ptr_myfile);

}

int readFile(char *filename, char * buffer, long *bufferSize)
{
	FILE *ptr_myfile;

	ptr_myfile=fopen(filename,"rb");
	if(!ptr_myfile)
	{
		return 0;
	}

	//get flie size
	fseek(ptr_myfile, 0, SEEK_END);
	long filelen = ftell(ptr_myfile);
	rewind(ptr_myfile);


	buffer = (char *)malloc((filelen+1)*sizeof(char));
	fread(buffer, filelen, 1, ptr_myfile);
	*bufferSize = filelen;

	fclose(ptr_myfile);

	return 1;
}

//read file
/*
 * 			char *buffer = NULL;
			char * filename = "file.bin";
			long bufferSize = 0;
			if(readFile(filename, buffer, &bufferSize))
			{
				printf("File, read %ld bytes.\n", bufferSize);
				buffer = "asdasd";
				//buffer[0] = buffer[1] = buffer[2] = buffer[3] = (char)0;
				//printf("Buffer[0] >%d<\n", buffer[0]);
				sendResponse(th_data->socketDescriptor, buffer, 100);
			}else
			{
				perror("Unable to read file");
				return 0;

			}
 *
 *
 *
 *
 */

void listFiles()
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir ("root")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			if(ent->d_type != 4)
				printf ("%s, %d\n", ent->d_name, ent->d_type);
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("Directory error");
	}
}

int isFileExist(char *file)
{
	FILE *fil = fopen(file, "r");
	if(fil == NULL)
	{
		return -1;
	}
	return 1;
}
