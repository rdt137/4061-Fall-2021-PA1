/*test machine: csel-kh1262-12.cselabs.umn.edu
* group number: G2
* name: Andrew Jee, Rijul Mahajan, Roby Thomas
* x500: jeexx014, mahaj068, thom5141 */

#include "myutils.h"


int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Less number of arguments.\n");
        printf("./master InputFileName\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    // Please keep this function call in the main.
    // This function initializes "output" folder
    // by deleting the folder and all its files,
    // and then creating a clean "output" folder.
    bookeepingCode();

    // Parse input file
    FILE * fp;                                                                  // File pointer
    char *line = (char *)malloc(sizeof(char) * LineBufferSize);                 // Line buffer where a new line is stored
    size_t len = LineBufferSize;                                                // The size of the line buffer
    ssize_t nread;                                                              // Length of a new line read

    char inputFileName[MaxFileNameLength];
    memset(inputFileName, '\0', MaxFileNameLength);
    sprintf(inputFileName, "input/%s", argv[1]);

    // Open a file and return file pointer to the file
    if ((fp = getFilePointer(inputFileName)) == NULL) {                         
        exit(EXIT_FAILURE);
    }

    // Read the number of data and depth
    int nData;
    int depth = 0;
    
    // Read next line and write it to line buffer
    if((nread = getLineFromFile(fp, line, len)) != -1) {                        
        sscanf(line, "%d %d\n", &nData, &depth);
    }

    // Check if the data is between 1 and 1000 
    if (nData < 1 || nData > 1000) {                                            
        printf("Invalid data - Needs at least one number.\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    // Check if depth is valid
    if (depth > 9 || depth < 0) {                                               
        printf("Depth is not valid - greater than 9 or negative.\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    // Array that stores all degrees
    int degrees[depth];                                                         
    
    // Variable to store each degree
    int num;          

    // Counts the number of leaves
    int totalLeafNodes = 1;                                                    
    
    // Degrees are separated by spaces
    char delimiter[] = " ";                                                     


    if(depth > 0)
    {
        // Read next line and write it to line buffer
        if((nread = getLineFromFile(fp, line, len)) != -1) {                    
            int i = 0;
            char *token = strtok(line, delimiter);
            while(i < depth) {
                num = strtol(token, NULL, 10);
                totalLeafNodes *= num;

                // Check if degree is valid
                if (num > 9 || num < 1) {                                       
                    printf("Degree is not valid - greater than 9 or below 1.\n");
                    fflush(stdout);
                    exit(EXIT_FAILURE);
                }

                degrees[i] = num;
                i++;
                token = strtok(NULL, delimiter);
            }
        }
    } else {
        if((nread = getLineFromFile(fp, line, len)) != -1) {
            depth = 0;
        }
    }

    // Check if there are too many leaf nodes
    if (totalLeafNodes > nData) {                                               
        printf("Too many leaf nodes.\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    // Read input data
    int * input = (int *)malloc(sizeof(int) * nData);
    int aNumber;
    int idxInput = 0;

    while((nread = getLineFromFile(fp, line, len)) != -1) {
        sscanf(line, "%d\n", &aNumber);
        input[idxInput++] = aNumber;
    }

    // exit input file
    free(line);
    fclose(fp);

    // Check if the input data is between 0 and 10000
    for(int i = 0; i < nData; i++) {                                            
        if(input[i] < 0 || input[i] > 10000) {
            printf("Invalid data - Numbers must be between 0 and 10000.\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid;
    pid_t terminated_pid;

    // Spawns child processes and launch childProgram if necessary
    if(depth > 0) {                                                             

        // Spawns children only if the depth is larger than zero
        for (int i=0; i<degrees[0]; i++) {
            pid = fork();
            
            // lower and higher index for each child of parent
            int lowerIndex, higherIndex;
            lowerIndex = i * (nData/degrees[0]);
            higherIndex = (i + 1) * (nData/degrees[0]);

            // separate indeces for last degree (since all other degrees are divided evenly)
            if(i == degrees[0] - 1)                                             
            {
                lowerIndex = (degrees[0] - 1) * (nData/degrees[0]);
                higherIndex = nData;
            }

            // Converts parameters to string
            char lIndex[1000], hIndex[1000], id[2], dataLength[1000];
            sprintf(lIndex, "%d", lowerIndex);
            sprintf(hIndex, "%d", higherIndex);
            sprintf(id, "%d", i+1);
            sprintf(dataLength, "%d", higherIndex - lowerIndex); 

            // Executing childProgram with proper parameters
            if (pid == 0) {            	
                char *args[] = {"childProgram", "1", id, lIndex, hIndex, dataLength, inputFileName, NULL}; 
                execv(*args, args);
                printf("Error executing execv()\n");
                exit(0);
                break;

            }  else if (pid < 0) {
                printf("Failure creating child process (error number: %d)\n", errno);
                break;
            }
        }
    }

    // Waits all child processes to terminate if necessary (if created)
    if (pid > 0 && depth > 0) {
    	for (int j=0; j<degrees[0]; j++) {
    		terminated_pid = wait(NULL);
    	}
    }

    // Merge sort or Insertion sort (if depth is zero)
    if (depth > 0) {
        merge("master", 0, degrees[0], nData);
        printf("Process [master] - Merge Sort - Done\n");
    } else {
        insertionSort(input, 0, nData);
        writeSortedResultToFile("master", input, nData);
        printf("Process [master] - Insertion Sort - Done\n");
    }

    // free input array
    free(input);

    return EXIT_SUCCESS;
}
