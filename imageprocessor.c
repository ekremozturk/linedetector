#include <stdio.h>
#include "mpi.h"

int main(int argc, char* argv[])
{
	int rank, size;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &size);

   
   int transNumber = 0;
   int transSize = 200/(size-1);
   int smoothedNumber = 0;

   if(rank == 0){
   	FILE *File= fopen("input.txt", "r");
		int input[200][200];
   
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

            int smoothedInput[transSize-1][198];

            for(int row = 1; row < transSize; row ++){
               for(int column = 1; column < 199; column++){
                  
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
				
   		} /**else if(rank % 2 == 0){
   			int transInput[transSize+2][200];

   			for(int row = 1; row <transSize+1; row++){
					for(int column = 0; column < 200; column++){
						MPI_Recv(&transNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						transInput[row][column] = transNumber;
						//printf("Process %d received input %d from process 0\n", proc, transInput[row][column]);
					}
				}


            for(int column = 0; column < 200; column++){

               int transNumber = transInput[transSize][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc+1, 0, &transNumber, 1, MPI_INT, proc+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               transInput[transSize+1][column] = transNumber;

               transNumber = transInput[1][column];
               MPI_Sendrecv(&transNumber, 1, MPI_INT, proc-1, 0, &transNumber, 1, MPI_INT, proc-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
               transInput[0][column] = transNumber;

            }

   		}*/ else {

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

   		}

			//TODO: smoothing
         if(rank == 1){


         } else if(rank == size-1){
            int smoothedInput[transSize-1][198];
            for(int row = 1; row < transSize; row ++){
               for(int column = 1; column < 199; column++){

               }
            }

         } else {
            int smoothedInput[transSize][198];
            for(int row = 1; row < transSize+1; row ++){
               for(int column = 1; column < 199; column++){

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
