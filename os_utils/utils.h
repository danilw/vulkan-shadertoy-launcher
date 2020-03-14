
struct my_time_struct{
	int msec;
	int sec;
	int min;
	int hour;
	int day;
	int month;
	int year; 
};

#if defined(_WIN32)

#include <windows.h>
#define CLOCK_MONOTONIC_RAW 0
#define BILLION                             (1E9)

int clock_gettime(int dummy, struct timespec *ct)
{
	static BOOL g_first_time = 1;
	static LARGE_INTEGER g_counts_per_sec;
	LARGE_INTEGER count;
	if (g_first_time)
	{
		g_first_time = 0;
		if (0 == QueryPerformanceFrequency(&g_counts_per_sec))
		{
			g_counts_per_sec.QuadPart = 0;
		}
	}
	if ((NULL == ct) || (g_counts_per_sec.QuadPart <= 0) ||
		(0 == QueryPerformanceCounter(&count)))
	{
		return -1;
	}
	ct->tv_sec = count.QuadPart / g_counts_per_sec.QuadPart;
	ct->tv_nsec = ((count.QuadPart % g_counts_per_sec.QuadPart) * BILLION) / g_counts_per_sec.QuadPart;

	return 0;
}

void get_local_time(struct my_time_struct *my_time){
	SYSTEMTIME lt = {0};
    GetLocalTime(&lt);
	my_time->msec=lt.wMilliseconds;
	my_time->sec=lt.wSecond;
	my_time->min=lt.wMinute;
	my_time->hour=lt.wHour;
	my_time->day=lt.wDay;
	my_time->month=lt.wMonth;
	my_time->year=lt.wYear;
}


#else

#include <sys/time.h>

void get_local_time(struct my_time_struct *my_time){
    struct timeval te; 
    gettimeofday(&te, NULL);
    time_t T= time(NULL);
    struct tm tm = *localtime(&T);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    my_time->msec=(int)(milliseconds%(1000));
	my_time->sec=tm.tm_sec;
	my_time->min=tm.tm_min;
	my_time->hour=tm.tm_hour;
	my_time->day=tm.tm_mday;
	my_time->month=tm.tm_mon+1;
	my_time->year=tm.tm_year+1900;
    return;
}

#endif

double get_time_ticks(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    double ticks = (ts.tv_sec * 1000) + (ts.tv_nsec) / 1000000.0;
    return ticks;
}

//return total time of pause pressed
float pres_pause(bool pause){
	static bool lpause=false;
	static float pause_time=0;
	static double before_time=0;
    double now_time = get_time_ticks();
    if(pause&&(!lpause)){
		lpause=true;
		before_time=now_time;
	}
    else{
		if(!pause&&(lpause)){
			lpause=false;
			pause_time+=(float)(now_time-before_time)/1000.;
		}
	}
	return pause_time;
}

float update_fps_delta(){
	static double before_time=0;
	if((int)before_time==0)before_time=get_time_ticks();
	
    double now_time = get_time_ticks();
    
    if ((long)before_time==(long)now_time) { //this happend on IMMEDIATE and MAILBOX
		return 1.0/9999.0;
    }
    double tdelta=(now_time-before_time);
    if(tdelta<=0)tdelta=1;
    float delta=(float)tdelta/1000.0;
    if(delta>1){
		delta=1;
	}
	if(delta<=0){
		delta=1.0/9999.0;
	}
    before_time=now_time;
    return delta;
}

// cross-platform sleep function
void sleep_ms(int milliseconds) 
{
#ifdef WIN32
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
}

void FPS_LOCK(int fps){
	static double before_time=0;
	if(before_time==0)before_time=get_time_ticks();
	double now_time=get_time_ticks();
	if(before_time==now_time)return;
	if(fps<=0)fps=1;
	double wdelta=((double)(1.0/(double)fps)*(double)1000.0);
	double tdelta=(now_time-before_time);
	if(tdelta<=0)tdelta=1;
	double rdelta=wdelta-tdelta;
	if(rdelta<=0){
		before_time=now_time;
		return;
	}
	sleep_ms((int)rdelta);
	before_time=get_time_ticks();
	
	return;
	
}


