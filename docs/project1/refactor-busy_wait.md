# thread_init
``` c
void
thread_init (void) {
	ASSERT (intr_get_level () == INTR_OFF);

	/* Reload the temporal gdt for the kernel
	 * This gdt does not include the user context.
	 * The kernel will rebuild the gdt with user context, in gdt_init (). */
	struct desc_ptr gdt_ds = {
		.size = sizeof (gdt) - 1,
		.address = (uint64_t) gdt
	};
	lgdt (&gdt_ds);

	/* Init the globla thread context */
	lock_init (&tid_lock);
	list_init (&ready_list);
	list_init (&destruction_req);

	/* Set up a thread structure for the running thread. */
	initial_thread = running_thread ();
	init_thread (initial_thread, "main", PRI_DEFAULT);
	initial_thread->status = THREAD_RUNNING;
	initial_thread->tid = allocate_tid ();
}
```
list_init (&sleep_list); 초기화

# 정의 및 선언
``` c
static struct list ready_list;
static struct list sleep_list;
```
thread.c에 sleep_list 선언

---
# timer_sleep
``` C
/* Suspends execution for approximately TICKS timer ticks. */
void
timer_sleep (int64_t ticks) {
	int64_t start = timer_ticks ();

	ASSERT (intr_get_level () == INTR_ON);
	// while (timer_elapsed (start) < ticks)
	// 	thread_yield ();

    if (timer_elapsed (start) < ticks) { // 아직 틱 값이 다 안 지났으면
        thread_sleep (ticks); //1. block 2. sleep queue에 넣기
    }

}
```
여기서 해야 하는 거
1. thread current 호출, 현재 cur을 sleep queue에 넣기
2. cur->state = BLOCKED   상태 변경
3. 

TODO. 쓰레드 구조체에 언제 깨워야 하는지 시간 저장.
변수 이름은 wake_up_tick

TODO.  질문 : 왜 timer.c에서는 int64_t 로 tick을 선언하는데 thread에서는 long long으로 선언함???
> 일단 크기는 똑같다. 둘다 64 bit(8 byte);
> 경: int64_t로 tick을 먼저 선언했으니까 다음부터는 longlong으로 해도 충분하다.

# timer_interrupt
``` C

static void
timer_interrupt (struct intr_frame *args UNUSED) { //
	ticks++;
	thread_tick ();

	thread_wakeup ();  // 이 줄 추가하기
}

```
여기서 해야 하는거 -> thread_wakeup () 추가하기.

# thread_sleep
``` C
/* Suspends execution for approximately TICKS timer ticks. */
void
thread_sleep (int64_t ticks) {

	1. 현재 스레드를 블럭한다.
	2. 일어날 로컬 틱(wakeup_tick)을 설정한다. <- (timer_ticks () + ticks)
	3. sleep 큐에 넣는다.
	4. 아 맞다 앞 뒤로 인터럽트 방금모 껐다 켜기
	5. 필요한 경우 next_wakeup_tick 업데이트

}
```
todo: global next_wakeup_tick 어디에 만들어야 할까? timer.c인가?
	> 



# thread_wakeup

thread_wakeup () 는 만들 필요 없음. 대신 timer_interrupt에 구현해야 함. 아니면 함수로 따로 구현해서 거기에 그대로 넣는 것도 좋은데??
> 그럼 timer_interrupt에 thread_wakeup을 호출하도록 설계.

``` C
/* Suspends execution for approximately TICKS timer ticks. */
void
thread_wakeup () {

	1. next_wakeup_tick이 timer_ticks와 같다면(알람 울릴 시간)
	2. sleep_list에서 next_wakeup_tick와 기상 시각이 같은 thread들에게 깨우는 작업을 수행
	3. 깨우는 작업이란, 다음을 의미함.
		3.1. 쓰레드의 상태를 BLOCKED-> READY로 변경
		3.2. ready list에 추가
	4. next_wakeup_tick을 sleep list에서 가장 빠른 시각으로 변경
	5. 이 작업을 하는 동안, list를 만지는 동안에 interrupt는 무시한다.

}
```

wakeup_tick = find_min_tic();
# set_wakeup_tick
``` C
set_next_wakeup_tick () {
	1. 기존의 next_wakeup_tick 과 sleep_list[0]->wakeup_tick 비교한다.
	2. new_wakeup_tick가 더 작다면, next_wakeup_tick를 업데이트한다.
	3. 아님 말고
}
```

# get_wakeup_tick
필요 할지는 의문이다.
``` C
get_wakeup_tick (int new_wakeup_tick) {
	return next_wakeup_tick;
}
```