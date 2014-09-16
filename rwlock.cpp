/*
 * =====================================================================================
 *
 *       Filename:  rwlock.cpp
 *
 *    Description:  rwlock
 *
 *        Version:  1.0
 *        Created:  09/16/14 21:30:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Fritz Mehner (mn), mehner@fh-swf.de
 *        Company:  FH Südwestfalen, Iserlohn
 *
 * =====================================================================================
 */
#include<thread>
#include<mutex>
#include<iostream>
#include<unistd.h>
#include<condition_variable>
using namespace std;
class rwlock {
	private:
		mutex _lock;
		condition_variable _wcon, _rcon;
		unsigned _writer, _reader;
		int _active;
	public:
		void read_lock() {
			unique_lock<mutex> lock(_lock);
			++_reader;
			while(_active < 0 || _writer > 0)
				_rcon.wait(lock);
			--_reader;
			++_active;
		}
		void write_lock() {
			unique_lock<mutex> lock(_lock);
			++_writer;
			while(_active != 0)
				_wcon.wait(lock);
			--_writer;
			_active = -1;
		}
		void unlock() {
			unique_lock<mutex> lock(_lock);
			if(_active > 0) {
				--_active;
				if(_active == 0) _wcon.notify_one();
			}else{
				_active = 0;
				if(_writer > 0) _wcon.notify_one();
				else if(_reader > 0) _rcon.notify_all();
			}

		}
		rwlock():_writer(0),_reader(0),_active(0){
		}
};

void t1(rwlock* rwl) {
	while(1) {
		cout << "I want to write." << endl;
		rwl->write_lock();
		cout << "writing..." << endl;
		sleep(5);
		rwl->unlock();
		sleep(5);
	}
}

void t2(rwlock* rwl) {
	while(1) {
		cout << "t2-I want to read." << endl;
		rwl->read_lock();
		cout << "t2-reading..." << endl;
		sleep(1);
		rwl->unlock();
	}
}

void t3(rwlock* rwl) {
	while(1) {
		cout << "t3-I want to read." << endl;
		rwl->read_lock();
		cout << "t3-reading..." << endl;
		sleep(1);
		rwl->unlock();
	}
}

int main(){
	rwlock* rwl = new rwlock();
	thread th1(t1,rwl);
	thread th2(t2,rwl);
	thread th3(t3,rwl);
	th1.join();
	th2.join();
	th3.join();
	return 0;
}
