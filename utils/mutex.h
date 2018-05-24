#ifndef MUTEX_H
#define MUTEX_H

class Mutex {
  public:
	Mutex();
	~Mutex();
	void lock();
	void unlock();
  private:
	class Impl;
	Impl* impl;
};

#endif
