struct thread:
    waiting_lock 내가 지금 기다리는 lock
    도네이터 리스트: 기부 받은 priority(아마 리스트 속의 리스트 형태?)
    base_priority 원래 priority

lock_acquire: 우리 테스트에서는 High_스레드(현재_스레드)가 lock을 얻으려고 함. 
    1.lock holder가 없다면 sema_down 바로 실행. value가 1이어서 sema_down 바로 통과되고 lock 획득.(lock_init에서 일단 value 1로 설정해줌)
    2.lock holder가 있고 || lock holder가 현재_스레드보다 우선순위가 작다면:
        lock.holder의 donator 리스트에 푸쉬백
        donator 리스트에서 list_sort 후 제일 앞에 있는 값으로 priority 설정
        sema_down으로 lock 기다림
    (lock_release로 깨어나서 lock을 얻고 난 후)
    내 waiting_lock = NULL
    lock->holder = 현재_스레드(나)

lock_release: donation을 받은 현재_스레드가 들고 있던 lock을 내려놓고, 그 lock을 기다리던 스레드가 도전할 수 있게 하는 함수
ㄴ> 우리 테스트에서는 현재_스레드가 Low임
    현재_스레드의 donator가 있고 || 이 donator의 waiting_lock과 지금 release하는 lock이 같다면:
        현재_스레드의 donator = NULL
    현재_스레드의 원래 priority를 base_priority로 복구
    lock->holder = NULL
    sema_up으로 기다리던 High 스레드 깨움(대신 이때 정렬을 한 번 더 해야 됨. 근데 언제 깨어나는지 모르겠음)