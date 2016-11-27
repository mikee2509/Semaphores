#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include "monitor.h"
#include "main.h"

//Buffer capacity
#define MAX_BUFFER  9

//Number of products created by the producer
#define NUM_PRODUCED  50

//Number of times a consumer attempts to access a product
#define NUM_ATTEMPTS_A 40
#define NUM_ATTEMPTS_B 20
#define NUM_ATTEMPTS_C 30

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

queue<Product> buffer; //Buffer is a FIFO queue

Semaphore mutex(1),          //mutual exclusion for critical region
          full(0),           //buffer overfill watch
          empty(MAX_BUFFER); //buffer empty watch


void* produce(void *ptr) {

    unsigned sleep_time = *((unsigned*) ptr); //Sleep time in miliseconds
    Product temp;

    for (int i = 1; i <= NUM_PRODUCED; ++ i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);
        
        temp.id = i;

        empty.p();
        mutex.p();

        buffer.push(temp);
        cout << "Produced element:    " << i << endl;
        cout << "Current buffer size: " << buffer.size() << endl;

        mutex.v();
        full.v();
    }
    cout << "\n**********************PRODUCER THREAD ENDS**********************\n" 
        << endl;
}


void* consume(void *ptr) {

    unsigned sleep_time = *((unsigned*) ptr); //Sleep time in miliseconds
    char consumer_id;
    switch (sleep_time) {
    	case TIME_A: 
    		consumer_id = 'A';
            readAB(sleep_time, consumer_id);
    		break;
    	case TIME_B: 
    		consumer_id = 'B';
            readAB(sleep_time, consumer_id);
    		break;
		case TIME_C: 
    		consumer_id = 'C';
            readC(sleep_time, consumer_id);
    		break;
    	default:
    		cout << "THIS SHOULD NEVER HAPPEN" << endl; ///TODO: change to throw here
    }
    cout << "\n*********************COMSUMER " << consumer_id << " THREAD ENDS**********************\n\n";
}

void indent() { 
    cout << "                           "; 
}

void readAB(unsigned &sleep_time, char &consumer_id)
{
    int num = consumer_id == 'A' ? NUM_ATTEMPTS_A : NUM_ATTEMPTS_B; 
    for(int i = 1; i <= num; ++i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);

        full.p();
        mutex.p();

        char arb = buffer.front().who_read; //arb- already read by
        
        if(arb == 'A' || arb == 'B') {
            if(arb == consumer_id) {
                indent();
                cout << "COMSUMER " << consumer_id << " could NOT read product: " 
                    << buffer.front().id << endl;

                mutex.v();
                full.v();
            }
            else {
                indent();
                cout << "COMSUMER " << consumer_id << " read & deleted product: " 
                    << buffer.front().id << endl;
                buffer.pop();
                //indent();
                //cout << "Current buffer size: " << buffer.size() << endl;
            

                mutex.v();
                empty.v();
            }
        }
        else {
            buffer.front().who_read = consumer_id;
            indent();
            cout << "COMSUMER " << consumer_id << " read product: " 
                << buffer.front().id << endl;
            mutex.v();
            full.v();
        }
    }
}

void readC(unsigned &sleep_time, char &consumer_id)
{
    for(int i = 1; i <= NUM_ATTEMPTS_C; ++i) {
        usleep(sleep_time * 1000); //int usleep(useconds_t microseconds);
        
        full.p();
        mutex.p();

        char arb = buffer.front().who_read; //arb- already read by
        
        //N stands for nobody
        if(arb == 'N') {
            indent();
            cout << "COMSUMER " << consumer_id << " read & deleted product: " 
                << buffer.front().id << endl;
            buffer.pop();
            //indent();
            //cout << "Current buffer size: " << buffer.size() << endl;


            mutex.v();
            empty.v();
        }
        else {
            indent();
            cout << "COMSUMER " << consumer_id << " could NOT read product: " 
                << buffer.front().id << endl;

            mutex.v();
            full.v();
        }
        mutex.v();
    }
}

int main(void) {

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
