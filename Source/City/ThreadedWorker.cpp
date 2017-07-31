// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "ThreadedWorker.h"



//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
ThreadedWorker* ThreadedWorker::Runnable = NULL;
//***********************************************************

ThreadedWorker::ThreadedWorker(AHouseBuilder* house, bool shellOnly, bool simple, bool fullReplacement)
	: houseBuilder(house)
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
	return true;
}

//Run
uint32 ThreadedWorker::Run()
{

	resultingInfo = houseBuilder->getHouseInfo(shellOnly);
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