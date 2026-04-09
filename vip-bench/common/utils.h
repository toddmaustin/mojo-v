#ifndef UTILS_H
#define UTILS_H

void mysrand(unsigned int seed);
unsigned int myrand(void);

#include<iostream>
#include<chrono>
#include<string>
#include<stdio.h>
#include<sys/mman.h>
#include<unistd.h>
#include<assert.h>
#include<linux/perf_event.h>
#include<sys/syscall.h>
#include<asm/unistd.h>
#include<sys/ioctl.h>
#include<string.h>


long perf_event_open(perf_event_attr*, int, int, int, unsigned long);
void record_mem();
void record_mem(std::string fileName );
//int  record_mem();

class Stopwatch
  {
    public:
      static double timeTaken;
      static bool precision;
      static int numIter;
      static int nSlots;
      static bool tableFormat;
      struct perf_event_attr pe;
	    int fd;
      Stopwatch(std::string timer_name):name_(timer_name)
      {
        
        // Linux Perf Event Utility to Measure Instruction Count
        memset(&pe, 0, sizeof(struct perf_event_attr));
		    pe.type = PERF_TYPE_HARDWARE;
		    pe.size = sizeof(struct perf_event_attr);
		    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
		    pe.disabled = 1;
		    pe.exclude_kernel = 1;
		    pe.exclude_hv = 1;
		    fd = perf_event_open(&pe, 0, -1, -1, 0);
		    if (fd == -1) {
		        fprintf(stderr, "WARNING: could not access performance monitoring kernel facility (leader %llx).\n", pe.config);
		    }
        else {
		      ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		      ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
        }
        std::cerr<<"\t";
        //start memory usage measurement
        // record_mem();
        //start the high resolution timer
        start_time_=(std::chrono::high_resolution_clock::now());
        //start processor cycle counter
        __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
        cycles=((uint64_t)hi << 32) | lo;
      }
      ~Stopwatch()
      {
        __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
        cycles=(((uint64_t)hi << 32) | lo) - cycles;
        auto end_time=std::chrono::high_resolution_clock::now();
        auto duration=std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
        auto duration_millis=std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
        if(precision)
          timeTaken=duration.count();
        else
          timeTaken=duration_millis.count();
        if(!tableFormat)
          std::cerr<<"[VIP] Time taken:"<<timeTaken/numIter<<(precision?" microseconds, ":" milliseconds, ")<<cycles<<" processor cycles";
        else
          std::cerr<<"[VIP] Time taken:\t"<</*timeTaken/numIter*/duration_millis.count()<<"\t"<<(timeTaken/numIter)/nSlots<<"\t"<<cycles;
        // Print Linux Perf Event Utility to Measure Instruction Count
        long long count;
        if (fd != -1) {
		      ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
          if (read(fd, &count, sizeof(long long)) != sizeof(long long))
          {
            fprintf(stderr, "Cannot read perf counters.\n");
            exit(1);
          }
          if(!tableFormat)
            fprintf(stderr, " %lld instructions executed\t", count);
          else
          fprintf(stderr, "\t%lld\n", count);
		      close(fd);
        }
        record_mem();
        std::cerr<<std::endl;
      }
    private: 
      std::string name_;
      std::chrono::high_resolution_clock::time_point start_time_;
      uint64_t cycles;
      unsigned int lo,hi;
  };


#ifdef notdef
    class Stopwatch
    {
        public:
            static double timeTaken;
            static bool precision;
            static int numIter;
            Stopwatch(std::string timer_name):name_(timer_name), start_time_(std::chrono::high_resolution_clock::now())
            {}
            ~Stopwatch()
            {
                auto end_time=std::chrono::high_resolution_clock::now();
                auto duration=std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
                auto duration_millis=std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);
                if(precision)
                    timeTaken=duration.count();
                else
                    timeTaken=duration_millis.count();
                
                std::cerr<<name_<<": "<<timeTaken/numIter<<(precision?" microseconds":" milliseconds")<<std::endl;
            }
        private:
            std::string name_;
            std::chrono::high_resolution_clock::time_point start_time_;
    };
#endif


#endif /* UTILS_H */
