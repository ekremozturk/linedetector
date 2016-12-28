/* Student Name: Ekrem Ozturk
 * Student Number: 2012400006
 * Compile Status: Compiling
 * Program Status: Working
 */

#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[])
{

	int rank, size;


   MPI_Init(&argc, &argv);  //initialize MPI
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);  //Rank of each processor initialized from 0 to n-1
   MPI_Comm_size(MPI_COMM_WORLD, &size);  //Total number of processors initialized

   int thresholdArgument[1];  //

   sscanf(argv[3], "%d", &thresholdArgument[0]) ;

   int transNumber = 0;  //Variable used to pass values in mpi_send and mpi_receive functions

   int transSize = 200/(size-1);  //Transferred array size: 200 / (# of servant processors)

   int threshold = thresholdArgument[0];  //Threshold variable

   if(rank == 0){ //Master processor

   	FILE *File= fopen(argv[1], "r");
		int input[200][200];  //input 2D array
      int output[196][196];  //output 2D array

      //reads the input file and inserts into array
		for(int row=0; row<200; row++){
   		for( int col=0; col<200; col++){
   			int s[1];
    			fscanf(File, "%d" ,&s[0]);
    			input[row][col]= s[0];
  			}
   	}

      //Divides the input into equal parts and sends them to servants
   	for(int proc = 1; proc < size; proc++){
			for(int row = 0; row <transSize; row++){
				for(int column = 0; column < 200; column++){
					int transNumber = input[row+(transSize*(proc-1))][column];
					MPI_Send(&transNumber, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
					//printf("Process %d received input %d from process 0\n", proc, transNumber);
				}
			}
		}

      //Receives output and inserts into output array
      for(int proc = 1; proc<size; proc++){
         if(proc == 1){
            for(int row = 0; row <transSize-2; row++){
               for(int column = 0; column < 196; column++){
                  MPI_Recv(&transNumber, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                  output[row][column] = transNumber;
                  //printf("Process 0 received input %d from process %d\n", transNumber, proc);
               }
            }
         } else if(proc == size-1){
            for(int row = 0; row <transSize-2; row++){
               for(int column = 0; column < 196; column++){
                  MPI_Recv(&transNumber, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                  output[row+(transSize*(proc-1)-2)][column] = transNumber;
               }
            }
         } else {
            for(int row = 0; row <transSize; row++){
               for(int column = 0; column < 196; column++){
                  MPI_Recv(&transNumber, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                  output[row+(transSize-2)+(transSize*(proc-2))][column] = transNumber;
               }
            }
         }
      }


      FILE *outputFile = fopen(argv[2], "w");

      //Writes output into output file
      for(int row=0; row<196; row++){
         for( int col=0; col<196; col++){
            int outputNumber = output[row][col];
            fprintf(outputFile, "%d ", outputNumber);
         }
         fprintf(outputFile, "\n");
      }


   }

   for(int proc = 1; proc < size; proc++){  //Servant processors
   	if(rank == proc){

   		if(rank == 1){ //processor 2

   			int transInput[transSize+1][200];  //input array for this processor
   			
            //receive input from master
   			for(int row = 0; row <transSize; row++){
					for(int column = 0; column < 200; column++){
						MPI_Recv(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						transInput[row][column] = transNumber;
						//printf("Process %d received input %d from process 0\n", proc, transInput[row][column]);
					}
				}

            //Send and receive border line
				for(int column = 0; column < 200; column++){

               int transNumber = transInput[transSize-1][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc+1, 0, &transNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   				transInput[transSize][column] = transNumber;

   			}

            //SMOOTHING

            int smoothedInput[transSize][198];
            int smoothedNumber = 0;  //Variable for produced number after taking mean

            //Takes mean of 3x3 matrix and inserts into smoothedInput array
            for(int row = 1; row < transSize; row ++){
               for(int column = 1; column < 199; column++){
                  for(int i = -1; i < 2; i++){
                     for(int j = -1; j < 2; j++){
                        smoothedNumber = smoothedNumber + transInput[row+i][column+j];
                     }
                  }
                  smoothedNumber = smoothedNumber/9;
                  smoothedInput[row-1][column-1] = smoothedNumber;
                  smoothedNumber = 0;
               }
            }

            //Send and receive border line
            for(int column = 0; column < 198; column++){

               int smoothedNumber = smoothedInput[transSize-2][column];
               //printf("1. sayı : %d\n", smoothedNumber);
               MPI_Sendrecv(&smoothedNumber, 1, MPI_INT, proc+1, 0, &smoothedNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               //printf("2. sayı : %d\n", smoothedNumber);
               smoothedInput[transSize-1][column] = smoothedNumber;

            }


            int thresholdedMatrix[transSize-2][196];

            //Variables for produced numbers after multiplications
            int horizontalLine = 0;
            int verticalLine = 0;
            int obliquePlus45 = 0;
            int obliqueMinus45 = 0;

            int isLine = 0;  // if it is greater than zero, one of above values has passed the threshold


            //Multiplies 3x3 matrix and inserts 0 or 255 into thresholdedMatrix array
            for(int row = 1; row < transSize-1; row++){
               for(int column = 1; column<197; column++){
                  horizontalLine = 
                  -1*smoothedInput[row-1][column-1]
                  +2*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  +2*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(horizontalLine>threshold) isLine++;

                  verticalLine = 
                  -1*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  +2*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  +2*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(verticalLine>threshold) isLine++;

                  obliquePlus45 = 
                  -1*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  +2*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  +2*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(obliquePlus45>threshold) isLine++;

                  obliqueMinus45 = 
                  +2*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  +2*smoothedInput[row+1][column+1];

                  if(obliqueMinus45>threshold) isLine++;

                  if(isLine>0){
                     thresholdedMatrix[row-1][column-1] = 255;
                  } else {
                     thresholdedMatrix[row-1][column-1] = 0;
                  }

                  horizontalLine = 0;
                  verticalLine = 0;
                  obliquePlus45 = 0;
                  obliqueMinus45 = 0;
                  isLine = 0;

               }
            }

            //Sends processed input back to master
            for(int row = 0; row <transSize-2; row++){
               for(int column = 0; column<196; column++){
                  int transNumber = thresholdedMatrix[row][column];
                  MPI_Send(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
               }
            }

   		} else if(rank == size-1){  //processor n
   			int transInput[transSize+1][200];//input array for this processor
            
            //receive input from master
   			for(int row = 1; row <transSize+1; row++){
					for(int column = 0; column < 200; column++){
						MPI_Recv(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						transInput[row][column] = transNumber;
						//printf("Process %d received input %d from process 0\n", proc, transInput[row][column]);
					}
				}

            //Send and receive border line
				for(int column = 0; column < 200; column++){

               int transNumber = transInput[1][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc-1, 0, &transNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   				transInput[0][column] = transNumber;

   			}

            //SMOOTHING

            int smoothedInput[transSize][198];
            int smoothedNumber = 0; //Variable for produced number after taking mean

            //Takes mean of 3x3 matrix and inserts into smoothedInput array
            for(int row = 1; row < transSize; row ++){
               for(int column = 1; column < 199; column++){
                  for(int i = -1; i < 2; i++){
                     for(int j = -1; j < 2; j++){
                        smoothedNumber = smoothedNumber + transInput[row+i][column+j];
                     }
                  }
                  smoothedNumber = smoothedNumber/9;
                  smoothedInput[row][column-1] = smoothedNumber;
                  smoothedNumber = 0;

               }
            }

            //Send and receive border line
            for(int column = 0; column < 198; column++){

               int smoothedNumber = smoothedInput[1][column];
               MPI_Sendrecv(&smoothedNumber, 1, MPI_INT, proc-1, 0, &smoothedNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               smoothedInput[0][column] = smoothedNumber;

            }




            int thresholdedMatrix[transSize-2][196];

            //Variables for produced numbers after multiplications
            int horizontalLine = 0;
            int verticalLine = 0;
            int obliquePlus45 = 0;
            int obliqueMinus45 = 0;

            int isLine = 0; // if it is greater than zero, one of above values has passed the threshold

            //Multiplies 3x3 matrix and inserts 0 or 255 into thresholdedMatrix array
            for(int row = 1; row < transSize-1; row++){
               for(int column = 1; column<197; column++){
                  horizontalLine = 
                  -1*smoothedInput[row-1][column-1]
                  +2*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  +2*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(horizontalLine>threshold) isLine++;

                  verticalLine = 
                  -1*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  +2*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  +2*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(verticalLine>threshold) isLine++;

                  obliquePlus45 = 
                  -1*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  +2*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  +2*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(obliquePlus45>threshold) isLine++;

                  obliqueMinus45 = 
                  +2*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  +2*smoothedInput[row+1][column+1];

                  if(obliqueMinus45>threshold) isLine++;

                  if(isLine>0){
                     thresholdedMatrix[row-1][column-1] = 255;
                  } else {
                     thresholdedMatrix[row-1][column-1] = 0;
                  }

                  horizontalLine = 0;
                  verticalLine = 0;
                  obliquePlus45 = 0;
                  obliqueMinus45 = 0;
                  isLine = 0;

               }
            }

            //sending thresholded values to master

            for(int row = 0; row <transSize-2; row++){
               for(int column = 0; column<196; column++){
                  int transNumber = thresholdedMatrix[row][column];
                  MPI_Send(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
               }
            }
				
   		} else { //between processor 2 and n-1

   			int transInput[transSize+2][200];//input array for this processor
            
            //receive input from master
   			for(int row = 1; row <transSize+1; row++){
					for(int column = 0; column < 200; column++){
						MPI_Recv(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						transInput[row][column] = transNumber;
					}
				}

            //Send and receive border line
				for(int column = 0; column < 200; column++){

               transNumber = transInput[1][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc-1, 0, &transNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   				transInput[0][column] = transNumber;

               int transNumber = transInput[transSize][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc+1, 0, &transNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               transInput[transSize+1][column] = transNumber;
               
   			}


            //SMOOTHING

            int smoothedInput[transSize+2][198];
            int smoothedNumber = 0; //Variable for produced number after taking mean

            //Takes mean of 3x3 matrix and inserts into smoothedInput array
            for(int row = 1; row < transSize+1; row ++){
               for(int column = 1; column < 199; column++){
                  for(int i = -1; i < 2; i++){
                     for(int j = -1; j < 2; j++){
                        smoothedNumber = smoothedNumber + transInput[row+i][column+j];
                     }
                  }
                  smoothedNumber = smoothedNumber/9;
                  smoothedInput[row][column-1] = smoothedNumber;
                  smoothedNumber = 0;
               }
            }

            //Send and receive border line
            for(int column = 0; column < 198; column++){

               int smoothedNumber = smoothedInput[transSize][column];
               MPI_Sendrecv(&smoothedNumber, 1, MPI_INT, proc+1, 0, &smoothedNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               smoothedInput[transSize+1][column] = smoothedNumber;

               smoothedNumber = smoothedInput[1][column];
               MPI_Sendrecv(&smoothedNumber, 1, MPI_INT, proc-1, 0, &smoothedNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               smoothedInput[0][column] = smoothedNumber;

            }
            

            int thresholdedMatrix[transSize][196];

            //Variables for produced numbers after multiplications
            int horizontalLine = 0;
            int verticalLine = 0;
            int obliquePlus45 = 0;
            int obliqueMinus45 = 0;

            int isLine = 0; // if it is greater than zero, one of above values has passed the threshold

            //Multiplies 3x3 matrix and inserts 0 or 255 into thresholdedMatrix array
            for(int row = 1; row < transSize+1; row++){
               for(int column = 1; column<197; column++){
                  horizontalLine = 
                  -1*smoothedInput[row-1][column-1]
                  +2*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  +2*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(horizontalLine>threshold) isLine++;

                  verticalLine = 
                  -1*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  +2*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  +2*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(verticalLine>threshold) isLine++;

                  obliquePlus45 = 
                  -1*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  +2*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  +2*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  -1*smoothedInput[row+1][column+1];

                  if(obliquePlus45>threshold) isLine++;

                  obliqueMinus45 = 
                  +2*smoothedInput[row-1][column-1]
                  -1*smoothedInput[row-1][column]
                  -1*smoothedInput[row-1][column+1]
                  -1*smoothedInput[row][column-1]
                  +2*smoothedInput[row][column]
                  -1*smoothedInput[row][column+1]
                  -1*smoothedInput[row+1][column-1]
                  -1*smoothedInput[row+1][column]
                  +2*smoothedInput[row+1][column+1];

                  if(obliqueMinus45>threshold) isLine++;

                  if(isLine>0){
                     thresholdedMatrix[row-1][column-1] = 255;
                  } else {
                     thresholdedMatrix[row-1][column-1] = 0;
                  }

                  horizontalLine = 0;
                  verticalLine = 0;
                  obliquePlus45 = 0;
                  obliqueMinus45 = 0;
                  isLine = 0;

               }
            }

            //send processed input to master

            for(int row = 0; row <transSize; row++){
               for(int column = 0; column<196; column++){
                  int transNumber = thresholdedMatrix[row][column];
                  MPI_Send(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
               }
            }

   		}

   	}
   }

   MPI_Barrier(MPI_COMM_WORLD);
   MPI_Finalize(); //MPI endsr

   return 0;
}
