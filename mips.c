#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#define MAX_FILE_NAME 100 
#define NULL ((char *)0)

typedef struct Operation {
	int insAdress;
	char opCode[6];
	char operand1[4];
	char operand2[4];
	char operand3[4];
} Operation;
typedef struct Stage {
	char name[6];
	Operation currentlyHostedOperation;
} Stage;
void main(int argc, char* argv[])
{
	char trace[11] = "trace1.txt"; // required trace file name
	int numberOfLines = countlines(trace); // count file number of lines
	printf("\n");
	Operation* arrayOfOperations;	// create an array of operations 
	arrayOfOperations = (Operation*)malloc(sizeof(Operation) * numberOfLines);
	Operation stall = { 0,"stall",NULL,NULL,NULL }; // initialize STALL
	readFromFile(arrayOfOperations, numberOfLines, trace); // read from trace1/2.txt and append to arrayOfOperations
	// create an array of Stages
	Stage Fetch = { "FETCH",NULL }; Stage Decode = { "DECOD", NULL }; Stage Execute = { "EXECU", NULL }; Stage Mem = { "MEM", NULL }; Stage WB = { "WBack" ,NULL };
	Stage Stages[5] = { Fetch,  Decode,  Execute,  Mem, WB }; // all initialized with NULL
	float clockcycles;
	if (!strcmp(argv[1], "0") && !strcmp(argv[2], "0")) clockcycles = mode_00(numberOfLines, Stages, arrayOfOperations);
	if (!strcmp(argv[1], "0") && !strcmp(argv[2], "1")) clockcycles = mode_01(numberOfLines, Stages, arrayOfOperations);
	if (!strcmp(argv[1], "1") && !strcmp(argv[2], "0")) clockcycles = mode_10(numberOfLines, Stages, arrayOfOperations);
	if (!strcmp(argv[1], "1") && !strcmp(argv[2], "1")) clockcycles = mode_11(numberOfLines, Stages, arrayOfOperations);
	float CPI = (clockcycles / numberOfLines);
	printf("for file:%s and %s %s mode , the CPI IS %f\n", trace, argv[1], argv[2], CPI);
	system("pause");
}
int mode_00(int numberOfLines, Stage* Stages, Operation* arrayOfOperations) {
	printf("You chose 0 0 mode - stalling only\n\n\n");
	Operation stall = { 0,"stall",NULL,NULL,NULL };
	int clockcycles = 1;
	for (int i = 0; i < numberOfLines; i++)
	{
		waitFor(1); // time sleep between clocks
		promote_stages(Stages, i, arrayOfOperations); // instead of 6 lines 
		printf("Cycle number %d\n", clockcycles);
		clockcycles++;
		// stalling 3 stages if branch taken
		if (
			(
				!strcmp(Stages[3].currentlyHostedOperation.opCode, "beq") ||
				!strcmp(Stages[3].currentlyHostedOperation.opCode, "bneq")
				) && (
					Stages[3].currentlyHostedOperation.insAdress > Stages[2].currentlyHostedOperation.insAdress
					))
		{
			printAllStages(Stages); printf("\n");
			promote_stages(Stages, i, arrayOfOperations);
			Stages[0].currentlyHostedOperation = Stages[3].currentlyHostedOperation;
			Stages[1].currentlyHostedOperation = stall;
			Stages[2].currentlyHostedOperation = stall;
			Stages[3].currentlyHostedOperation = stall;
			printAllStages(Stages); printf("\n");
			i = i - 2;
			printf("\n  Branch Taken #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		// R-type
		if ( // compare_to_next_first_stage
			(
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "add") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "sub") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "or") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "and") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "addi") ||//
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "subi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "ori") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "andi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "lw")
				)
			&&
			(
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[0].currentlyHostedOperation.operand2) ||
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[0].currentlyHostedOperation.operand3) ||//
				(!strcmp(Stages[1].currentlyHostedOperation.opCode, "lw") ||
					!strcmp(Stages[1].currentlyHostedOperation.opCode, "beq") ||
					!strcmp(Stages[1].currentlyHostedOperation.opCode, "bneq")
					) &&  // since lw beq bneq reads from its first operand; ex: 1004 lw $2 200 $10 
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand1)
				)
			||
			!strcmp(Stages[0].currentlyHostedOperation.opCode, "sw") &&
			!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[0].currentlyHostedOperation.operand3)
			)
		{
			printAllStages(Stages); printf("\n");
			promote_stages(Stages, i, arrayOfOperations);
			Stages[1].currentlyHostedOperation = stall;
			Stages[0].currentlyHostedOperation = stall;
			printAllStages(Stages); printf("\n");
			i--;
			printf("\n 2 STALLS ADDED IN CYCLE  #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		if (
			// compare_to_next_second_stage
			(
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "add") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "sub") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "or") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "and") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "addi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "subi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "ori") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "andi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "lw")
				)
			&&
			(
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand2) ||
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand3) ||
				(!strcmp(Stages[1].currentlyHostedOperation.opCode, "lw") ||
					!strcmp(Stages[1].currentlyHostedOperation.opCode, "beq") ||
					!strcmp(Stages[1].currentlyHostedOperation.opCode, "bneq")
					) &&  // since lw beq / bneq reads from its first operand; ex: 1004 lw $2 200 $10 
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand1)
				) ||
			!strcmp(Stages[1].currentlyHostedOperation.opCode, "sw") &&
			!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand3)
			) {
			promote_stages(Stages, i, arrayOfOperations);
			Stages[1].currentlyHostedOperation = stall;
			Stages[2].currentlyHostedOperation = stall;
			Stages[0].currentlyHostedOperation = stall;
			printAllStages(Stages); printf("\n");
			i = i - 2;
			printf("\STALL ADDED IN CYCLE  #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		printAllStages(Stages); printf("\n");
	}
	return clockcycles;
}
int mode_01(int numberOfLines, Stage* Stages, Operation* arrayOfOperations) {
	printf("You chose 0 1 mode - Forwarding Only\n\n\n");
	Operation stall = { 0,"stall",NULL,NULL,NULL };
	int clockcycles = 1;
	for (int i = 0; i < numberOfLines; i++)
	{
		waitFor(1); // time sleep between clocks
		printf("Cycle number %d\n", clockcycles);
		clockcycles++;
		// stalling 3 stages if branch taken
		if (
			(
				!strcmp(Stages[3].currentlyHostedOperation.opCode, "beq") ||
				!strcmp(Stages[3].currentlyHostedOperation.opCode, "bneq")
				) && (
					Stages[3].currentlyHostedOperation.insAdress > Stages[2].currentlyHostedOperation.insAdress
					))
		{
			promote_stages(Stages, i, arrayOfOperations);
			Stages[0].currentlyHostedOperation = Stages[3].currentlyHostedOperation;
			Stages[1].currentlyHostedOperation = stall;
			Stages[2].currentlyHostedOperation = stall;
			Stages[3].currentlyHostedOperation = stall;
			printAllStages(Stages); printf("\n");
			i = i - 3;
			printf("\n  Branch Taken #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}

		if ( // forwarding - only lw problematic 
			(
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "lw")
				) && (
					!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand2) ||
					!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand3) ||//
					(!strcmp(Stages[1].currentlyHostedOperation.opCode, "lw") ||
						!strcmp(Stages[1].currentlyHostedOperation.opCode, "beq") ||
						!strcmp(Stages[1].currentlyHostedOperation.opCode, "bneq")
						) && (// since lw beq bneq reads from its first operand; ex: 1004 lw $2 200 $10 
							!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand1))
					))
		{
			printAllStages(Stages); printf("\n");
			Stages[4].currentlyHostedOperation = Stages[3].currentlyHostedOperation;
			Stages[3].currentlyHostedOperation = Stages[2].currentlyHostedOperation;
			Stages[2].currentlyHostedOperation = stall;
			printAllStages(Stages); printf("\n");
			printf("\n  FORWARDING ACTIVATED HERE #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		promote_stages(Stages, i, arrayOfOperations); // instead of 6 lines 
		printAllStages(Stages); printf("\n");
	}
	return clockcycles;
}
int mode_10(int numberOfLines, Stage* Stages, Operation* arrayOfOperations) {
	printf("You chose 1 0 mode - branch resolution only \n\n\n");
	Operation stall = { 0,"stall",NULL,NULL,NULL };
	int clockcycles = 1;
	for (int i = 0; i < numberOfLines; i++)
	{
		waitFor(1); // time sleep between clocks
		promote_stages(Stages, i, arrayOfOperations); // instead of 6 lines 
		printf("Cycle number %d\n", clockcycles);
		clockcycles++;
		// stalling 3 stages if branch taken
		if (
			(
				!strcmp(Stages[1].currentlyHostedOperation.opCode, "beq") ||
				!strcmp(Stages[1].currentlyHostedOperation.opCode, "bneq")
				) && (
					Stages[1].currentlyHostedOperation.insAdress > Stages[0].currentlyHostedOperation.insAdress
					))
		{
			printAllStages(Stages); printf("\n");
			promote_stages(Stages, i, arrayOfOperations);
			Stages[0].currentlyHostedOperation = Stages[1].currentlyHostedOperation;
			Stages[1].currentlyHostedOperation = stall;
			//i--;
			printAllStages(Stages); printf("\n");
			printf("\n  Branch Taken #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		// R-type
		if ( // compare_to_next_first_stage
			(
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "add") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "sub") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "or") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "and") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "addi") ||//
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "subi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "ori") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "andi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "lw")
				)
			&&
			(
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[0].currentlyHostedOperation.operand2) ||
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[0].currentlyHostedOperation.operand3) ||//
				(!strcmp(Stages[1].currentlyHostedOperation.opCode, "lw") ||
					!strcmp(Stages[1].currentlyHostedOperation.opCode, "beq") ||
					!strcmp(Stages[1].currentlyHostedOperation.opCode, "bneq")
					) &&  // since lw beq bneq reads from its first operand; ex: 1004 lw $2 200 $10 
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand1)
				)
			||
			!strcmp(Stages[0].currentlyHostedOperation.opCode, "sw") &&
			!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[0].currentlyHostedOperation.operand3)
			)
		{
			printAllStages(Stages); printf("\n");
			promote_stages(Stages, i, arrayOfOperations);
			Stages[1].currentlyHostedOperation = stall;
			Stages[0].currentlyHostedOperation = stall;
			printAllStages(Stages); printf("\n");
			i--;
			printf("\n 2 STALLS ADDED IN CYCLE  #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		if (
			// compare_to_next_second_stage

			(
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "add") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "sub") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "or") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "and") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "addi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "subi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "ori") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "andi") ||
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "lw")
				)
			&&
			(
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand2) ||
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand3) ||
				(!strcmp(Stages[1].currentlyHostedOperation.opCode, "lw") ||
					!strcmp(Stages[1].currentlyHostedOperation.opCode, "beq") ||
					!strcmp(Stages[1].currentlyHostedOperation.opCode, "bneq")
					) &&  // since lw beq / bneq reads from its first operand; ex: 1004 lw $2 200 $10 
				!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand1)
				) ||
			!strcmp(Stages[1].currentlyHostedOperation.opCode, "sw") &&
			!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand3)
			) {
			promote_stages(Stages, i, arrayOfOperations);
			Stages[1].currentlyHostedOperation = stall;
			Stages[2].currentlyHostedOperation = stall;
			Stages[0].currentlyHostedOperation = stall;
			printAllStages(Stages); printf("\n");
			i = i - 2;
			printf("\STALL ADDED IN CYCLE  #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		printAllStages(Stages); printf("\n");
	}
	return clockcycles;
}
int mode_11(int numberOfLines, Stage* Stages, Operation* arrayOfOperations) {
	printf("You chose 1 1 mode - Forwarding +Branch Resolution\n\n\n");
	Operation stall = { 0,"stall",NULL,NULL,NULL };
	int clockcycles = 1;
	for (int i = 0; i < numberOfLines; i++)
	{
		waitFor(1); // time sleep between clocks
		promote_stages(Stages, i, arrayOfOperations); // instead of 6 lines 
		printf("Cycle number %d\n", clockcycles);
		clockcycles++;
		// stalling 3 stages if branch taken
		if (
			(
				!strcmp(Stages[1].currentlyHostedOperation.opCode, "beq") ||
				!strcmp(Stages[1].currentlyHostedOperation.opCode, "bneq")
				) && (
					Stages[1].currentlyHostedOperation.insAdress > Stages[0].currentlyHostedOperation.insAdress
					))
		{
			printAllStages(Stages); printf("\n");
			promote_stages(Stages, i, arrayOfOperations);
			Stages[0].currentlyHostedOperation = Stages[1].currentlyHostedOperation;
			Stages[1].currentlyHostedOperation = stall;
			//i--;
			printAllStages(Stages); printf("\n");
			printf("\n  Branch Taken #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		// R-type
		if ( // forwarding - only lw problematic 
			(
				!strcmp(Stages[2].currentlyHostedOperation.opCode, "lw")
				) && (
					!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand2) ||
					!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand3) ||//
					(!strcmp(Stages[1].currentlyHostedOperation.opCode, "lw") ||
						!strcmp(Stages[1].currentlyHostedOperation.opCode, "beq") ||
						!strcmp(Stages[1].currentlyHostedOperation.opCode, "bneq")
						) && (// since lw beq bneq reads from its first operand; ex: 1004 lw $2 200 $10 
							!strcmp(Stages[2].currentlyHostedOperation.operand1, Stages[1].currentlyHostedOperation.operand1))
					))
		{
			//promote_stages(Stages, i, arrayOfOperations);
			printAllStages(Stages); printf("\n");
			Stages[4].currentlyHostedOperation = Stages[3].currentlyHostedOperation;
			Stages[3].currentlyHostedOperation = Stages[2].currentlyHostedOperation;
			Stages[2].currentlyHostedOperation = stall;
			printAllStages(Stages); printf("\n");
			printf("\n  FORWARDING ACTIVATED HERE #%d\n\n", clockcycles);
			continue; // inorder to restart the loop
		}
		printAllStages(Stages); printf("\n");
	}
	return clockcycles;
}
int printAllStages(Stage arr[]) {
	for (int i = 0; i < 5; i++)
	{
		printf("%s instruction: %d %s %s %s %s\n", arr[i].name, arr[i].currentlyHostedOperation.insAdress, arr[i].currentlyHostedOperation.opCode, arr[i].currentlyHostedOperation.operand1, arr[i].currentlyHostedOperation.operand2, arr[i].currentlyHostedOperation.operand3);
	}
}
int promote_stages(Stage Stages[], int i, Operation* arrayOfOperations) {
	Stages[4].currentlyHostedOperation = Stages[3].currentlyHostedOperation;
	Stages[3].currentlyHostedOperation = Stages[2].currentlyHostedOperation;
	Stages[2].currentlyHostedOperation = Stages[1].currentlyHostedOperation;
	Stages[1].currentlyHostedOperation = Stages[0].currentlyHostedOperation;
	Stages[0].currentlyHostedOperation = arrayOfOperations[i];
}
int countlines(char filename[MAX_FILE_NAME])
{
	FILE* fp;
	int count = 0;  // Line counter (result) 
	char c;  // To store a character read from file 
	fp = fopen(filename, "r"); // Open the file 

	// Check if file exists 
	if (fp == NULL)
	{
		printf("Could not open file %s", filename);
		return 0;
	}

	// Extract characters from file and store in character c 
	for (c = getc(fp); c != EOF; c = getc(fp))
		if (c == '\n') // Increment count if this character is newline 
			count = count + 1;

	// Close the file 
	fclose(fp);
	//printf("The file %s has %d lines\n ", filename, count);

	return count;
}
int readFromFile(Operation* OP, int numoflines, const char* trace)
{
	/* demo.txt which contains following data
	   1000 lw $1 100 $10
	   1004 lw $2 200 $10
	   1008 add $3 $2 $1
	*/

	FILE* pointer = fopen(trace, "r");
	if (pointer == NULL) {
		printf("No file found.");
		return 0;
	}
	int i = 0;
	while (i < numoflines) {
		fscanf(pointer, "%d %s %s %s %s", &OP[i].insAdress, OP[i].opCode, OP[i].operand1, OP[i].operand2, OP[i].operand3);
		i++;
	}
	return 0;
}
int waitFor(unsigned int secs) {
	unsigned int retTime = time(0) + secs;   // Get finishing time.
	while (time(0) < retTime);               // Loop until it arrives.
}