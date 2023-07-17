1. First copy the files "new_alarm_mutex.c", and "errors.h", and makefile into your
   own directory.

2. To compile the program "new_alarm_mutex.c" and run the program, use the following command:

      make start
      
3. This will execute the following:

      cc new_alarm_mutex.c -D_POSIX_PTHREAD_SEMANTICS -lpthread

      ./a.out

4. 5. At the prompt "alarm>", you can input 2 type of alarm requests

	For example: Start_Alarm(5) 10 Family Message
	For example: Cancel_Alarm(5)

(To exit from the program, type Ctrl-d.)
	
 
