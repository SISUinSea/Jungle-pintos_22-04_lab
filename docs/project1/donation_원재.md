priority 비교 함수
a_thread = a에서 thread 꺼냄
b_thread = b에서 thread 꺼냄

a_thread의 max_p > b_thread의 max_p 이면 a가 먼저

struct thread
1. waiting_lock
   내가 지금 기다리는 lock 하나
2. donor
   나에게 priority를 기부한 thread 하나
   여러 명 저장 안 함
3. max_p
   실제로 스케줄링에 쓰는 priority
4. base_p
   원래 priority
   thread_set_priority가 바꾸는 값

max_p 계산

나에게 donor가 없으면:
  max_p = base_p
나에게 donor가 있으면:
  max_p = max(base_p, donor의 max_p)

set_priority

내 base_p를 새 priority로 바꿈
내 max_p를 다시 계산
내가 waiting_lock을 기다리는 중이면:
  그 lock의 주인에게
  내 max_p를 기준으로 donation 갱신 요청
donation 갱신 함수

입력:
  donor = priority를 주는 thread
  receiver = priority를 받는 thread
receiver의 donor를 donor로 설정
receiver의 이전 max_p 기억
receiver의 max_p 다시 계산
receiver의 max_p가 이전보다 커졌고,
receiver도 어떤 lock을 기다리는 중이면:
  receiver가 기다리는 lock의 주인에게
  receiver를 donor로 해서 donation 갱신 요청

lock_acquire

lock 주인이 없으면:
  그냥 lock 획득 시도
lock 주인이 있으면:
  내 waiting_lock = 이 lock
  lock 주인에게 나를 donor로 donation 갱신 요청

sema_down으로 lock 기다림

깨어나서 lock을 얻으면:
  내 waiting_lock = NULL
  lock 주인 = 나

lock_release

내가 lock을 release함
만약 내 donor가 있고,
그 donor의 waiting_lock이 지금 release하는 lock이면:
  내 donor = NULL
내 max_p 다시 계산
lock 주인 = NULL
sema_up으로 다음 waiter 깨움