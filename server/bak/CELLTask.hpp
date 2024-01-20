#ifndef _CELLTask_hpp_
#define _CELLTask_hpp_

#include <thread>
#include <list>
#include <mutex>

class CellTask
{
public:
	CellTask()
	{

	}
	virtual ~CellTask()
	{
		
	}

	virtual void doTask()
	{
		
	}
};

typedef std::shared_ptr<CellTask> CellTaskPtr;
class CellTaskServer
{
private:
	// 任务数据
	std::list<CellTaskPtr> _tasks;
	// 任务数据缓冲区
	std::list<CellTaskPtr> _tasksBuf;
	// 改变数据缓冲区需要加锁
	std::mutex _mutex;
public:
	void addTask(CellTaskPtr& task)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		_tasksBuf.push_back(task);
	}
	void Start()
	{
		std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
		t.detach();
	}
protected:
	void OnRun()
	{
		while (true)
		{
			// 从缓冲区取出数据
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lg(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}

			// 如果没有任务
			if (_tasks.empty())
			{
				// 休息一下
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			// 处理任务
			for (auto pTask : _tasks)
			{
				pTask->doTask();
				//delete pTask; // 做完一个任务就要销毁该任务，（ps： 因为每一个任务都是new出来的，每一个new都必须有相应的delete）
			}
			// 清空任务
			_tasks.clear();
		}
	}

};
#endif
