// mpiTestAir1.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include<mpi.h>
#include<stdio.h>
#include<math.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;


#define MASTER_PROCESS 0
#define UP_DATA_TRANSFER 99
#define DOWN_DATA_TRANSFER 98

int initTempField(float* t, int nx, int ny)
{
	float Tinner = 1000.0;
	float Tb1 = 100.0;
	float Tb2 = 500.0;
	float Tb3 = 200;
	float Tb4 = 0.0;

	for (int x = 0; x < nx;++x)
	for (int y = 0; y < ny; ++y)
	{
		const int index = x*ny + y;
		t[index] = Tinner;
		if (x == 0)
			t[index] = Tb1;
		if (y == 0)
			t[index] = Tb2;
		if (x == nx-1)
			t[index] = Tb3;
		if (y == ny-1)
			t[index] = Tb4;
	}

	return 1;
}

int writeData(float* t,int nx,int ny)
{
	char filename[100];
	FILE *fp;
	sprintf(filename, "temp2d.plt");
	if ((fp = fopen(filename, "w")) == NULL)
	{
		printf("cannot open the %s file\n", filename);
		return -1;
	}
	fputs("Title=\"temp field\"\n", fp);
	fputs("VARIABLES=\"X\",\"Y\",\"T\"\n", fp);
	fprintf(fp, "ZONE T=\"BOX\",I=%d,J=%d,F=POINT\n", nx, ny);
	
	for (int j = 0; j<ny; j++)
	for (int i = 0; i<nx; i++)
	{
		int idx = i*ny + j;
		fprintf(fp, "%d,%d,%f\n", i, j, t[idx]);
	}
	fclose(fp);
	printf("写入文件.\n");
	return 1;
}

void main(int argc, char** argv)
{
	double startTime, endTime;//used for time count

	const int EightConst = 8;
	int NX = EightConst * 100;
	int NY = 1000;
	int cellNum = NX*NY;
	int maxIter = 100;/*default value for iteration times*/
	int outputIter = 100;//print to scr default value

	double meanTempOfSubDomain, meanTempOfTotalDomain;

	MPI_Status Stat;
	int rowStart, rowEnd;
	//set for field variables
	float* gCurTemp, *gNexTemp;//array for current and next temp field in global domain
	float* subCurTemp, *subNexTemp;//array for sub domain field
	int subNx,subRowNum,subCellNum;//subNx=Nx/np,subCelNum=(subNx+2)*NY
	
	//init for pointer
	gCurTemp = NULL;
	gNexTemp = NULL;
	subCurTemp = NULL;
	subNexTemp = NULL;


	int mpi_init_status = MPI_Init(&argc, &argv);
	if (mpi_init_status != MPI_SUCCESS){
		printf("Error in MPI_Init()!\n");
		return;
	}


	int numOfSubDomain, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &numOfSubDomain);/*get process num as task num*/
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);/*get current process as rask index for each sub domain*/

	if (numOfSubDomain < 2){
		printf("process number is less than 2.\n");
		return;
	}

	//now comes to the master process to init total domain
	if (rank == MASTER_PROCESS)
	{
		printf("total proc: %d\nmaster proc id: %d\n",
			numOfSubDomain, MASTER_PROCESS);


		printf("input NX:\n");
		cin >> NX;
		if ((NX%numOfSubDomain) != 0)
		{
			printf("NX couldn't be divided by num of process.\n");
			return;
		}

		printf("input NY:\n");
		cin >> NY;

		printf("input iter times:\n");
		cin >> maxIter;

		printf("input output iter times:\n");
		cin >> outputIter;

		cellNum = NX*NY;
		printf("NX=%d NY=%d MaxIter=%d\n", NX, NY, maxIter);

		//operate begin now
		startTime = MPI_Wtime();

		gCurTemp = (float*)malloc(cellNum*sizeof(float));
		gNexTemp = (float*)malloc(cellNum*sizeof(float));

		initTempField(gCurTemp, NX, NY);
		initTempField(gNexTemp, NX, NY);

	}

	int iter = 0;
	
	//note that we already set NX,NY, cellNum and maxIter in master process
	//now we bcast to all process
	MPI_Bcast(&NX, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);
	MPI_Bcast(&NY, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);
	MPI_Bcast(&cellNum, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);
	MPI_Bcast(&maxIter, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);
	MPI_Bcast(&outputIter, 1, MPI_INT, MASTER_PROCESS, MPI_COMM_WORLD);


	//now we set assist array for sub domain cal 
	subNx = NX / numOfSubDomain;
	subRowNum = subNx + 2;//used for cal sub array
	subCellNum = subRowNum*NY;

	subCurTemp = (float*)malloc(subCellNum*sizeof(float));
	subNexTemp = (float*)malloc(subCellNum*sizeof(float));

	//now we prepare for Scatter to all sub process for sub domain cal
	//start recv row is rowIdx=1 in sub array
	MPI_Scatter(gCurTemp, subNx*NY, MPI_FLOAT, 
		subCurTemp + NY, subNx*NY, MPI_FLOAT,
		MASTER_PROCESS, MPI_COMM_WORLD);

	//now we recv the sub domain data
	//copy to nex temp field for sub domain
	for (int x = 1; x <= subNx;++x)
	for (int y = 0; y < NY; ++y)
	{
		const int index = x*NY + y;
		subNexTemp[index] = subCurTemp[index];
	}
	
	while (iter <= maxIter)
	{
		iter++;
		//99 88 99 88
		if (rank>MASTER_PROCESS)
		{
			//send the first row of rank  to last row of rank-1
			MPI_Send(subCurTemp + NY, NY, MPI_FLOAT, rank - 1, UP_DATA_TRANSFER, MPI_COMM_WORLD);
			//rank recv the last row of rank-1
			MPI_Recv(subCurTemp, NY, MPI_FLOAT, rank - 1, DOWN_DATA_TRANSFER, MPI_COMM_WORLD,&Stat);
		}

		if (rank < numOfSubDomain - 1)
		{
			MPI_Recv((subCurTemp + NY*(subNx + 1)), NY, MPI_FLOAT, rank + 1, UP_DATA_TRANSFER, MPI_COMM_WORLD,&Stat);
			MPI_Send((subCurTemp + NY*subNx), NY, MPI_FLOAT, rank + 1, DOWN_DATA_TRANSFER, MPI_COMM_WORLD);
		}

		if (rank == MASTER_PROCESS)
		{
			rowStart = 2;
			rowEnd = subNx;
		}
		else if (rank == (numOfSubDomain - 1))
		{
			rowStart = 1;
			rowEnd = subNx-1;
		}
		else
		{
			rowStart = 1;
			rowEnd = subNx;
		}
	
		//now we cal in each sub domain
		for (int x = rowStart; x <= rowEnd;++x)
		for (int y = 1; y <= NY - 2; ++y)
		{
			const int index = x*NY + y;
			subNexTemp[index] = subCurTemp[index]+0.1f*(
				subCurTemp[index - 1] + 
				subCurTemp[index + 1] + 
				subCurTemp[index - NY] + 
				subCurTemp[index + NY]-
				4.0f*subCurTemp[index]);

		}
#if 1
		if (iter % outputIter == 0)
		{
			meanTempOfSubDomain = 0.0;
			//meanTempOfTotalDomain = 0.0;
			for (int x = rowStart; x <= rowEnd; ++x)
			for (int y = 1; y <= NY - 2; ++y)
			{
				const int index = x*NY + y;
				meanTempOfSubDomain += subNexTemp[index];
			}

			MPI_Reduce(&meanTempOfSubDomain,&meanTempOfTotalDomain, 1, MPI_DOUBLE, MPI_SUM, MASTER_PROCESS, MPI_COMM_WORLD);

			if (rank == MASTER_PROCESS)
			{
				meanTempOfTotalDomain = meanTempOfTotalDomain / (double)cellNum;
				printf("mT=%6.2f\n",meanTempOfTotalDomain);
			}

		}
#endif
		float* tempSwap = subCurTemp;
		subCurTemp = subNexTemp;
		subNexTemp = tempSwap;

	}


	MPI_Gather(subCurTemp + NY, subNx*NY, MPI_FLOAT, gCurTemp, subNx*NY, MPI_FLOAT, MASTER_PROCESS, MPI_COMM_WORLD);

	if (rank == MASTER_PROCESS)
	{
		endTime = MPI_Wtime();
		printf("time spend:%f s\n", endTime - startTime);

		writeData(gCurTemp, NX, NY);
	}

	//MPI_Reduce

	MPI_Finalize();

}




