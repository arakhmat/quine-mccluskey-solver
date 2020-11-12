#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Number of rows contained at the table that is stored in group[x][x]
#define numRows 100

// Opens and parses the file
int readFile(char* filename, int *onSet, int *dontCare, int* numOnSet, int* numDontCare);

// Parses the minterm and don't care lines
void parseTerms(char* token, int* term, int* numTerm);

// Mallocs first two dimensions of group
void mallocGroup(int**** group, int numVar);

// Checks if the number has base of two
int isPowerOfTwo(int num);

// Takes minterms and don't cares and puts them in group[0] accordingly with the number of ones
void groupTerms(int* all, int numAll, int**** group, int numVar);

// Finds prime implicants using Quine-McCluskey technique and then converts them to appropriate strings
int findPrimeImpl(int**** group, int** prImp, int numVar);

/* Transforms minterms from integer representation to string
   For example (0, 1, 2, 3) will be 00-- */
void primeImplToString(int**** group, char** allPrImp, int numVar);

/* Creates prime implicant table*/
char** createPrimeTable(int** prImp, int numImpl, int* onSet, int numOnSet);

// Finds the best solution for the program.
int simplify(int** prImp, int numImpl, int* onSet, int numOnSet, char** implicant, int numVar, int n, char*** primeTable);

// Converts the decimal number into an appropriate string 
char* decToBin(int dec, int numVar);

// Saves the file to the correct format
void saveFile(char* filename, char** primeImplicant, int* onSet, int* dontCare,
        int numVar, int numImpl, int numOnSet, int numDontCare, char** primeTable, char** allPrImp, int numPrimeImpl);


int readFile(char* filename, int *onSet, int *dontCare, int* numOnSet, int* numDontCare) {

    int numVar = 0;
    char line[101];
    char* token;

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file\n");
        exit(0);
    }

    while (!feof(file)) {

        fgets(line, 100, file);
        line[strlen(line)-1] = '\0';

        if (strstr(line, ".i") != NULL) {
            token = strtok(line, " ");
            token = strtok(NULL, " ");
            numVar = atoi(token);
            continue;
        } else if (strstr(line, ".m") != NULL) {
            fgets(line, 100, file);

            if (line[strlen(line)-1] == '\n')
                line[strlen(line)-1] = ' ';
            strcat(line, " "); // To make sure that all terms are saved

            token = strtok(line, " ");
            parseTerms(token, onSet, numOnSet);
            continue;
        } else if (strstr(line, ".d") != NULL) {
            fgets(line, 100, file);
            if (line[strlen(line)-1] == '\n')
                line[strlen(line)-1] = ' ';
            strcat(line, " "); // To make sure that all terms are saved
            if (line == NULL) return numVar;
            if (strcmp(line, "") == 0) return numVar;
            strcat(line, " "); // To make sure that all terms are saved
            token = strtok(line, " ");
            if (token == NULL) return numVar;
            if (strcmp(token, "") == 0) return numVar;
            parseTerms(token, dontCare, numDontCare);
            return numVar;
        }

        
    }
    fclose(file);

    return numVar;
}

void parseTerms(char* token, int* term, int* numTerm) {

    int i = 0;
    
    (*numTerm) = 0;

    if (strstr(token,"\n") == NULL && ((token[0] < '0' || token[0] > '9') || token[0] == 13)) { // 13 is CR in ASCII
        (*numTerm) = 0;
        return;
    }
    term[0] = atoi(token);
    for (i = 1; i < 100; i++) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            (*numTerm) = i;
            break;
        }
        if (token[0] == 13) {
            (*numTerm) = i;
            break;
        }
        if(strstr(token,"\n") != NULL){
            (*numTerm) = i;
            term[i] = atoi(token);
            break;
        }
        term[i] = atoi(token);
    }
    return;

}

char* decToBin(int dec, int numVar) {
    char bin[15];
    char *result;
    result = malloc(sizeof (char)*15);
    int temp = 0;
    int pos = 0;
    int i = 0;

    while (dec != 0) {
        temp = dec % 2;
        bin[pos] = temp + '0';
        dec /= 2;
        pos++;
    }
    while (pos < numVar) {
        bin[pos] = '0';
        pos++;
    }

    for (i = 0; i < numVar; i++) {
        result[i] = bin[pos - 1 - i];
    }
    result[numVar] = '\0';

    return result;
}

void mallocGroup(int**** group, int numVar) {

    int i = 0, j = 0;

    if (group == NULL) {
        printf("group\n malloc fail");
        exit(0);
    }

    for (i = 0; i < numVar; i++) {
        group[i] = (int***) malloc(sizeof (int**)*numVar - i);
        if (group[i] == NULL) {
            printf("group[%d] malloc fail\n", i);
            exit(0);
        }
        for (j = 0; j < numVar - i; j++) {
            group[i][j] = (int**) malloc(sizeof (int*)*numRows);
            if (group[i][j] == NULL) {
                printf("group[%d][%d] malloc fail\n", i, j);
                exit(0);
            }
        }
    }
}

void groupTerms(int* all, int numAll, int**** group, int numVar) {
    int i = 0, j = 0, k = 0;
    char temp[32];
    int ones = 0;

    // Loops through all of the minterms and don't cares
    for (i = 0; i < numAll; i++) {
        ones = 0;
        strcpy(temp, decToBin(all[i], numVar));
        for (j = 0; j < numVar; j++) {
            if (temp[j] == '1') {
                ones++;
            }
        }
        for (k = 0; k < numRows; k++) {
            if (group[0][ones][k] == NULL) {
                group[0][ones][k] = (int*) malloc(sizeof (int) * 2);
                if (group[0][ones][k] == NULL) {
                    printf("group[%d] malloc fail\n", i);
                    exit(0);
                }
                group[0][ones][k][0] = all[i];
                // Last integer is equal to 0, meaining that this prime implicant was not yet used to combine into a new one
                group[0][ones][k][1] = 0;
                break;
            }
        }
    }
}

/* True is  0
   False is 1*/
int isPowerOfTwo(int num) {
    // Special case because 2^num will never be 0
    if (num == 0) {
        return 1;
    }/* As an example 0100 & 0011 will give 0 which is true
         * and 0011 & 0010 will give 0010 which is not true*/
    else if ((num & (num - 1)) == 0) {
        return 0;
    }
    return 1;
}

int findPrimeImpl(int**** group, int** prImp, int numVar) {

    // Stores the difference between the minterms and don't cars of two prime implicants that have Hamming distance of 1
    int diff[16];
    /* Stores the newly found implicant in order to compare it with the existing ones.
     * Saves the program from repeating the same implicant twice*/
    int temp[16];
    int success = 0;
    int repeat = 0;

    int i = 0, j = 1, k = 0, l = 0;
    int m = 0, n = 0;
    int col = 0;

    int iter_1 = 0, iter_2 = 0, iter_main = 0;

    for (i = 0; i < numVar; i++) {
        for (j = 1; j < numVar - i; j++) {
            for (k = 0; k < numRows; k++) {
                if (group[i][j][k] != NULL) {
                    for (l = 0; l < numRows; l++) {
                        if (group[i][j - 1][l] != NULL) {
                            success = 0;

                            //Finds the difference between two prime implicants and makes sure that it is a power of two
                            diff[0] = group[i][j][k][0] - group[i][j - 1][l][0];
                            if ((isPowerOfTwo(diff[0]) == 0) && (diff[0] > 0)) {
                                success++;
                            }
                            /* For implicants that have more than 1 term it also checks if the differences
                             * between the terms (in order from smallest to largest for both) of two implicants is the same
                             * As an example, the following will group:
                             * 0  1  2  3
                             * 8  9  10 11
                             * */
                            for (n = 1; n < (int) pow(2, i); n++) {
                                diff[n] = group[i][j][k][n] - group[i][j - 1][l][n];
                                if ((diff[n] == diff[n - 1]) && (isPowerOfTwo(diff[n]) == 0) && (diff[n] > 0)) {
                                    success++;
                                }
                            }

                            // Success is only equal to the 2^i if all the differences are the same and are power of two
                            if (success == (int) pow(2, i)) {

                                /* The group[x][x][x][x] has one extra integer at the end. That integer is equal to 0 if it's a prime implicant
                                 * otherwise it is to 1 */
                                group[i][j][k][(int) pow(2, i)] = 1;
                                group[i][j - 1][l][(int) pow(2, i)] = 1;

                                iter_1 = 0;
                                iter_2 = 0;
                                // Stores the temporary result and makes sure that the values go from the smallest to the biggest
                                for (iter_main = 0; iter_main < (int) pow(2, i + 1); iter_main++) {
                                    if ((group[i][j][k][iter_1] < group[i][j - 1][l][iter_2] && iter_1 != ((int) pow(2, i)))
                                            || iter_2 == ((int) pow(2, i))) {
                                        temp[iter_main] = group[i][j][k][iter_1];
                                        iter_1++;
                                    } else {
                                        temp[iter_main] = group[i][j - 1][l][iter_2];
                                        iter_2++;
                                    }
                                }

                                m = 0;
                                // Checks if the temporary result already exists. If it does not, then adds it.
                                while (group[i + 1][j - 1][m] != NULL) {
                                    success = 0;
                                    for (col = 0; col < (int) pow(2, i + 1); col++) {
                                        if (temp[col] == group[i + 1][j - 1][m][col]) {
                                            success++;
                                        }
                                    }
                                    if (success == (int) pow(2, i + 1)) {
                                        repeat = 1;
                                        break;
                                    }
                                    m++;
                                }
                                if (repeat == 1) {
                                    repeat = 0;
                                    continue;
                                }

                                m = 0;
                                //Adds the result
                                while (group[i + 1][j - 1][m] != NULL) {
                                    m++;
                                }
                                group[i + 1][j - 1][m] = (int*) malloc(sizeof (int)*(((int) pow(2, i + 1)) + 1));
                                // Sets last integer equal to 0 intially
                                group[i + 1][j - 1][m][(int) pow(2, i + 1)] = 0;
                                for (col = 0; col < (int) pow(2, i + 1); col++) {
                                    group[i + 1][j - 1][m][col] = temp[col];
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // The following loops go through all the implicants and identifies the ones that were not combined.
    // Then, it puts all the prime implicant in the same table.
    m = 0;
    for (i = 0; i < numVar; i++) {
        for (j = 0; j < numVar - i; j++) {
            for (k = 0; k < numRows; k++) {
                if (group[i][j][k] != NULL) {
                    if (group[i][j][k][(int) pow(2, i)] == 0) {
                        prImp[m] = malloc(sizeof (int)*16);
                        prImp[m][0] = (int) pow(2, i);
                        for (n = 1; n < (int) pow(2, i) + 1; n++) {
                            prImp[m][n] = group[i][j][k][n - 1];
                        }
                        m++;
                    }
                }
            }
        }
    }
    return m;

}

void primeImplToString(int**** group, char** primeImplicant, int numVar) {

    int difference = 0;

    int i = 0, j = 1, k = 0;
    int m = 0, n = 0;

    for (i = 0; i < numVar; i++) {
        for (j = 0; j < numVar - i; j++) {
            for (k = 0; k < numRows; k++) {
                if (group[i][j][k] != NULL) {
                    if (group[i][j][k][(int) pow(2, i)] == 0) {
                        n = 0;
                        while (primeImplicant[n] != NULL) {
                            n++;
                        }
                        primeImplicant[n] = (char*) malloc(sizeof (char)*numVar);
                        strcpy(primeImplicant[n], decToBin(group[i][j][k][0], numVar - 1));
                        for (m = 1; m < (int) pow(2, i) + 1; m++) {
                            difference = group[i][j][k][m] - group[i][j][k][0];
                            if (isPowerOfTwo(difference) == 0) {
                                difference = (int) log2(difference);
                                primeImplicant[n][numVar - 2 - difference] = '-';
                            }
                        }
                    }
                }
            }
        }
    }
}

int simplify(int** prImp, int numImpl, int* onSet, int numOnSet, char** implicant, int numVar, int n, char*** primeTable) {
    int i = 0, j = 0, k = 0, l = 0;
    int m = 0;
    int success = 0;

    int isEssent = 0;
    int essent = 0;

    int temp = 0;
    temp = numImpl;

    int same = 0;
    int match = 0;
    int countI = 0;
    int countL = 0;

    int lenI = 0;
    int lenJ = 0;

    static int recursionDepth = 0;

    //Gets rid of the primeImplicants without minterms
    for (j = 0; j < numImpl; j++) {
        match = 0;
        for (k = 1; k < prImp[j][0] + 1; k++) {
            for (i = 0; i < numOnSet; i++) {
                if (prImp[j][k] == onSet[i]) {
                    match++;
                }
            }
        }
        if (match == 0) {
            numImpl--;
            for (k = j; k < numImpl; k++) {
                for (l = 0; l < prImp[k + 1][0] + 1; l++) {
                    prImp[k][l] = prImp[k + 1][l];
                }
            }
            j = -1;
        }
    }

    /* Prime implicant table is created here because the loop above needs to run first,
       in order to remove the prime implicants with only don't cares in them
       Also, the prime table should only be generated before the first iteration of simplify function*/
    if (recursionDepth == 0) {
        (*primeTable) = createPrimeTable(prImp, numImpl, onSet, numOnSet);
        recursionDepth++;
    }

    /* Finds essential prime implicants.
     * Saves them in char* array and removes them form prime implicant table*/
    for (i = 0; i < numOnSet; i++) {
        essent = 0;
        isEssent = 0;
        for (j = 0; j < numImpl; j++) {
            for (k = 1; k < prImp[j][0] + 1; k++) {
                if (onSet[i] == prImp[j][k]) {
                    isEssent++;
                    essent = j;
                }
            }
        }
        if (isEssent == 1) {
            for (j = 1; j < prImp[essent][0] + 1; j++) {
                for (k = 0; k < numOnSet; k++) {
                    if (prImp[essent][j] == onSet[k]) {
                        numOnSet--;
                        for (m = k; m < numOnSet; m++) {
                            onSet[m] = onSet[m + 1];
                        }
                    }
                }
            }
            implicant[n] = decToBin(prImp[essent][1], numVar);
            for (m = 2; m < prImp[essent][0] + 1; m++) {
                success = prImp[essent][m] - prImp[essent][1];
                if (isPowerOfTwo(success) == 0) {
                    success = (int) log2(success);
                    implicant[n][numVar - 1 - success] = '-';
                }
            }
            numImpl--;
            for (j = essent; j < numImpl; j++) {
                for (k = 0; k < prImp[j + 1][0] + 1; k++) {
                    prImp[j][k] = prImp[j + 1][k];
                }
            }
            n++;
            i = -1;
        }
    }

    /* Checks for column dominance, removes the dominating minterms.*/
    for (i = 0; i < numOnSet; i++) {
        for (l = 0; l < numOnSet; l++) {
            match = 0;
            if (i == l) {
                continue;
            }
            countI = 0, countL = 0;
            for (j = 0; j < numImpl; j++) {
                same = 0;
                for (k = 1; k < prImp[j][0] + 1; k++) {
                    if (prImp[j][k] == onSet[i]) {
                        same++;
                        countI++;
                    }
                    if (prImp[j][k] == onSet[l]) {
                        same++;
                        countL++;
                    }
                }
                if (same == 2) {
                    match++;
                }
            }
            if (match == countL) {
                numOnSet--;
                for (m = i; m < numOnSet; m++) {
                    onSet[m] = onSet[m + 1];
                }
                i = -1;
            } else if (match == countI) {
                numOnSet--;
                for (m = l; m < numOnSet; m++) {
                    onSet[m] = onSet[m + 1];
                }
                l = -1;
            }
        }
    }
    // Gets rid of prime implicants that do not match any of the minterms that were left after column dominance
    for (j = 0; j < numImpl; j++) {
        match = 0;
        for (k = 1; k < prImp[j][0] + 1; k++) {
            for (i = 0; i < numOnSet; i++) {
                if (prImp[j][k] == onSet[i]) {
                    match++;
                }
            }
        }
        if (match == 0) {
            numImpl--;
            for (k = j; k < numImpl; k++) {
                for (l = 0; l < prImp[k + 1][0] + 1; l++) {
                    prImp[k][l] = prImp[k + 1][l];
                }
            }
            j = -1;
        }
    }

    /*Performs row dominance and removes the dominated prime implicants.*/
    for (i = 0; i < numImpl; i++) {
        for (j = 0; j < numImpl; j++) {
            if (i == j)
                continue;
            same = 0;
            lenI = 0, lenJ = 0;
            for (m = 0; m < numOnSet; m++) {
                for (l = 1; l < prImp[j][0] + 1; l++) {
                    if (prImp[j][l] == onSet[m]) {
                        lenJ++;
                    }
                }
                for (k = 1; k < prImp[i][0] + 1; k++) {
                    if (prImp[i][k] == onSet[m]) {
                        lenI++;
                    }
                    for (l = 1; l < prImp[j][0] + 1; l++) {
                        if (prImp[i][k] == prImp[j][l] && prImp[i][k] == onSet[m]) {
                            same++;
                        }
                    }
                }
            }

            int toRemove = 0;
            if (same != 0) {
                if (same == lenI) {
                    toRemove = i;
                    if (lenI == lenJ) {
                        if (prImp[j][0] > prImp[i][0]) {
                            toRemove = j;
                        }
                    }
                    numImpl--;
                    for (k = toRemove; k < numImpl; k++) {
                        for (l = 0; l < prImp[k + 1][0] + 1; l++) {
                            prImp[k][l] = prImp[k + 1][l];
                        }
                    }
                } else if (same == lenJ) {
                    numImpl--;
                    toRemove = j;
                    for (k = toRemove; k < numImpl; k++) {
                        for (l = 0; l < prImp[k + 1][0] + 1; l++) {
                            prImp[k][l] = prImp[k + 1][l];
                        }
                    }
                }
            }
        }
    }

    /* Recursion is stopped if there are no more minterms and implicants.
     * It also stops if temp equals to numImpl*/
    if (numOnSet > 0 && numImpl > 0 && numImpl != temp){
        n = simplify(prImp, numImpl, onSet, numOnSet, implicant, numVar, n, primeTable);
    }
    
    return n;
}

char** createPrimeTable(int** prImp, int numImpl, int* onSet, int numOnSet) {
    int i = 0, j = 0, k = 0;
    int max = 0;
    int isX = 0;

    char** primeTable = (char**) malloc(sizeof (char*)*numImpl + 2);
    char integer[10];

    for (i = 0; i < numImpl + 2; i++) {
        primeTable[i] = (char*) malloc(sizeof (char)*150);
    }

    for (i = 0; i < numImpl; i++) {
        if (max < prImp[i][0]) {
            max = prImp[i][0];
        }
    }
    
    sprintf(integer, "%d\n", numImpl);
    strcpy(primeTable[0], integer);
    
    strcpy(primeTable[1], "");
    for (i = 0; i < max * 4; i++) {
        strcat(primeTable[1], " ");
    }

    for (i = 0; i < numOnSet; i++) {
        sprintf(integer, "%2d ", onSet[i]);
        strcat(primeTable[1], integer);
    }
    strcat(primeTable[1], "\n");

    for (i = 0; i < numImpl; i++) {
        strcpy(primeTable[i + 2], "(");
        for (j = 1; j < prImp[i][0] + 1; j++) {
            if (j < prImp[i][0]) {
                sprintf(integer, "%2d, ", prImp[i][j]);
            } else {
                sprintf(integer, "%2d", prImp[i][j]);
            }
            strcat(primeTable[i + 2], integer);
        }
        strcat(primeTable[i + 2], ")");
        for (j = 4 * prImp[i][0]; j < max * 4; j++) {
            strcat(primeTable[i + 2], " ");
        }
        for (j = 0; j < numOnSet; j++) {
            isX = 0;
            for (k = 1; k < prImp[i][0] + 1; k++) {
                if (prImp[i][k] == onSet[j]) {
                    isX = 1;
                }
            }
            if (isX == 1) {
                strcat(primeTable[i + 2], " X ");
            } else {
                strcat(primeTable[i + 2], " | ");
            }
        }
        strcat(primeTable[i + 2], "\n");
    }

    return primeTable;
}

void saveFile(char* filename, char** primeImplicant, int* onSet, int* dontCare,
        int numVar, int numImpl, int numOnSet, int numDontCare, char** primeTable, char** allPrImp, int numPrimeImpl) {

    int i = 0, j = 0;

    FILE* file = fopen(filename, "w");

    char line[150];
    char integer[10];
    char literal[3];

    if (file == NULL) {
        printf("Error opening file\n");
        exit(0);
    }

    strcpy(line, ".i ");
    sprintf(integer, "%d", numVar);
    strcat(line, integer);
    strcat(line, "\n");
    fputs(line, file);

    strcpy(line, ".m\n");
    fputs(line, file);


    strcpy(line, "");
    for (i = 0; i < numOnSet; i++) {
        sprintf(integer, "%d", onSet[i]);
        strcat(line, integer);
        strcat(line, " ");
    }
    strcat(line, "\n");
    fputs(line, file);

    strcpy(line, ".d\n");
    fputs(line, file);

    strcpy(line, "");
    for (i = 0; i < numDontCare; i++) {
        sprintf(integer, "%d", dontCare[i]);
        strcat(line, integer);
        strcat(line, " ");
    }
    strcat(line, "\n");
    fputs(line, file);
    
    strcpy(line, ".p ");
    sprintf(integer, "%d", numPrimeImpl);
    strcat(line, integer);
    strcat(line, "\n");
    fputs(line, file);

    for (i = 0; i < numPrimeImpl; i++) {
        strcpy(line, allPrImp[i]);
        strcat(line, "\n");
        fputs(line, file);
    }

    printf("\nPrime Implicant table:\n");
    fputs("\nPrime Implicant table:\n", file);
    for(i = 1; i < atoi(primeTable[0]) + 2; i++) {
        if(primeTable != NULL){
            strcpy(line, primeTable[i]);
            fputs(line, file);
            printf("%s", line);
        }
    }

    printf("\nThe boolean function was simplified to the following terms:\n");
    fputs("\nThe boolean function was simplified to the following terms:\n", file);
    strcpy(line, "F = ");
    int isOne = 0;
    for (i = 0; i < numImpl; i++) {
        isOne = 0;
        for (j = 0; j < numVar; j++) {
            if (primeImplicant[i][j] == '1') {
                literal[0] = 'a' + j;
                literal[1] = '\0';
                strcat(line, literal);
            } else if (primeImplicant[i][j] == '0') {
                literal[0] = 'a' + j;
                literal[1] = 39;
                literal[2] = '\0';
                strcat(line, literal);
            } else {
                isOne++;
                continue;
            }
        }
        if (i < numImpl - 1) {
            strcat(line, " + ");
        }
        if (isOne == numVar) {
            strcat(line, "1 ");
        }
    }
    if (numImpl == 0) {
        strcat(line, "0 ");
    }
    strcat(line, "\n");
    printf("%s", line);
    fputs(line, file);

    strcpy(line, ".end\n");
    fputs(line, file);
    fclose(file);
}


int main(int args, char** argv) {


    char* inputFile = malloc(sizeof (char)*20);
    if (args > 1)
        strcpy(inputFile, argv[1]);
    else {
        printf("Input file is required!\n");
        exit(0);
    }

    printf("Input file is %s\n", inputFile);
    char* outputFile = "out.dat";

    int i = 0, j = 0, k = 0;

    // onSet contains minterms
    int *onSet = (int*) malloc(sizeof (int)*64);
    int *minterm = (int*) malloc(sizeof (int)*64);
    int numOnSet = 0;
    // dontCare contains don't care terms
    int *dontCare = (int*) malloc(sizeof (int)*64);
    int numDontCare = 0;
    // all contains all of the above together
    int* all;
    int numAll = 0;

    /* group is the main data structure of this program. 
     * Fourth dimensions groups minterms by the number of literals.
     * Third dimensions groups the minterms by the number of ones
     * Second dimension contains the table of implicants with the same number of ones
     * First dimension contains the minterms and don't cares for the implicant.
     * */
    int**** group;
    int numVar = 0;

    // primeImplicant is what is being printed out to the output file
    char** primeImplicant = (char**) malloc(sizeof (char*)*1024);
    char** allPrImp = (char**) malloc(sizeof (char*)*1024);
    int** prImp;
    int numImpl = 0;
    int numPrimeImpl = 0;

    char** primeTable = NULL;

    numVar = readFile(inputFile, onSet, dontCare, &numOnSet, &numDontCare);
    numAll = numOnSet + numDontCare;
    printf("Function has %d variables and %d terms\n", numVar, numAll);

    all = (int*) malloc(sizeof (int)*numAll);
    prImp = malloc(sizeof (int*)*((int) (pow(2, numVar))));

    for (i = 0; i < numAll; i++) {
        if ((onSet[j] < dontCare[k] && j != numOnSet) || k == numDontCare || numDontCare == 0) {
            minterm[j] = onSet[j];
            all[i] = onSet[j];
            j++;
        } else {
            all[i] = dontCare[k];
            k++;
        }
    }

    group = (int****) malloc(sizeof (int***)*(numVar + 1));
    mallocGroup(group, numVar + 1);
    // Puts minterms and don't cares to the group[0]
    groupTerms(all, numAll, group, numVar);

    numPrimeImpl = findPrimeImpl(group, prImp, numVar + 1);
    primeImplToString(group, allPrImp, numVar + 1);

    numImpl = simplify(prImp, numPrimeImpl, minterm, numOnSet, primeImplicant, numVar, 0, &primeTable);

    saveFile(outputFile, primeImplicant, onSet, dontCare, numVar, numImpl, numOnSet, numDontCare, primeTable, allPrImp, numPrimeImpl);
    printf("Output was saved in %s\n", outputFile);


    // Frees all of the malloced variables
    free(onSet);
    free(minterm);
    free(dontCare);
    free(all);
    for (i = 0; i < numPrimeImpl; i++) {
        free(primeImplicant[i]);
    }
    free(primeImplicant);
    for (i = 0; i < numImpl; i++) {
        free(prImp[i]);
    }
    free(prImp);
    for (i = 0; i < numPrimeImpl; i++) {
        free(allPrImp[i]);
    }
    free(allPrImp);
    for (i = 0; i < numVar; i++) {
        for (j = 0; j < numVar + 1 - i; j++) {
            for (k = 0; k < numRows; k++) {
                if (group[i][j][k] != NULL) {
                    free(group[i][j][k]);
                }
            }
            free(group[i][j]);
        }
        free(group[i]);
    }
    free(group);
    free(inputFile);

    return 0;
}
