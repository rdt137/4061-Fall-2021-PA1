/*test machine: csel-kh1262-12.cselabs.umn.edu
* group number: G2
* name: Andrew Jee, Rijul Mahajan, Roby Thomas
* x500: jeexx014, mahaj068, thom5141 */

#include "myutils.h"
#include <limits.h>

void printArray(int arr[], int size) {
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

// writes associated array to file along with the nuumber of data
void writeSortedResultToFile(char* myID, int arr[], int size) {
    char line[WriteBufferSize];
    char filepath[MaxFileNameLength];
    memset(line, '\0', WriteBufferSize);
    memset(filepath, '\0', MaxFileNameLength);
    sprintf(filepath, "output/%s.out", myID);

    int i;
    int idx = 0;
    idx += sprintf(&line[idx], "%d\n", size);
    for (i = 0; i < size; i++) {
        idx += sprintf(&line[idx], "%d\n", arr[i]);
    }
    // Write data in the write buffer to the file
    writeLineToFile(filepath, line);                                            
}

// Insertion Sort
void insertionSort(int arr[], int low, int high) {
    int i, j, key;
    for(int i = low+1; i < high; i++)
    {
        key = arr[i];
        j = i - 1;

        while(j >= low && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}


// Multiway Merge Sort with multiple data streams from intermediate files
void merge(char* myID, int depth, int nChild, int size) {
    // Line buffer where a new line is stored
    char *line = (char *)malloc(sizeof(char) * LineBufferSize);                 
    size_t len = LineBufferSize;
    ssize_t nread; 

    int arr[size];

    FILE *fpt_child[nChild];
    char buffer[LineBufferSize];
    
    int ID = 0;
    char childID[10];
    if (depth > 0) {
        ID = strtol(myID, NULL, 10);
    }

    // Compute the length of last child file
    int child_length = size / nChild;

    // floor(M/K)                                           
    int last_child_length = child_length;

    // If the numbers are not evenly distributed.
    if (size % nChild != 0) {        
        // residual data                                           
        last_child_length = size - ((nChild - 1) * child_length);               
    }

    // Array to contain each array
    int *child_arr[nChild];

    for (int n=0; n<nChild-1; n++) {
        child_arr[n] = (int *) malloc(sizeof(int) * child_length);
    }

    child_arr[nChild - 1] = (int *) malloc(sizeof(int) * last_child_length);

    int index = 0;
    int aNumber;

    int temp;
    for (int i = 0; i < nChild; i++) {
        temp = (ID * 10) + i + 1;

        // Open each child file
        char inputFileName[MaxFileNameLength];

        sprintf(childID, "%d", temp);
        memset(inputFileName, '\0', MaxFileNameLength);
        sprintf(inputFileName, "output/%s.out", childID);

        // Open a file and return file pointer to the file
        if ((fpt_child[i] = getFilePointer(inputFileName)) == NULL) {           
            exit(EXIT_FAILURE);
        }

        if ((nread = getLineFromFile(fpt_child[i], line, len)) != -1) { 
            // The first line denotes the amount of numbers in the file,
            // so it should be ignored, as it is not the number we want to be sorting.
            sscanf(line, "%d\n", &aNumber);
        }

        index = 0;
        while((nread = getLineFromFile(fpt_child[i], line, len)) != -1) {       
            // Fill up each child_arr
            sscanf(line, "%d\n", &aNumber);
            child_arr[i][index] = aNumber;
            index += 1;
        }        
    }
    
    // Merge all integers into a single array
    int child_arr_index[nChild];                                                
    
    // Keep track of progress in each array
    for (int ind=0; ind < nChild; ind++) {child_arr_index[ind] = 0;}
    int minimum;
    int min_arr = 0;
    
    // Essentially, find the minimum in a linear fashion
    for (int a = 0; a < size; a++) {                                            
        minimum = INT_MAX;

        for (int b = 0; b < nChild-1; b++) {
            if (child_arr_index[b] < child_length) {
                if (minimum > child_arr[b][child_arr_index[b]]) {
                    minimum = child_arr[b][child_arr_index[b]];
                    min_arr = b;
                }
            }
        }
        
        // Since the last file has a different length than other files,
        // it needs a special condition check.
        if (child_arr_index[nChild-1] < last_child_length && minimum > child_arr[nChild-1][child_arr_index[nChild-1]]) {
            minimum = child_arr[nChild-1][child_arr_index[nChild-1]];
            min_arr = nChild-1;
        }
        arr[a] = minimum;
        child_arr_index[min_arr] += 1;
    }
    
    writeSortedResultToFile(myID, arr, size);

    free(line);
    

    for (int child=0; child < nChild; child++) {
        free(child_arr[child]);
    }

    for (int k=0; k<nChild; k++) {
        fclose(fpt_child[k]);
    }

}
