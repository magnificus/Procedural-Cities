#include "City.h"
#include "ThreadedWorker.h"



//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
ThreadedWorker* ThreadedWorker::Runnable = NULL;
//***********************************************************
FThreadSafeCounter  WorkerCounter = 0;
ThreadedWorker::ThreadedWorker(AHouseBuilder* house)
	: houseBuilder(house)
{
	//Link to where data should be stored
	Thread = FRunnableThread::Create(this, *FString::Printf(TEXT("Thread %i"), WorkerCounter.Increment()), 0, TPri_Normal); //windows default = 8mb for thread, could specify more
}

ThreadedWorker::~ThreadedWorker()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool ThreadedWorker::Init()
{
	return true;
}

//Run
uint32 ThreadedWorker::Run()
{

	resultingInfo = houseBuilder->getHouseInfo();
	done = true;
	return 0;
}

//stop
void ThreadedWorker::Stop()
{
	StopTaskCounter.Increment();
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