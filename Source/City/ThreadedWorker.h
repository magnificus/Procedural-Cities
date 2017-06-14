// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HouseBuilder.h"
//~~~~~ Multi Threading ~~~
class ThreadedWorker : public FRunnable
{
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  ThreadedWorker* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** The Data Ptr */
	TArray<uint32>* PrimeNumbers;

	AHouseBuilder &houseBuilder;
	FHousePolygon &housePol;
	/** The PC */
	//AVictoryGamePlayerController* ThePC;
	FHouseInfo resultingInfo;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

private:
	int32				PrimesFoundCount;
public:

	bool done = false;

	//Done?
	bool IsFinished() const
	{
		return done;
	}

	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	ThreadedWorker(AHouseBuilder& house, FHousePolygon p, float floorHeight, float maxRoomArea, bool shellOnly, bool simple);
	virtual ~ThreadedWorker();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();



	//~~~ Starting and Stopping Thread ~~~



	/*
	Start the thread and the worker from static (easy access)!
	This code ensures only 1 Prime Number thread will be able to run at a time.
	This function returns a handle to the newly started instance.
	*/
	static ThreadedWorker* JoyInit(AHouseBuilder& house, FHousePolygon p, float floorHeight, float maxRoomArea, bool shellOnly, bool simple);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();

};