#include <iostream>
#include <pthread.h>
#include <queue>
#include <deque>
#include <unistd.h>
#include "semaphore.h"
#include "main.h"
#include "iterable_queue.h"

//Buffer capacity
#define MAX_BUFFER  9

//Number of products created by the producer
#define NUM_PRODUCED  50

//Number of times a consumer attempts to access a product
#define NUM_ATTEMPTS_A 20
#define NUM_ATTEMPTS_B 20
#define NUM_ATTEMPTS_C 20

//Consumers' and producer's sleep times in miliseconds
#define TIME_A  100
#define TIME_B  300
#define TIME_C  200
#define TIME_P  150

using namespace std;

struct Product
{
    unsigned id;
    char who_read;

    Product() : id(0), who_read('N') {}
};


iterable_queue<Product> buffer;  //Buffer is a FIFO queue
bool isProducerWorking = false;  //Used only for better printing


Semaphore full(0),            //counts the number of slots that are full in buffer
          empty(MAX_BUFFER);  //counts the number of slots that are empty in buffer
          

Mutex cregion,  //mutual exclusion for critical region
	  blockA,   //blocking consumer A
	  blockB,   //blocking consumer B
	  blockC;   //blocking consumer C

          
const char indent[] = "                                   ";


void printBuffer()
{
    cout << "BUFFER: ";
    for(auto it=buffer.begin(); it!=buffer.end(); ++it)
        cout << it->id << " ";
    cout << endl;
}


void* produce(void *ptr) 
{
    unsigned sleep_time = *((unsigned*) ptr); //Sleep time in miliseconds
    Product new_product;

    cregion.p();
        isProducerWorking = true;
    cregion.v();

    for (int i = 1; i <= NUM_PRODUCED; ++ i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);
        
        new_product.id = i;

        empty.p();
        cregion.p();

        buffer.push(new_product);
        cout << "PRODUCED ELEMENT:  " << i << endl;
        //cout << "Current buffer size: " << buffer.size() << endl;
        printBuffer();

        cregion.v();
        full.v();
    }

    cregion.p();
        isProducerWorking = false;
    cregion.v();
    cout << "\n**********************PRODUCER THREAD ENDS**********************\n" << endl;
}


void* consume(void *ptr) 
{
    unsigned sleep_time = *((unsigned*) ptr); //Sleep time in miliseconds
    char consumer_id;
    switch (sleep_time) {
    	case TIME_A: 
    		consumer_id = 'A';
            readA(sleep_time, consumer_id);
    		break;
    	case TIME_B: 
    		consumer_id = 'B';
            readB(sleep_time, consumer_id);
    		break;
		case TIME_C: 
    		consumer_id = 'C';
            readC(sleep_time, consumer_id);
    		break;
    	default:
    		throw runtime_error("Wrong argument in consume()");
    }
    cout << "\n*********************COMSUMER " << consumer_id << " THREAD ENDS**********************\n\n";
}


void readA(unsigned &sleep_time, char &consumer_id)
{
    for(int i = 1; i <= NUM_ATTEMPTS_A; ++i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);

        cout << indent << "   " << consumer_id << " wants to read: " << buffer.front().id << endl;

        blockC.try_p();
        blockA.p();
        full.p();
        cregion.p();
        
        if(buffer.front().who_read == 'B') {
            cout << indent << consumer_id << " READ & DELETED PRODUCT: " << buffer.front().id << endl;
            buffer.pop();
            if(!isProducerWorking) printBuffer();

            cregion.v();
            empty.v();
            blockB.v();
            blockA.v();
            blockC.v();
        }
        else {
        	buffer.front().who_read = consumer_id;
            cout << indent << consumer_id << " READ PRODUCT: " 
                << buffer.front().id << endl;

            cregion.v();
            full.v();
            
        }
    }
}


void readB(unsigned &sleep_time, char &consumer_id)
{
    for(int i = 1; i <= NUM_ATTEMPTS_B; ++i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);
        
        cout << indent << "   " << consumer_id << " wants to read: " << buffer.front().id << endl;

        blockC.try_p();
        blockB.p();
        full.p();
        cregion.p();
        
        if(buffer.front().who_read == 'A') {
            cout << indent << consumer_id << " READ & DELETED PRODUCT: " << buffer.front().id << endl;
            buffer.pop();
            if(!isProducerWorking) printBuffer();

            cregion.v();
            empty.v();
            blockA.v();
            blockB.v();
        	blockC.v();
        }
        else {
        	buffer.front().who_read = consumer_id;
            cout << indent << consumer_id << " READ PRODUCT: " << buffer.front().id << endl;

            cregion.v();
            full.v();
        }
    }
}


void readC(unsigned &sleep_time, char &consumer_id)
{
    for(int i = 1; i <= NUM_ATTEMPTS_C; ++i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);
        
        cout << indent << "   " << consumer_id << " wants to read: " << buffer.front().id << endl;

        blockC.p();
        full.p();
        cregion.p();

        char arb = buffer.front().who_read;
        if(arb == 'A' || arb == 'B') {
        	cout << indent << "   " << consumer_id << " was declined access to: " << buffer.front().id << endl;
        	
            cregion.v();
	        full.v();
        } 
        else {
	        cout << indent << consumer_id << " READ & DELETED PRODUCT: " << buffer.front().id << endl;
	        buffer.pop();
            if(!isProducerWorking) printBuffer();

	        cregion.v();
	        empty.v();
    	}

        blockC.v();
    }
}

int main(void) 
{
    pthread_t producer, consumerA, consumerB, consumerC;

    unsigned *timeP = new unsigned(TIME_P),
             *timeA = new unsigned(TIME_A), 
             *timeB = new unsigned(TIME_B), 
             *timeC = new unsigned(TIME_C);

    cout << "! ======================START OF SIMULATION===================== !" << endl;

    pthread_create(&producer, NULL, produce, (void*) timeP);
    pthread_create(&consumerA, NULL, consume, (void*) timeA);
    pthread_create(&consumerB, NULL, consume, (void*) timeB);
    pthread_create(&consumerC, NULL, consume, (void*) timeC);

    //pthread_join(producer, NULL);
    pthread_join(consumerA, NULL);
    pthread_join(consumerB, NULL);
    pthread_join(consumerC, NULL);

    cout << "! ======================END OF SIMULATION====================== !" << endl;
    cout << "Elements left in buffer: " << buffer.size() << endl;

    delete timeP;
    delete timeA;
    delete timeB;
    delete timeC;

    return 0;
}
