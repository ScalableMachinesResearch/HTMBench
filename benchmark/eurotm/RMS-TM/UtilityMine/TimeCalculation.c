#include "pardhp.h"

TM_CALLABLE
void atomic_totalhousekeeping_time(int pid, all_atomic_time * atomic_secs_time, uint64_t inside_enter, uint64_t inside_exit, uint64_t outside_enter, uint64_t outside_exit)
{
	uint64_t executing_time = inside_exit - inside_enter;
	uint64_t total_time = outside_exit - outside_enter;
        atomic_secs_time[pid].total_housekeeping_time += total_time - executing_time;
}

TM_CALLABLE
void atomic_totalexecuting_time(int pid, all_atomic_time * atomic_secs_time, uint64_t inside_enter, uint64_t inside_exit)
{
        atomic_secs_time[pid].total_executing_time += inside_exit - inside_enter;
 
}

TM_CALLABLE
void atomic_totaltime(int pid, all_atomic_time * atomic_sections_time, uint64_t outside_enter, uint64_t outside_exit)
{
    atomic_sections_time[pid].total_atomic_time += outside_exit - outside_enter;
}

TM_CALLABLE
void rearrange_atomic_total_time(int pid, all_atomic_time * atomic_sections_time, uint64_t inside_enter, uint64_t inside_exit, uint64_t outside_enter, uint64_t outside_exit)
{
  atomic_totalhousekeeping_time(pid,atomic_sections_time, inside_enter, inside_exit, outside_enter,outside_exit);
  atomic_sections_time[pid].total_executing_time += inside_exit - inside_enter;
  atomic_totalexecuting_time(pid, atomic_sections_time, inside_enter, inside_exit);
  atomic_totaltime(pid, atomic_sections_time, outside_enter, outside_exit);
 
}
