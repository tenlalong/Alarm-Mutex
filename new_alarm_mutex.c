/*
 * alarm_mutex.c
 *
 * This is an enhancement to the alarm_thread.c program, which
 * created an "alarm thread" for each alarm command. This new
 * version uses a single alarm thread, which reads the next
 * entry in a list. The main thread places new requests onto the
 * list, in order of absolute expiration time. The list is
 * protected by a mutex, and the alarm thread sleeps for at
 * least 1 second, each iteration, to ensure that the main
 * thread can lock the mutex to add new work to the list.
 */
#include <pthread.h>
#include <time.h>
#include "errors.h"

/*
 * The "alarm" structure now contains the time_t (time since the
 * Epoch, in seconds) for each alarm, so that they can be
 * sorted. Storing the requested number of seconds would not be
 * enough, since the "alarm thread" cannot tell how long it has
 * been on the list.
 */
typedef struct alarm_tag
{
    struct 	alarm_tag *link;
    int         alarm_id;
    int         seconds;
    time_t      time; 
    char        message[128];
    char        category[16];
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t d_cond = PTHREAD_COND_INITIALIZER;
alarm_t *alarm_list = NULL;
alarm_t *current_alarm = NULL;
char Ncate[10][16]; //stores unquie Message_category
int ncate = 0; 
int count = 0;
char storesS[100][300] = {"a"}; //stores lines of the user input
int countS = 0;
int firstM = 0;

/*
 * The alarm thread's start routine.
 */
void *alarm_thread(void *arg)
{
	alarm_t *alarm;
    int sleep_time;
    time_t now;
    int status;
    pthread_t thread;
    ncate = 0;
    int i = 0;

     /*
     * Loop forever, processing commands. The alarm thread will
     * be disintegrated when the process exits.
     */
    while (1)
    {
        // locks the status
	    status = pthread_mutex_lock(&alarm_mutex);
	    if (status != 0)
		    err_abort(status, "Lock mutex");

	    alarm = alarm_list;

        /*
        * Checks if the alarm list is empty, waits for one seconds.
        * Allowing the main thread to run and also read the commands.
        * If the list is not empty, checks its Message_category is unique
        * if the message_category is unique comparing to the exisiting ones
        * fromt he list Ncate[], creates a thread message_category and then calls
        * display thread. If not calls the display thread
        */
	    if (alarm == NULL)
	    {
		    sleep_time = 1;
	    }
	    else
	    {
	        // loop of size 10, that means 10 message_category supported.
		    for (i = 0; i < 10; i++)
		    {
		        // checks if this alarm category is unique or not
			    if (strcmp(Ncate[i], alarm->category) == 0)
			    {
				    ncate = 1;
			    }
		    }

            /*
            * if the alarm category is unique.
            */
		    if (ncate == 0)
		    {
		        // stores in the Ncate[] list, now not unique
			    strcpy( Ncate[count], alarm->category);

                // initialize
			    count++;
			    ncate = 0;

			    alarm_list = alarm->link;
			    current_alarm = alarm;
			    
			    // creates a thread Message_category
			    printf("Create New Display Alarm Thread for Message_Category %s to Display Alarm (%d) at %d: %s \n", alarm->category, alarm->alarm_id, alarm->seconds, alarm->message);
			    status = pthread_cond_signal(&d_cond);
			    if (status != 0)
				    err_abort(status, "signal cond");
		    }
		    /*
		    * not unique category.
		    */
		    else
		    {
		        // calls the display thread
			    alarm_list = alarm->link;
			    current_alarm = alarm;
			    status = pthread_cond_signal(&d_cond);
			    if (status != 0)
				    err_abort(status, "signal cond");
		    }
	    }
	    
	    // unlocks the thread
	    status = pthread_mutex_unlock(&alarm_mutex);
	    if (status != 0)
		    err_abort(status, "Unlock mutex");
	    sleep(sleep_time);
    }
}


void *display_thread(void *arg)
{
	alarm_t *alarm;
	int status;
	time_t now;
    
     /*
     * Loop forever, processing commands. The alarm thread will
     * be disintegrated when the process exits.
     */
	while (1)
	{
	    // locks the thread
		status = pthread_mutex_lock(&alarm_mutex);
		if (status != 0)
			err_abort(status, "Lock mutex");

        // waits for alarm thread to signal so the alrm is ready to go
		status = pthread_cond_wait(&d_cond, &alarm_mutex);
		if (status != 0)
			err_abort(status, "wait on cond");
		alarm = current_alarm;

        // while the alarm is not expiered the thread alarm prints a message every 
        // Time seconds
		while (alarm->time > time (NULL))
		{
			printf("Alarm %d Printed by Alarm Thread Display Thread %ld at %ld : %s \n", alarm->alarm_id, pthread_self(), alarm->time - time(NULL), alarm->message);
			sleep(1);
		}

        // unlock the thread
		status = pthread_mutex_unlock(&alarm_mutex);
		if (status != 0)
			err_abort(status, "unlock mutex");

		free(alarm);
	}

	free(alarm);
}

int main(int argc, char *argv[])
{
    int status;
    char line[128];
    int alarmT;
    int thisAlarm;
    int i = 0;
    int k = 0;
    int identified = 0;
    alarm_t *alarm, **last, *next;
    pthread_t thread; // alarm thread
    pthread_t d1_thread; //display thread
    int countM = 0;

    // temporary stores fore user input
    int seconds_;
    int id_;
    int time_;
    char message_[128];
    char category_[16];
    char temp_category_[16];
    int temp_time_;

    // initialize the array firstM
    if (firstM == 0)
    {
	    for (k = 0; k < 100; k++)
	    {
		    strcpy(storesS[k],"");
	    }
    }
    firstM = 1;

    // initialize threads
    status = pthread_create(&thread, NULL, alarm_thread, NULL);
    if (status != 0)
        err_abort(status, "Create alarm thread");
    status = pthread_create(&d1_thread, NULL, display_thread, NULL);
    if (status != 0)
	    err_abort(status, "display thread one");

    while (1)
    {
        printf("alarm> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            exit(0);
        if (strlen(line) <= 1)
            continue;
        alarm = (alarm_t *)malloc(sizeof(alarm_t));
        if (alarm == NULL)
            errno_abort("Allocate alarm");

        /*
         * Parse input line contains id a %d, seconds a %d, Massage_category
         * containing %s and message %s of 128 cahracters seperated from seconds
         * by whitespaces called message
         */
         
	if ((sscanf(line, "Start_Alarm(%d) %d %s %128[^\n]", &alarm->alarm_id, &alarm->seconds, alarm->category, alarm->message) < 3) && (sscanf(line, "Cancel_Alarm(%d)", &alarm->alarm_id) < 1))
        {
            fprintf(stderr, "Bad command\n");
            free(alarm);
        }
	else if (!(sscanf(line, "Start_Alarm(%d) %d %s %128[^\n]", &alarm->alarm_id, &alarm->seconds, alarm->category, alarm->message) < 3))
        {
		strcpy(storesS[countS], line);
		countS++;

         /*
         * sorts the list by the smallest id
         */
         
         // locks the thread
        status = pthread_mutex_lock(&alarm_mutex);
            if (status != 0)
                err_abort(status, "Lock mutex");

	    alarm->time = time(NULL) + alarm->seconds;

	    last = &alarm_list;
	    next = *last;
	    while (next != NULL)
	    {
		    if (next -> alarm_id >= alarm-> alarm_id)
		    {
			    alarm->link = next;
			    *last = alarm;
			    break;
		    }
		    last = &next->link;
		    next = next->link;
	    }

        /*
        * If we reached the end of the list, insert the new
        * alarm there. ("next" is NULL, and "last" points
        * to the link field of the last item, or to the
        * list header).
        */
	    if (next == NULL)
	    {
		    *last = alarm;
		    alarm->link = NULL;
	    }
        }
	else if (!(sscanf (line, "Cancel_Alarm(%d)", &alarm->alarm_id) < 1))
	{
	    // locks the thread
		status = pthread_mutex_lock(&alarm_mutex);
		if (status != 0)
			err_abort(status, "Lock mutex");

        //intialize
		i = 0;
		countM = 0;

        // goes through the storesS[] list and checks the cancel alarm
        // prints the alarm is cancelled
		do
		{
			sscanf(storesS[i], "Start_Alarm(%d) %d %s %128[^\n]", &id_, &seconds_, category_, message_);

			if (alarm->alarm_id == id_)
			{
				printf("Alarm %d Canceled at %d: %s \n", id_, seconds_, message_);
				strcpy(temp_category_,category_);
				temp_time_ = seconds_;
			}
			i++;
		} while (strcmp(storesS[i],"") != 0);

		i = 0;

        // goes thorought the storeS[] lsit of lines and checks if the Message_Category
        // for the particuler alarm is the only one or not.
		do
		{
			sscanf(storesS[i], "Start_Alarm(%d) %d %s %128[^\n]", &id_, &seconds_, category_, message_);
			
			if (strcmp(category_, temp_category_) == 0) countM = countM + 1;
			
			i++;
		} while (strcmp(storesS[i],"") != 0);

        // if it is prints the thread is terminated
		if (countM == 1)
		{
			printf("Display Alarm Thread %ld for Message_Category %s Terminated at %d \n", pthread_self(), alarm->category, temp_time_);
		}
        
        //close the thread
		status = pthread_mutex_unlock(&alarm_mutex);
		if (status != 0)
			err_abort(status, "Unlock_mutex");
	}

#ifdef DEBUG
            printf ("[list: ");
            for (next = alarm_list; next != NULL; next = next->link)
                printf ("%d(%d)[\"%s\"] ", next->time,
                    next->time - time (NULL), next->message);
            printf ("]\n");
#endif
            status = pthread_mutex_unlock (&alarm_mutex);
            if (status != 0)
                err_abort (status, "Unlock mutex");
    }
}
