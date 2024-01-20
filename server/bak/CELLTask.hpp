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
	// ��������
	std::list<CellTaskPtr> _tasks;
	// �������ݻ�����
	std::list<CellTaskPtr> _tasksBuf;
	// �ı����ݻ�������Ҫ����
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
			// �ӻ�����ȡ������
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lg(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}

			// ���û������
			if (_tasks.empty())
			{
				// ��Ϣһ��
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			// ��������
			for (auto pTask : _tasks)
			{
				pTask->doTask();
				//delete pTask; // ����һ�������Ҫ���ٸ����񣬣�ps�� ��Ϊÿһ��������new�����ģ�ÿһ��new����������Ӧ��delete��
			}
			// �������
			_tasks.clear();
		}
	}

};
#endif
