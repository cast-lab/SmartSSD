/*
 Samsung implementation of DeQueue using circular array 

*/

#pragma once

#include<iostream> 
using namespace std;

// Maximum size of array or Dequeue 
#define MAX 10000 

// A structure to represent a Deque and keeps type T
template <class T>
class S_Deque {
	T sArray[MAX];
	int front;
	int rear;
	int size;
public:
	S_Deque() {
		front = -1;
		rear = 0;
		size = 1000;
	}
	
	S_Deque(int size) {
		front = -1;
		rear = 0;
		this->size = size;
	}

	// Operations on Deque: 
	void s_insertFront(T key) {
		// check whether Deque if full or not 
		if (s_isFull()) {
			cout << "Overflow\n" << endl;
			return;
		}
		// If queue is initially empty 
		if (front == -1) {
			front = 0;
			rear = 0;
		}

		// front is at first position of queue 
		else if (front == 0)
			front = size - 1;

		else // decrement front end by '1' 
			front = front - 1;
		// insert current element into Deque 
		sArray[front] = key;
	}

	void s_insertRear(T key) {
		if (s_isFull()) {
			cout << " Overflow\n " << endl;
			return;
		}

		// If queue is initially empty 
		if (front == -1) {
			front = 0;
			rear = 0;
		}

		// rear is at last position of queue 
		else if (rear == size - 1)
			rear = 0;

		// increment rear end by '1' 
		else
			rear = rear + 1;

		// insert current element into Deque 
		sArray[rear] = key;
	}

	void s_deleteFront() {
		// check whether Deque is empty or not 
		if (s_isEmpty()) {
			cout << "Queue Underflow\n" << endl;
			return;
		}

		// Deque has only one element 
		if (front == rear) {
			front = -1;
			rear = -1;
		}
		else
			// back to initial position 
			if (front == size - 1)
				front = 0;

			else // increment front by '1' to remove current 
				// front value from Deque 
				front = front + 1;
	}

	void s_deleteRear() {
		if (s_isEmpty()) {
			cout << " Underflow\n" << endl;
			return;
		}

		// Deque has only one element 
		if (front == rear) {
			front = -1;
			rear = -1;
		}
		else if (rear == 0)
			rear = size - 1;
		else
			rear = rear - 1;
	}

	bool s_isFull() {
		return ((front == 0 && rear == size - 1) || front == rear + 1);
	}

	bool s_isEmpty() {
		return (front == -1);
	}

	T s_getFront() {
		// check whether Deque is empty or not 
		if (s_isEmpty()) {
			cout << " Underflow\n" << endl;
			//return -1;
		}
		return sArray[front];
	}

	T s_getRear() {
		// check whether Deque is empty or not 
		if (s_isEmpty() || rear < 0) {
			cout << " Underflow\n" << endl;
			//return -1;
		}
		return sArray[rear];
	}

	int s_getSize() {
		return size;
	}
};