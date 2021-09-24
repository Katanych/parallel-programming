#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <locale.h>


static double f(double a);
static double fi(double a);

void main(int argc, char *argv[])
{
    setlocale(LC_ALL, "Rus");
    int done = 0, n, myid, numprocs, i;
    double myfunk, funk, h, sum, x;
    double xl = -0.5,	// левая граница
	   xh =  1;	// правая граница
    double startwtime, endwtime;
    int  namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Status stats;
    	
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);
	
    fprintf(stderr,"Proc: %d, name %s\n",
		myid, processor_name);
    fflush(stderr);
	
    n = 0;
    while (!done)
    {
        if (myid == 0)
        {
	    printf("Number procs (0 to quit): ");
	    fflush(stdout);
	    scanf("%d",&n);
	    
	    startwtime = MPI_Wtime();
        
            /* Отправка количества интервалов на другой узел */

            for (i=1; i < numprocs; i++)
            {
	        MPI_Send (&n,                 /* buffer               */
        	          1,                  /* one data             */
	                  MPI_INT,            /* type                 */
	       		  i,                  /* to which node        */
	       		  1,                  /* number of message    */
	        	  MPI_COMM_WORLD);    /* common communicator  */
	    }
	}
	else
	{    
    	    MPI_Recv  (&n,                /* buffer               */
               	       1,                 /* one data             */
	       	       MPI_INT,           /* type                 */
	      	       0,                 /* to which node        */
	               1,                 /* number of message    */
	               MPI_COMM_WORLD,    /* common communicator  */
	               &stats);
	}
       
        if (n == 0)
            done = 1;
        else
        {
	    /* Считаем локальные (частичные) суммы */
            h   = (xh-xl) / (double) n;
            sum = 0.0;
            for (i = myid + 1; i <= n; i += numprocs)
            {
                x = xl + h * ((double)i - 0.5);
                sum += f(x);
            }
            myfunk = h * sum;
    	    printf("Proc %d, local sum: %.16f\n", myid, myfunk);
    
    	    /* Отправляем частичные суммы в узел 0 */
	    if (myid !=0) 
	    {
    	        MPI_Send (&myfunk,            /* buffer               */
               		  1,                  /* one data             */
	       		  MPI_DOUBLE,         /* type                 */
	       		  0,                  /* to which node        */
	       		  1,                  /* number of message    */
	       		  MPI_COMM_WORLD);    /* common communicator  */
	    }
			
            if (myid == 0)
	    {
		/* Считаем в процессе 1 все частичные суммы */
	        funk = myfunk;
      		for (i = 1; i < numprocs; i++)
         	{ 
		    MPI_Recv (&myfunk,
	             	      1,
		    	      MPI_DOUBLE,
		  	      i,
		 	      1,
			      MPI_COMM_WORLD,
		    	      &stats);
	   	    funk += myfunk;  
	 	} // ;?

                printf("Integral:  %.16f,  Eps:  %.16f\n",
		    funk, funk - fi(xh) + fi(xl));
		endwtime = MPI_Wtime();
		printf("Time: %f\n", endwtime-startwtime);
		
		/* Выведем информацию в файл */
		FILE *myfile;
		myfile = fopen("data1.txt", "a");
		fprintf(myfile, "\n     %d      |     %d     |%.10f|%.10f|%.10f", 
			numprocs, n, funk, funk - fi(xh) + fi(xl), endwtime - startwtime);
		fclose(myfile);	       
	    }
        }
    }
    MPI_Finalize();
}


static double f(double x)
{
    double c = 0.8;
    return x * sinh(x * c);
}

static double fi(double x)
{
    double c = 0.8;
    return (x * cosh(c * x) / c) - (sinh(c * x) / (c * c));
}

