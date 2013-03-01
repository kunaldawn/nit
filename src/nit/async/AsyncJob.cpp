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

#include "nit_pch.h"

#include "nit/async/AsyncJob.h"
#include "nit/runtime/ErrorHandler.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

AsyncJob::AsyncJob()
{
	_manager			= NULL;
	_status			= JOB_IDLE;

	_subJobCount		= 0;
}

AsyncJob::~AsyncJob()
{
	// TODO: warn if abnormal

	// if state is still DOING or Managed job present, something is wrong!
}

void AsyncJob::enqueueSubJob(AsyncJob* subJob)
{
	ASSERT_THROW(subJob->_parentJob == NULL, EX_INVALID_STATE);
	ASSERT_THROW(_manager != NULL, EX_INVALID_STATE);

	subJob->_parentJob = this;
	_manager->enqueue(subJob);
	++_subJobCount;
}

void AsyncJob::subJobFinished(AsyncJob* subJob, Status status)
{
	--_subJobCount;
	onSubJobFinished(subJob, status);
}

bool AsyncJob::prepare()
{
	if (isPrepared())
		return true;

	return onPrepare();
}

bool AsyncJob::execute(bool async)
{
	return onExecute(async);
}

void AsyncJob::finish()
{
	onFinish();

	if (!_parentJob) return;

	// if a ParentJob exists, notify to ParentJob
	Status st = getStatus();
	switch (st)
	{
	case AsyncJob::JOB_SUCCESS:
	case AsyncJob::JOB_FAILED:
	case AsyncJob::JOB_CANCELED:
	case AsyncJob::JOB_ERROR:
		_parentJob->subJobFinished(this, st);
		break;
    default: break;
	}
}

void AsyncJob::retry(bool prepared)
{
	switch (getStatus())
	{
	case JOB_DOING:
	case JOB_SUCCESS:
	case JOB_FAILED:
		setStatus(prepared ? JOB_PENDING : JOB_PREPARING);
		break;

	default:
		NIT_THROW(EX_INVALID_STATE);
	}
}

bool AsyncJob::isDone()
{
	switch (getStatus())
	{
	case JOB_SUCCESS:
	case JOB_FAILED:
	case JOB_CANCELED:
	case JOB_ERROR:
		return true;

	default:
		return false;
	}
}

void AsyncJob::cancel(bool join)
{
	if (_manager)
	{
		_manager->cancel(this, join);
	}
	else
	{
		setStatus(JOB_CANCELED);
	}
}

AsyncJob::Status AsyncJob::setStatus(Status stat)
{
	Mutex::ScopedLock lock(_mutex); 

	if (_status != JOB_CANCELED)
	{
		_status = stat;
	}

	return _status;
}

////////////////////////////////////////////////////////////////////////////////

void AsyncJobQueue::enqueue(AsyncJob* job)
{
	lock();
	_queue.push_back(job);
	unlock();

	_ready.set();
}

bool AsyncJobQueue::eject(AsyncJob* job)
{
	bool ejected = false;

	lock();
	Queue::iterator itr = std::find(_queue.begin(), _queue.end(), job);
	if (itr != _queue.end())
	{
		ejected = true;
		_queue.erase(itr);
	}
	unlock();

	return ejected;
}

AsyncJob* AsyncJobQueue::pop()
{
	AsyncJob* job = NULL;

	lock();
	if (!_queue.empty())
	{
		job = _queue.front();
		_queue.pop_front();
	}
	unlock();

	return job;
}

void AsyncJobQueue::clear(bool decRef, bool cancel)
{
	lock();

	if (decRef)
	{
		for (uint i=0; i<_queue.size(); ++i)
		{
			if (cancel)
				_queue[i]->setStatus(AsyncJob::JOB_CANCELED);
			_queue[i]->decRefCount();
		}
	}
	_queue.clear();

	unlock();
}

////////////////////////////////////////////////////////////////////////////////

AsyncJobManager::AsyncJobManager(const String& name, uint numWorkers)
{
	if (numWorkers < 1)
		numWorkers = 1;

	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	_name = name;

	_nextWorkerID = 0;
	_jobCount = 0;

	addWorkers(numWorkers);
}

void AsyncJobManager::onDelete()
{
	stop();
}

void AsyncJobManager::addWorkers(uint count)
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	for (uint i=0; i < count; ++i)
	{
		AsyncWorker* worker = new AsyncWorker(this, StringUtil::format("%s#%d", _name.c_str(), _nextWorkerID++), false);
		_workers.push_back(worker);
	}

	resume();
}

void AsyncJobManager::resume()
{
	for (uint i=0; i < _workers.size(); ++i)
	{
		_workers[i]->Resume();
	}
}

void AsyncJobManager::suspend(bool join)
{
	for (uint i=0; i < _workers.size(); ++i)
	{
		_workers[i]->Suspend(false);
		_asyncIn.ready();				// TODO: use semaphore?
	}

	if (!join) return;

	_asyncIn.ready();

	for (uint i=0; i < _workers.size(); ++i)
	{
		_workers[i]->Suspend(true);
	}
}

void AsyncJobManager::stop()
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	for (uint i=0; i < _workers.size(); ++i)
	{
		_workers[i]->Stop(false);
		_asyncIn.ready();				// TODO: use semaphore?
	}

	for (uint i=0; i < _workers.size(); ++i)
	{
		_workers[i]->Stop(true);
	}

	for (uint i=0; i < _workers.size(); ++i)
	{
		delete _workers[i];
	}

	_workers.clear();

	_nextWorkerID = 0;

	// no doing at now, job must be in only one queue
	if (getDoingCount() > 0)
	{
		ASSERT_THROW(getDoingCount() == 0, EX_INVALID_STATE);
	}

	_prepQueue.clear(true, true);
	_asyncIn.clear(true, true);
	_asyncOut.clear(true, true);
}

void AsyncJobManager::enqueue(AsyncJob* job)
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);
	ASSERT_THROW(job->_manager == NULL, EX_INVALID_STATE);

	++_jobCount;
	job->incRefCount(); // Increase manually here to prevent threads to touch ref count
	job->_manager = this;
	job->setStatus(AsyncJob::JOB_PREPARING);

	if (job->isPrepared())
	{
		// it's already ready state: skip prep queue and directly into async-in
		_asyncIn.enqueue(job);
	}
	else
	{
		_prepQueue.enqueue(job);
	}
}

void AsyncJobManager::cancel(AsyncJob* job, bool join)
{
	// TODO: Not sufficient implementation. Proper finish and cancel propagation should be considered
	NIT_THROW(EX_NOT_IMPLEMENTED);

	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);
	ASSERT_THROW(job->_manager == this, EX_INVALID_STATE);

	if (!join)
	{
		// let the queue handle natural ejection
		job->setStatus(AsyncJob::JOB_CANCELED);
		return;
	}

	// NOTE: eject and join would be ineffective

	if (!eject(job))
	{
		while (join)
		{
			join = false;

			for (uint i=0; i<_workers.size(); ++i)
			{
				if (_workers[i]->IsDoing(job))
				{
					join = true;
					break;
				}
			}

			if (join) Thread::sleep(10);
		}
	}

	job->setStatus(AsyncJob::JOB_CANCELED);
	release(job);
}

void AsyncJobManager::release(Ref<AsyncJob> job)
{
	--_jobCount;
	job->_manager = NULL;
	job->decRefCount();
}

void AsyncJobManager::update()
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	// TODO: apply timeLimit

	while (true)
	{
		AsyncJob* job = _prepQueue.pop();
		if (job == NULL) break;

		if (job->isCanceled())
		{
			// out if canceled
			release(job);
			return;
		}

		if (job->prepare())
		{
			// ready success : enlist to async-in
			job->setStatus(AsyncJob::JOB_PENDING);
			_asyncIn.enqueue(job);
		}
		else
		{
			// ready fail : enlist to async-out
			job->setStatus(AsyncJob::JOB_ERROR);
			_asyncOut.enqueue(job);
		}
	}

	while (true)
	{
		Ref<AsyncJob> job = _asyncOut.pop();
		if (job == NULL) break;

		// call Finish()
		switch (job->getStatus())
		{
		case AsyncJob::JOB_DOING:
		case AsyncJob::JOB_SUCCESS:
		case AsyncJob::JOB_FAILED:
			job->finish();
			break;
        default:
            break;
		}

		// Retry or Release
		switch (job->getStatus())
		{
		case AsyncJob::JOB_PREPARING:
			// return to preparing state
			_prepQueue.enqueue(job);
			break;

		case AsyncJob::JOB_PENDING:
			// return to pending state
			_asyncIn.enqueue(job);
			break;

		case AsyncJob::JOB_DOING:
			ASSERT_THROW(false, EX_INVALID_STATE);

		default:
			// other states (IDLE, SUCCESS etc) : job released here and out of Manager scope
			release(job);
		}
	}
}

bool AsyncJobManager::eject(Ref<AsyncJob> job)
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);
	ASSERT_THROW(job->_manager == this, EX_INVALID_STATE);

	bool ejected = _prepQueue.eject(job) || _asyncIn.eject(job) || _asyncOut.eject(job);

	if (ejected)
	{
		// on successful eject: other threads not yet touched, release it.
		job->setStatus(AsyncJob::JOB_IDLE);
		release(job);
	}

	return ejected;
}

AsyncJob* AsyncJobManager::nextJob()
{
	ASSERT_THROW(Thread::current() != NULL, EX_ACCESS);

	AsyncJob* job = _asyncIn.pop();
	
	if (job)
	{
		_doingMutex.lock();
		_doing.insert(job);
		_doingMutex.unlock();
	}

	return job;
}

void AsyncJobManager::waitJob()
{
	ASSERT_THROW(Thread::current() != NULL, EX_ACCESS);

	_asyncIn.wait();
}

void AsyncJobManager::jobDone(AsyncJob* job, bool success)
{
	ASSERT_THROW(Thread::current() != NULL, EX_ACCESS);

	_doingMutex.lock();
	_doing.erase(job);
	_doingMutex.unlock();

	switch (job->getStatus())
	{
	case AsyncJob::JOB_DOING:
		if (success)
			job->setStatus(AsyncJob::JOB_SUCCESS);
		else
			job->setStatus(AsyncJob::JOB_FAILED);
		_asyncOut.enqueue(job);
		break;

	case AsyncJob::JOB_PREPARING:
		_prepQueue.enqueue(job);
		break;

	case AsyncJob::JOB_PENDING:
		_asyncIn.enqueue(job);
		break;

	default:
		_asyncOut.enqueue(job);
	}
}

////////////////////////////////////////////////////////////////////////////////

AsyncWorker::AsyncWorker(AsyncJobManager* manager, const String& name, bool suspended)
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	_name			= name;

	_manager		= manager;

	_suspending	= false;
	_suspended		= suspended;
	_stopping		= false;
	_stopped		= false;

	_thread		= NULL;
}

AsyncWorker::~AsyncWorker()
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	if (_thread)
	{
		Stop(true);

		_thread->join();

		delete _thread;
	}
}

bool AsyncWorker::WaitResumed()
{
	while (true)
	{
		_mutex.lock();
		_currentJob = NULL;
		bool suspended = _suspending || _suspended;

		if (suspended)
		{
			_suspending = false;
			_suspended = true;
			_mutex.unlock();
			_resumeReady.wait();
		}
		else
		{
			bool stopping = _stopping || _stopped;
			_mutex.unlock();
			return !stopping;
		}
	}
}

void AsyncWorker::Suspend(bool join)
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	if (_thread == NULL)
		return;

	{ 
		Mutex::ScopedLock lock(_mutex); 

		if (_stopping || _stopped || _suspending || _suspended) return;

		_suspending = true; 
	}

	if (!join) return;

	while (true)
	{
		bool done;

		{ Mutex::ScopedLock lock(_mutex); done = _suspended || _stopped; }

		if (done) break;

		Thread::sleep(10);
	}
}

void AsyncWorker::Resume()
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	if (_thread == NULL)
	{
		_thread = new Thread(_name.c_str());
		_thread->setPriority(Thread::PRIO_LOW);
		_thread->start(*this);
	}
	else
	{
		_mutex.lock();
		_suspended = false;
		_suspending = false;
		_mutex.unlock();

		_resumeReady.set();
	}
}

void AsyncWorker::Stop(bool join)
{
	ASSERT_THROW(Thread::current() == NULL, EX_ACCESS);

	if (_thread == NULL)
		return;

	{
		Mutex::ScopedLock lock(_mutex);

		if (_stopping || _stopped) return;

		_suspending = false;
		_suspended = false;
		_stopping = true;
	}

	_resumeReady.set();

	if (!join) return;

	while (true)
	{
		bool stopped;

		{ Mutex::ScopedLock lock(_mutex); stopped = _stopped; }

		if (stopped) break;

		Thread::sleep(10);
	}
}

void AsyncWorker::run()
{
	Thread* thread = Thread::current();

	ASSERT_THROW(thread == _thread, EX_ACCESS);

	LOG(0, "&& AsyncWorker '%s' started on cpu %d\n", thread->name().c_str(), GetCurrentProcessorNumber());

	while (WaitResumed())
	{
		AsyncJob* job = SetCurrent(_manager->nextJob());

		if (job == NULL)
		{
			_manager->waitJob();
			continue;
		}

		try
		{
			bool success = false;

			if (job->setStatus(AsyncJob::JOB_DOING) == AsyncJob::JOB_DOING)
				success = job->execute(true);

			_manager->jobDone(job, success);

			continue;
		}
		catch (Exception& ex)
		{
			ErrorHandler::handle(ex);
		}
		catch (std::exception& ex)
		{
			ErrorHandler::handle(ex);
		}
		catch (...)
		{
			ErrorHandler::handle();
		}

		job->setStatus(AsyncJob::JOB_ERROR);
		_manager->jobDone(job, false);
	}

	LOG(0, "&& AsyncWorker '%s' finished on cpu %d\n", thread->name().c_str(), GetCurrentProcessorNumber());

	{ Mutex::ScopedLock lock(_mutex); _stopped = true; }
}

bool AsyncWorker::IsDoing(AsyncJob* job)
{
	Mutex::ScopedLock lock(_mutex);

	return _currentJob == job;
}

AsyncJob* AsyncWorker::SetCurrent(AsyncJob* job)
{
	Mutex::ScopedLock lock(_mutex);

	// NOTE: always release before wait.
	_currentJob = job;

	return job;
}

// TODO: move to Thread.h
int AsyncWorker::GetCurrentProcessorNumber()
{
#ifdef NIT_WIN32
    return ::GetCurrentProcessorNumber();
#else
    return -1; // TODO: impl
#endif
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
