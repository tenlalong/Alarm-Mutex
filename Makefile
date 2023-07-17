# Start_Alarm(ID) <time> <Category> <Message>
# Start_Alarm(1234) 5 Family Some

compile:
	cc new_alarm_mutex.c -D POSIX_PTHREAD_SEMANTICSc -lpthread

run:
	./a.out

start:  compile run
