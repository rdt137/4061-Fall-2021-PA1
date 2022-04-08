/*test machine: csel-kh1262-12.cselabs.umn.edu
* group number: G2
* name: Andrew Jee, Rijul Mahajan, Roby Thomas
* x500: jeexx014, mahaj068, thom5141 */

#include "myutils.h"

int main(int argc, char *argv[]) {

    if (argc < 7) {
        printf("Less number of arguments.\n");
        printf("./childProgram myDepth parentID startIdx endIdx myDataLen InputFileName\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    // Parse input arguments
    int myDepth = strtol(argv[1], NULL, 10);
    int myID = strtol(argv[2], NULL, 10);
    char* strMyID = argv[2];
    int myStartIdx = strtol(argv[3], NULL, 10);
    int myEndIdx = strtol(argv[4], NULL, 10);
    int myDataLen = strtol(argv[5], NULL, 10);
    char* inputFileName = argv[6];
    char parentID[10];
    
    // Print Parent and current child with associated arguments
    if(myDepth == 1) {
        printf("Parent [%s] - Spawn Child [%s, %s, %s, %s, %s]\n", "master", argv[1], argv[2], argv[3], argv[4], argv[5]);
    } else {
        sprintf(parentID, "%d", myID);
        parentID[strlen(parentID)-1] = '\0';
        printf("Parent [%s] - Spawn Child [%s, %s, %s, %s, %s]\n", parentID, argv[1], argv[2], argv[3], argv[4], argv[5]);
    }
    
    // File pointer
    FILE * fp;                     

    // Line buffer where a new line is stored
    char *line = (char *)malloc(sizeof(char) * LineBufferSize);      

    // The size of the line buffer           
    size_t len = LineBufferSize;              

    // Length of a new line read                                  
    ssize_t nread;                                                              


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

    // Read degrees of each level
    // Note the similarity with the block found in master.c
    int degrees[depth];
    int num;    
    char delimiter[] = " ";

    if(depth > 0)
    {
        // Read next line and write it to line buffer
        if((nread = getLineFromFile(fp, line, len)) != -1) {                    
            int i = 0;
            char *token = strtok(line, delimiter);
            while(i < depth) {
                num = strtol(token, NULL, 10);

                // Check if degree number is valid
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
    }

    // Read input data
    int * input = (int *)malloc(sizeof(int) * nData);
    int aNumber;
    int idxInput = 0;

    while((nread = getLineFromFile(fp, line, len)) != -1) {
        sscanf(line, "%d\n", &aNumber);
        input[idxInput++] = aNumber;
    }

    // exit from input file
    free(line);
    fclose(fp);

	// Spawning child processes, again note the similarity with master.c
    pid_t pid;
    pid_t terminated_pid;
    int temp;
    
    // check for depth to see if children are necessary
    if(myDepth < depth) {                                                       
        for (int i=0; i<degrees[myDepth]; i++) {
            pid = fork();
            
            // lower and higher index for each child of parent
            int lowerIndex, higherIndex;
            lowerIndex = (i * (myDataLen/degrees[myDepth])) + myStartIdx;
            higherIndex = (i + 1) * (myDataLen/degrees[myDepth]) + myStartIdx;

            // separate indeces for last degree
            if(i == degrees[myDepth] - 1)                                       
            {
                lowerIndex = (degrees[myDepth] - 1) * (myDataLen/degrees[myDepth]) + myStartIdx;
                higherIndex = myDataLen + myStartIdx;
            }

            // Convert parameters to string
            char childDepth[10], lIndex[1000], hIndex[1000], id[10], dataLength[1000];
            sprintf(lIndex, "%d", lowerIndex);
            sprintf(hIndex, "%d", higherIndex);
            sprintf(childDepth, "%d", myDepth+1);

            // Correct ID for each child process
            temp = (myID * 10) + i + 1;                                         
            sprintf(id, "%d", temp);
            sprintf(dataLength, "%d", higherIndex - lowerIndex); 

            // child program calls exec with needed arguments
            if (pid == 0) {
                char *args[] = {"childProgram", childDepth, id, lIndex, hIndex, dataLength, inputFileName, NULL}; 
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

	 // Wait for child processes to finish (if created)
    if (pid > 0 && myDepth < depth) {       
    	for (int j=0; j<degrees[myDepth]; j++) {
    		terminated_pid = wait(NULL);
    	}
    }
	 
	 // Create an array of numbers corresponding to the current child
    int arr[myDataLen];
    for (int i = 0; i < myDataLen; i++) {
        arr[i] = input[myStartIdx + i];
    }
    
    // Merge for internal nodes
    if(myDepth < depth) {                                                       
        merge(strMyID, myDepth, degrees[myDepth], myDataLen);
        printf("Process [%s] - Merge Sort - Done\n", strMyID);
    } else {

        // Insertion sort for leaf nodes                                                                    
        insertionSort(arr, 0, myDataLen);
        writeSortedResultToFile(strMyID, arr, myDataLen);
        printf("Process [%s] - Insertion Sort - Done\n", strMyID);
    }    

    return EXIT_SUCCESS;
}
