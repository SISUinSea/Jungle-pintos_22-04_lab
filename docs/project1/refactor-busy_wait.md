``` C

static void
timer_interrupt (struct intr_frame *args UNUSED) {
	ticks++;
	thread_tick ();
}

```

여기서 해야 하는거
1. sleep list + global tick 확인
2. 깨울 thread들 찾기
3. 걔네들을 ready queue에 넣기
4. global tick을 업데이트하기


---

``` C

/* Suspends execution for approximately TICKS timer ticks. */
void
timer_sleep (int64_t ticks) {
	int64_t start = timer_ticks ();

	ASSERT (intr_get_level () == INTR_ON);
	// while (timer_elapsed (start) < ticks)
	// 	thread_yield ();

    if (timer_elapsed (start) < ticks) { // 아직 틱 값이 다 안 지났으면
        sleep queue에 넣기
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