semafory: main.cpp monitor.h
	g++ -pthread main.cpp -o semafory

clean:
	rm semafory