#ifndef MAIN_H
#define MAIN_H

void* produce(void *ptr);
void* consume(void *ptr);
void indent();
void readAB(unsigned &sleep_time, char &consumer_id);
void readC(unsigned &sleep_time, char &consumer_id);


#endif // MAIN_H