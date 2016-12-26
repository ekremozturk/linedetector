#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[])
{
	int rank, size;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &size);

   int thresholdArgument[1];
   sscanf(argv[3], "%d", &thresholdArgument[0]) ;

   int transNumber = 0;
   int transSize = 200/(size-1);
   int threshold = thresholdArgument[0];

   if(rank == 0){
   	FILE *File= fopen(argv[1], "r");
		int input[200][200];
      int output[196][196];
		for(int row=0; row<200; row++){
   		for( int col=0; col<200; col++){
   			int s[1];
    			fscanf(File, "%d" ,&s[0]);
    		//printf("%d ", s[0]);
    			input[row][col]= s[0];
  			}
   	}

   	for(int proc = 1; proc < size; proc++){
			for(int row = 0; row <transSize; row++){
				for(int column = 0; column < 200; column++){
					int transNumber = input[row+(transSize*(proc-1))][column];
					MPI_Send(&transNumber, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
					//printf("Process %d received input %d from process 0\n", proc, transNumber);
				}
			}
		}

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
      for(int row=0; row<196; row++){
         for( int col=0; col<196; col++){
            int outputNumber = output[row][col];
            fprintf(outputFile, "%d ", outputNumber);
         }
         fprintf(outputFile, "\n");
      }


   }

   for(int proc = 1; proc < size; proc++){
   	if(rank == proc){
   		if(rank == 1){

   			int transInput[transSize+1][200];
   			
   			for(int row = 0; row <transSize; row++){
					for(int column = 0; column < 200; column++){
						MPI_Recv(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						transInput[row][column] = transNumber;
						//printf("Process %d received input %d from process 0\n", proc, transInput[row][column]);
					}
				}

				for(int column = 0; column < 200; column++){

               int transNumber = transInput[transSize-1][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc+1, 0, &transNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   				transInput[transSize][column] = transNumber;

   			}

            //smoothing

            int smoothedInput[transSize][198];
            int smoothedNumber = 0;

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

            for(int column = 0; column < 198; column++){

               int smoothedNumber = smoothedInput[transSize-2][column];
               MPI_Sendrecv(&smoothedNumber, 1, MPI_INT, proc+1, 0, &smoothedNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               smoothedInput[transSize-1][column] = smoothedNumber;

            }


            int thresholdedMatrix[transSize-2][196];
            int horizontalLine = 0;
            int verticalLine = 0;
            int obliquePlus45 = 0;
            int obliqueMinus45 = 0;
            int isLine = 0;


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

            for(int row = 0; row <transSize-2; row++){
               for(int column = 0; column<196; column++){
                  int transNumber = thresholdedMatrix[row][column];
                  MPI_Send(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
               }
            }

   		} else if(rank == size-1){
   			int transInput[transSize+1][200];

   			for(int row = 1; row <transSize+1; row++){
					for(int column = 0; column < 200; column++){
						MPI_Recv(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						transInput[row][column] = transNumber;
						//printf("Process %d received input %d from process 0\n", proc, transInput[row][column]);
					}
				}

				for(int column = 0; column < 200; column++){

               int transNumber = transInput[1][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc-1, 0, &transNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   				transInput[0][column] = transNumber;

   			}

            //smoothing

            int smoothedInput[transSize][198];
            int smoothedNumber = 0;

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

            for(int column = 0; column < 198; column++){

               int smoothedNumber = smoothedInput[1][column];
               MPI_Sendrecv(&smoothedNumber, 1, MPI_INT, proc-1, 0, &smoothedNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               smoothedInput[0][column] = smoothedNumber;

            }


            int thresholdedMatrix[transSize-2][196];
            int horizontalLine = 0;
            int verticalLine = 0;
            int obliquePlus45 = 0;
            int obliqueMinus45 = 0;
            int isLine = 0;


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
				
   		} else {

   			int transInput[transSize+2][200];

   			for(int row = 1; row <transSize+1; row++){
					for(int column = 0; column < 200; column++){
						MPI_Recv(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						transInput[row][column] = transNumber;
					}
				}

				for(int column = 0; column < 200; column++){

               transNumber = transInput[1][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc-1, 0, &transNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   				transInput[0][column] = transNumber;

               int transNumber = transInput[transSize][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc+1, 0, &transNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               transInput[transSize+1][column] = transNumber;
               
   			}

            //smoothing

            int smoothedInput[transSize+2][198];
            int smoothedNumber = 0;

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

            for(int column = 0; column < 198; column++){

               int smoothedNumber = smoothedInput[transSize][column];
               MPI_Sendrecv(&smoothedNumber, 1, MPI_INT, proc+1, 0, &smoothedNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               smoothedInput[transSize+1][column] = smoothedNumber;

               smoothedNumber = smoothedInput[1][column];
               MPI_Sendrecv(&smoothedNumber, 1, MPI_INT, proc-1, 0, &smoothedNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               smoothedInput[0][column] = smoothedNumber;

            }

            int thresholdedMatrix[transSize][196];
            int horizontalLine = 0;
            int verticalLine = 0;
            int obliquePlus45 = 0;
            int obliqueMinus45 = 0;
            int isLine = 0;


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

            //send to master

            for(int row = 0; row <transSize; row++){
               for(int column = 0; column<196; column++){
                  int transNumber = thresholdedMatrix[row][column];
                  MPI_Send(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
               }
            }

   		}

   	}
   }

   //printf("Hello, world, I am %d of %d\n", rank, size);
   MPI_Barrier(MPI_COMM_WORLD);
   MPI_Finalize();

   return 0;
}
