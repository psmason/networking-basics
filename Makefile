task:
	g++ server.m.cpp -o server.tsk

client: 
	g++ client.m.cpp -o client.tsk

clean:
	rm server.tsk client.tsk
