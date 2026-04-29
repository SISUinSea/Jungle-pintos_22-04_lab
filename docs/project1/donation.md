``` C

/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority (int new_priority) {
	if (thread_mlfqs)
	{
		return ;
	}
	thread_current ()->priority = new_priority;
	struct thread* thread_begin = list_entry (list_begin(&ready_list), struct thread, elem);
	if(thread_begin->priority > new_priority)
	{
		thread_yield();
	}
}

---


struct thread {
    int priority;

    struct list_elem elem;

    // int effective_priority;
    int base_priority;

    struct list donator_list; // TODO. 스레드에서 donator를 표시하기 위한 list_elem
    lock* waiting_lock;
    struct list_elem wait_elem;  // TODO. 이게 맞나..??? 아마도/...? 락은 하나만 기다릴 수 있으니까??
    

}


void
lock_acquire (struct lock *lock) {
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (!lock_held_by_current_thread (lock));

    if (lock->holder != NULL) {
        donate_to_lock_holder(lock);
    }
	sema_down (&lock->semaphore);
	lock->holder = thread_current ();
}

void
sema_down (struct semaphore *sema) {
	enum intr_level old_level;

	ASSERT (sema != NULL);
	ASSERT (!intr_context ());

	old_level = intr_disable ();
	while (sema->value == 0) {
	    리스트에 순서 맞춰서 넣기 (&sema->waiters, &thread_current ()->wait_elem);
		thread_block ();
	
	sema->value--;
	intr_set_level (old_level);
}

 
void donate_to_lock_holder (struct lock *lock) {
    cur = thread_current ();
    cur ->waiting_lock = lock;

    list_insert_order (&lock->holder->donator_list, cur->wait_elem, /*TODO. 비교함수 새롭게 만들고 그걸 넣기*/, );

    recalculate_priority (lock);
}

/* 락을 쥐고 있는 holder의 priority를 재계산 */
newFunction recalculate_priority (lock * lock) {
    t = lock->holder;
    /* 1. 변경된 t->donator_list를 가지고 priority 재계산 */
    max_donated_priority = -1
    // for donator in t->donator_list: // TODO. 바꾸기: 꼭 리스트 순회를 안 해도 되지 않음??? 정렬된 상태로 넣어둔다면 O(1)으로 최고 우선순위 값을 찾을 수 있다.
    if donator_list is not empty:
        max_donated_priority = list_front( &lock->holder->donator_list);

    temp_p = t->priority
    t->priority = max (base_priority, max_donated_prirority)
    if (temp_p != t->priority) and (t->waiting_lock != NULL):
        recalculate_priority (t->waiting_lock)

    
    // if max_donated_priority == -1:
    //     t->priority = t->base_priority;
    // else if (max_donated_priority > t->priority) {
    //     mark priority has changed;
    //     t->priority = max_donated_priority;
    // }
    // /* 2. 만약 e_p가 바뀌었고 and t가 기다리는 락이 있었다면 */
    // if ((priorityriority has changed) && (t->waiting_lock != NULL)) {
    //     recalculate_priorityriority (t->waiting_lock);
    // }
}

void
lock_release (struct lock *lock) {
	ASSERT (lock != NULL);
	ASSERT (lock_held_by_current_thread (lock));

    return_donation_back_to_donator (lock);	
    lock->holder = NULL;
	sema_up (&lock->semaphore);
}

newFunction return_donation_back_to_donator (struct lock *lock) {
    // 락 홀더의 donator_list를 정리, priority 재계산
    for donator in lock->holder->donator_list:
        if donator->waiting_lock == lock:
            remove donator in donator_list //TODO. 리스트를 순회하는 도중에 리스트 element를 지워도 되나? -> list_remove 사용
    
    recalculate_priority (lock); // TODO. 이걸 재활용 하려고 하면 infinite recursive call이 발생.. 적절한 base case로 잘라내기?
    // max_priority = 0
    // for donator in lock->holder->donator_list:
    //     max_priority = max(max_priority, donator->priority)
    // if priority < max_priority:
    //     lock->holder->priority = max_priority
    
}

void
sema_up (struct semaphore *sema) {
	enum intr_level old_level;

	ASSERT (sema != NULL);

	old_level = intr_disable ();
	if (!list_empty (&sema->waiters))
		thread_unblock (list_entry (list_pop_front (&sema->waiters),
					struct thread, elem));
	sema->value++;
	intr_set_level (old_level);
}


// TODO. thread를 처음 만들었을 때 또는 donator_list에 아무것도 없을 때 base priority를 priority로 만들어야 함.

```