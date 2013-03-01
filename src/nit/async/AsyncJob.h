/// nit - Noriter Framework
/// A Cross-platform Open Source Integration for Game-oriented Apps
///
/// http://www.github.com/ellongrey/nit
///
/// Copyright (c) 2013 by Jun-hyeok Jang
/// 
/// (see each file to see the different copyright owners)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// Author: ellongrey

#pragma once

#include "nit/nit.h"

#include "nit/async/EventSemaphore.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

class AsyncJob;
class AsyncJobManager;
class AsyncJobQueue;
class AsyncWorker;

////////////////////////////////////////////////////////////////////////////////

class NIT_API AsyncJob : public RefCounted
{
public:
	AsyncJob();
	virtual ~AsyncJob();

public:
	enum Status
	{
		JOB_IDLE,						// not registered to manager yet

		JOB_PREPARING,					// Enqueueed, waiting for Prepare() call
		JOB_PENDING,					// Done Prepare(), waiting for Execute() call

		JOB_DOING,						// calling Execute()      (can Retry())
		JOB_SUCCESS,					// Execute(): true  - job success (can Retry() on Finish())
		JOB_FAILED,						// Execute(): false - job fail    (can Retry() on Finish())

		JOB_CANCELED,					// cancel() called outside           (can't Finish() - will OnDelete() )
		JOB_ERROR,						// exception thrown or error occurred, hence terminated (can't Finish() - will OnDelete())
	};

	Status								getStatus()								{ Mutex::ScopedLock lock(_mutex); return _status; }

	virtual bool						isPrepared()							{ return false; }

	bool								isCanceled()							{ return getStatus() == JOB_CANCELED; }
	bool								isDoing()								{ return getStatus() == JOB_DOING; }
	bool								isDone();

	uint								getSubJobCount()						{ return _subJobCount; }
	AsyncJob*							getParentJob()							{ return _parentJob; }

public:
	void								cancel(bool join);

protected:								// Implementation support
	Mutex&								getMutex()								{ return _mutex; }

	virtual bool						onPrepare() = 0;
	virtual bool						onExecute(bool async) = 0;
	virtual void						onSubJobFinished(AsyncJob* subJob, Status status) { }
	virtual void						onFinish() = 0;

	void								enqueueSubJob(AsyncJob* subJob);

	void								retry(bool prepared);					// possible on Execute(), Finish()

private:
	friend class AsyncJobManager;
	friend class AsyncWorker;
	friend class AsyncJobQueue;

	AsyncJobManager*					_manager;
	Status								_status;

	Weak<AsyncJob>						_parentJob;
	uint								_subJobCount;

	Mutex								_mutex;

	bool								prepare();
	bool								execute(bool async);
	void								finish();

	void								subJobFinished(AsyncJob* subJob, Status status);

	Status								setStatus(Status stat);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API AsyncJobQueue
{
public:									// ScopedLock support
	void								lock()									{ _mutex.lock(); }
	void								unlock()								{ _mutex.unlock(); }

public:
	uint								getCount()								{ lock(); uint count = _queue.size(); unlock(); return count; }

	void								enqueue(AsyncJob* job);
	bool								eject(AsyncJob* job);
	AsyncJob*							pop();
	void								clear(bool decRef, bool cancel);
	void								wait()									{ _ready.wait(); }

	void								ready()									{ _ready.set(); }

private:
	typedef deque<AsyncJob*>::type		Queue;

	Queue								_queue;
	Mutex								_mutex;
	EventSemaphore						_ready;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API AsyncJobManager : public RefCounted
{
public:
	AsyncJobManager(const String& name, uint numWorkers);

public:
	const String&						getName()								{ return _name; }

	uint								getWorkerCount()						{ return _workers.size(); }

	uint								getJobCount()							{ return _jobCount; }

	uint								getPrepCount()							{ return _prepQueue.getCount(); }
	uint								getPendingCount()						{ return _asyncIn.getCount(); }
	uint	/* TODO: remove? */			getDoingCount()							{ Mutex::ScopedLock lock(_doingMutex); return _doing.size(); }
	uint								getDoneCount()							{ return _asyncOut.getCount(); }

public:
	void								addWorkers(uint count);

	void								resume();
	void								suspend(bool join = true);
	void								stop();

public:
	void								update();

public:
	void								enqueue(AsyncJob* job);
	void								cancel(AsyncJob* job, bool join);

private:								// for workers
	friend class AsyncWorker;
	AsyncJob*							nextJob();
	void								waitJob();
	void								jobDone(AsyncJob* job, bool success);

protected:
	virtual void						onDelete();

protected:
	typedef vector<AsyncWorker*>::type	Workers;

	String								_name;

	Workers								_workers;
	uint								_nextWorkerID;

	uint								_jobCount;

	AsyncJobQueue						_prepQueue;
	AsyncJobQueue						_asyncIn;
	AsyncJobQueue						_asyncOut;

	Mutex								_doingMutex;
	set<AsyncJob*>::type				_doing;

	bool								eject(Ref<AsyncJob> job);
	void								release(Ref<AsyncJob> job);
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API AsyncWorker : public Runnable
{
public:
	AsyncWorker(AsyncJobManager* manager, const String& name, bool suspended);
	virtual ~AsyncWorker();

public:
	void 								Resume();
	void 								Suspend(bool join);
	void 								Stop(bool join);
	bool								IsDoing(AsyncJob* job);

protected:								// Runnable Impl
	virtual void						run();

protected:
	String								_name;

	AsyncJobManager*					_manager;

	bool								_suspending;
	bool								_suspended;
	bool								_stopping;
	bool								_stopped;

	Mutex								_mutex;

	EventSemaphore						_resumeReady;
	
	AsyncJob*							_currentJob;

	Thread*								_thread;

	bool								WaitResumed();
	AsyncJob*							SetCurrent(AsyncJob* job);
    
    static int                          GetCurrentProcessorNumber();
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;