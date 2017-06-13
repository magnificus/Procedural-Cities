// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "ThreadedWorker.h"

//ThreadedWorker::ThreadedWorker()
//{
//}
//
//ThreadedWorker::~ThreadedWorker()
//{
//}

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
ThreadedWorker* ThreadedWorker::Runnable = NULL;
//***********************************************************

ThreadedWorker::ThreadedWorker(TArray<uint32>& TheArray, const int32 IN_TotalPrimesToFind)
	: TotalPrimesToFind(IN_TotalPrimesToFind)
	, StopTaskCounter(0)
	, PrimesFoundCount(0)
{
	//Link to where data should be stored
	PrimeNumbers = &TheArray;

	Thread = FRunnableThread::Create(this, TEXT("ThreadedWorker"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

ThreadedWorker::~ThreadedWorker()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool ThreadedWorker::Init()
{
	//Init the Data 
	PrimeNumbers->Empty();
	PrimeNumbers->Add(2);
	PrimeNumbers->Add(3);

	return true;
}

//Run
uint32 ThreadedWorker::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	//While not told to stop this thread 
	//		and not yet finished finding Prime Numbers
	while (StopTaskCounter.GetValue() == 0 && !IsFinished())
	{
		PrimeNumbers->Add(FindNextPrimeNumber());
		PrimesFoundCount++;

		//***************************************
		//Show Incremental Results in Main Game Thread!

		//	Please note you should not create, destroy, or modify UObjects here.
		//	  Do those sort of things after all thread are completed.

		//	  All calcs for making stuff can be done in the threads
		//	     But the actual making/modifying of the UObjects should be done in main game thread.
		//ThePC->ClientMessage(FString::FromInt(PrimeNumbers->Last()));
		//***************************************

		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//prevent thread from using too many resources
		//FPlatformProcess::Sleep(0.01);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	}

	//Run ThreadedWorker::Shutdown() from the timer in Game Thread that is watching
	//to see when ThreadedWorker::IsThreadFinished()

	return 0;
}

//stop
void ThreadedWorker::Stop()
{
	StopTaskCounter.Increment();
}

ThreadedWorker* ThreadedWorker::JoyInit(TArray<uint32>& TheArray, const int32 IN_TotalPrimesToFind)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new ThreadedWorker(TheArray, IN_TotalPrimesToFind);
	}
	return Runnable;
}

void ThreadedWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void ThreadedWorker::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool ThreadedWorker::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished();
	return true;
}
int32 ThreadedWorker::FindNextPrimeNumber()
{
	//Last known prime number  + 1
	int32 TestPrime = PrimeNumbers->Last();

	bool NumIsPrime = false;
	while (!NumIsPrime)
	{
		NumIsPrime = true;

		//Try Next Number
		TestPrime++;

		//Modulus from 2 to current number - 1 
		for (int32 b = 2; b < TestPrime; b++)
		{
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//prevent thread from using too many resources
			//FPlatformProcess::Sleep(0.01);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

			if (TestPrime % b == 0)
			{
				NumIsPrime = false;
				break;
				//~~~
			}
		}
	}

	//Success!
	return TestPrime;
}