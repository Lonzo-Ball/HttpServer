#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <iostream>
#include <queue>
#include <pthread.h>

typedef int (*handler_t)(int);
class Task{
private:
	int sock;
	handler_t handler;
public:
	Task():sock(-1),handler(NULL)
	{}

	void SetTask(int sock_,handler_t handler_){
		sock = sock_;
		handler = handler_;
	}

	void Run(){
		handler(sock);
	}

	~Task(){
		sock = -1;
		handler = NULL;
	}
};

class ThreadPool{
private:
	int thread_total_num;  //线程总数
	int thread_idle_num;  //处于休眠状态的线程
	std::queue<Task> task_queue;  //任务队列
	pthread_mutex_t lock;  //互斥锁
	pthread_cond_t cond;  //条件变量
	bool is_quit;  //标记线程是否退出
public:
	ThreadPool(int num = 5):thread_total_num(num),thread_idle_num(0),is_quit(false)
	{
		pthread_mutex_init(&lock,NULL);
		pthread_cond_init(&cond,NULL);
	}

	//对任务队列加锁
	void LockQueue(){
		pthread_mutex_lock(&lock);
	}

	//对任务队列解锁
	void UnlockQueue(){
		pthread_mutex_unlock(&lock);
	}

	//判断任务队列是否为空
	bool IsEmpty(){
		return task_queue.size() == 0;
	}

	//从任务队列中pop掉一个任务
	void PopTask(){
		task_queue.pop();
	}

	//从任务队列拿一个任务
	Task GetTask(){
		return task_queue.front();
	}
	
	//唤醒一个进程
	void WakeupOneThread(){
		pthread_cond_signal(&cond);
	}

	void ThreadIdle(){
		if(is_quit)
		{
		    UnlockQueue();
		    thread_total_num--;
		    pthread_exit((void*)0);
		}

		thread_idle_num++;
		pthread_cond_wait(&cond,&lock);
		thread_idle_num--;
	}

	static void *thread_routine(void *arg){
		ThreadPool *tp = (ThreadPool*)arg;
		pthread_detach(pthread_self());

		while(1){
			tp->LockQueue();
			while(tp->IsEmpty())
			     tp->ThreadIdle();

			Task t;
			t = tp->GetTask();
			tp->PopTask();
			tp->UnlockQueue();
			t.Run();
		}
	}

	void InitThreadPool(){
		for(int i = 0;i < thread_total_num;++i){
			pthread_t tid;
			pthread_create(&tid,NULL,thread_routine,this);
		}
	}

	void PushTask(Task t){
		LockQueue();
		if(is_quit)
		{
		    UnlockQueue();
		    return;
		}

		task_queue.push(t);
		WakeupOneThread();
		UnlockQueue();
	}

	~ThreadPool(){
		pthread_mutex_destroy(&lock);
		pthread_cond_destroy(&cond);
	}
};

#endif //...
