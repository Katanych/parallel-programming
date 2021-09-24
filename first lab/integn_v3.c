#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <locale.h>

double c;

static double f(double a);
static double fi(double a);

void main(int argc, char *argv[])
{
    setlocale(LC_ALL, "Rus");
    char buf[100];
    int done = 0, n, myid, numprocs, i, pos;
    double myfunk, funk, h, sum, x;
    double xl, xh;	// границы
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
	
	    if (n != 0)
	    {
  	        printf("Enter left board: ");
	    	fflush(stdout);
	    	scanf("%lf", &xl); 
	    	printf("Enter right board: ");
	    	fflush(stdout);
	    	scanf("%lf", &xh);
	    	printf("Enter parameter func: ");
	    	fflush(stdout);
	    	scanf("%lf", &c); 
	    }

	    startwtime = MPI_Wtime();

	    pos = 0;
	    /* Упаковываем левую границу, правую и параметр */
	    MPI_Pack(&xl, 1, MPI_DOUBLE, &buf, 100, &pos, MPI_COMM_WORLD);
	    MPI_Pack(&xh, 1, MPI_DOUBLE, &buf, 100, &pos, MPI_COMM_WORLD);
	    MPI_Pack(&c, 1, MPI_DOUBLE, &buf, 100, &pos, MPI_COMM_WORLD);
	}

        /* Рассылаем всем эту инфу */
        MPI_Bcast(&buf, 100, MPI_PACKED, 0, MPI_COMM_WORLD);

        /* Распаковываем */
        pos = 0;
        MPI_Unpack(&buf, 100, &pos, &xl, 1, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Unpack(&buf, 100, &pos, &xh, 1, MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Unpack(&buf, 100, &pos, &c, 1, MPI_DOUBLE, MPI_COMM_WORLD);

	/* Отправка количества интервалов на другой узел */
	MPI_Bcast (&n,                 /* buffer               */
                   1,                  /* one data             */
	           MPI_INT,            /* type                 */
	           0,                  /* to which node        */
	           MPI_COMM_WORLD);    /* common communicator  */
	       
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
    	    printf("Proc %d, local sum: %.16f", myid, myfunk);
	    printf("\n");
    
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
			
	    MPI_Reduce (&myfunk,
			&funk,
			1,
			MPI_DOUBLE,
			MPI_SUM,
			0,
			MPI_COMM_WORLD);
            if (myid == 0)
	    {
		/* Считаем в процессе 1 все частичные суммы */

                printf("Integral:  %.16f\nEps:  %.16f\n",
		    funk, funk - fi(xh) + fi(xl));
		endwtime = MPI_Wtime();
		printf("Time: %f\n", endwtime-startwtime);
		
		/* Выведем информацию в файл */
		FILE *myfile;
		myfile = fopen("data3.txt", "a");
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
    //double c = 0.8;
    return x * sinh(x * c);
}

static double fi(double x)
{
    //double c = 0.8;
    return (x * cosh(c * x) / c) - (sinh(c * x) / (c * c));
}

