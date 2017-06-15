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

ThreadedWorker::ThreadedWorker(AHouseBuilder* house, FHousePolygon p, float floorHeight, float maxRoomArea, bool shellOnly, bool simple, bool fullReplacement)
	: houseBuilder(house)
	, housePol(p)
	, floorHeight(floorHeight)
	, maxRoomArea(maxRoomArea)
	, shellOnly(shellOnly)
	, simple(simple)
	, fullReplacement(fullReplacement)
{
	//Link to where data should be stored
	Thread = FRunnableThread::Create(this, TEXT("ThreadedWorker"), 0, TPri_AboveNormal); //windows default = 8mb for thread, could specify more
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
	return true;
}

//Run
uint32 ThreadedWorker::Run()
{
	//house
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	resultingInfo = houseBuilder->getHouseInfo(housePol, floorHeight, maxRoomArea, shellOnly);
	//While not told to stop this thread 
	//		and not yet finished finding Prime Numbers
	//while (StopTaskCounter.GetValue() == 0 && !IsFinished())
	//{
		//PrimeNumbers->Add(FindNextPrimeNumber());
	//	PrimesFoundCount++;

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
	//}

	//Run ThreadedWorker::Shutdown() from the timer in Game Thread that is watching
	//to see when ThreadedWorker::IsThreadFinished()
	done = true;
	return 0;
}

//stop
void ThreadedWorker::Stop()
{
	StopTaskCounter.Increment();
}

ThreadedWorker* ThreadedWorker::JoyInit(AHouseBuilder* house, FHousePolygon p, float floorHeight, float maxRoomArea, bool shellOnly, bool simple, bool fullReplacement)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new ThreadedWorker(house, p, floorHeight, maxRoomArea, shellOnly, simple, fullReplacement);
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