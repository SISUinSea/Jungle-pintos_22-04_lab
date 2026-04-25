# Project 1 Alarm Clock 정리

작성일: 2026-04-24
범위: `pintos/threads`

## 목적

Project 1의 첫 구현 대상인 Alarm Clock의 요구사항, 공개 테스트, 빠른 실행 명령, 구현 큰 그림을 한 곳에 정리한다.

참고한 문서:

- `/Users/sisu/Downloads/pintos-kaist-ko/project1/introduction.html`
- `/Users/sisu/Downloads/pintos-kaist-ko/project1/alarm_clock.html`

관련 코드:

- `pintos/devices/timer.c`
- `pintos/include/threads/thread.h`
- `pintos/tests/threads/*`

## 1. Alarm Clock 테스트 목록

공개된 alarm clock 테스트는 6개다.

- `alarm-single`
- `alarm-multiple`
- `alarm-simultaneous`
- `alarm-priority`
- `alarm-zero`
- `alarm-negative`

테스트 등록 위치:

- `pintos/tests/threads/tests.c`
- `pintos/tests/threads/Make.tests`
- `pintos/tests/threads/Rubric.alarm`

각 테스트의 의미는 아래와 같다.

### `alarm-single`

서로 다른 sleep 시간을 가진 여러 스레드가 한 번씩 잠들고, 깨는 순서가 올바른지 본다.

핵심 확인:

- 더 일찍 깨워야 하는 스레드가 먼저 깨어나는가
- `timer_sleep()`이 실제 시간 기준으로 동작하는가

### `alarm-multiple`

`alarm-single`과 비슷하지만 여러 번 반복한다.

핵심 확인:

- 한 번만 우연히 맞는 게 아니라 반복적으로 올바르게 동작하는가

### `alarm-simultaneous`

여러 스레드가 같은 시각에 깨어나야 할 때 실제로 같은 tick에 깨어나는지 본다.

핵심 확인:

- 같은 wakeup tick을 가진 스레드들이 함께 깨워지는가

### `alarm-priority`

같은 시각에 여러 스레드가 깨어난 뒤, 우선순위가 높은 스레드가 먼저 실행되는지 본다.

핵심 확인:

- 깨우는 것만이 아니라, ready 상태로 넣은 뒤 스케줄링 결과도 자연스럽게 맞는가

### `alarm-zero`

`timer_sleep(0)`이 즉시 반환되는지 본다.

핵심 확인:

- 0 tick sleep 요청에서 불필요하게 block 하지 않는가

### `alarm-negative`

`timer_sleep(-100)`이 크래시 없이 즉시 반환되는지 본다.

핵심 확인:

- 음수 입력을 안전하게 처리하는가

## 2. Alarm Clock에서 실제로 해야 하는 일

문서 요구사항의 핵심은 `timer_sleep()`을 다시 구현하는 것이다.

현재 제공 구현은 busy waiting 방식이다. 즉, 시간이 지났는지 계속 확인하면서 `thread_yield()`를 반복한다. 문서상 이 방식은 금지다.

## 3. AS IS -> TO BE

### AS IS

현재 `timer_sleep()`은 대략 이런 식이다.

```c
int64_t start = timer_ticks ();
while (timer_elapsed (start) < ticks)
  thread_yield ();
```

> TODO. Question: timer_ticks는 뭐하는 함수지? timer_elapsed는? thread_yield는?? 구현 코드 살펴봐야 하는건가?
이 방식은 CPU를 양보하긴 하지만, "아직 잘 시간이 남았는지"를 계속 확인하며 돌아간다. 즉, 완전히 잠드는 게 아니라 계속 깨어나서 시계를 보는 구조다.

### TO BE

`timer_sleep()`은 아래 흐름으로 바뀌어야 한다.

1. 현재 스레드가 "언제 깨워야 하는지"를 기록한다.
2. 잠자는 스레드 목록에 넣는다.
3. 현재 스레드를 block 상태로 바꾼다.
4. 타이머 인터럽트가 tick마다 목록을 보다가, wakeup tick이 된 스레드를 unblock 한다.

이 구조에서는 잠자는 동안 해당 스레드가 CPU를 거의 쓰지 않는다.

## 4. 비유로 이해하기

### 비유

AS IS는 "낮잠 자는 사람이 1분마다 스스로 눈을 떠서 아직 시간이 안 됐으면 다시 눕는 방식"이다.

TO BE는 "프런트 데스크에 `120틱 되면 깨워 달라`고 맡기고 진짜 잠드는 방식"이다. 시간이 되면 프런트가 깨워서 대기줄에 다시 세운다.

### 실제 예시

현재 tick이 `100`이라고 하자.

- 스레드 A가 `timer_sleep(20)` 호출
- 스레드 B가 `timer_sleep(40)` 호출

AS IS:

- A와 B는 계속 시간을 확인하며 `yield`를 반복한다.
- CPU를 불필요하게 계속 건드린다.

TO BE:

- A는 `wakeup_tick = 120`
- B는 `wakeup_tick = 140`
- 둘 다 sleep list에 들어가고 `BLOCKED` 상태가 된다.
- 타이머 인터럽트가 `120`에서 A를 깨우고, `140`에서 B를 깨운다.

## 5. 구현 포인트

Alarm Clock만 놓고 보면 보통 아래 변경이 필요하다.

### `devices/timer.c`

- `timer_sleep()`를 busy wait가 아니라 block 기반으로 바꾼다.
- 잠자는 스레드 목록을 관리한다.
- `timer_interrupt()`에서 깨울 시점이 된 스레드를 깨운다.

### `struct thread`

스레드마다 최소한 아래 정보 중 하나가 필요해진다.

- wakeup tick 값
- sleep list에 넣기 위한 list element 또는 기존 `elem` 재사용 전략

### 동기화 방식

이 자료구조는 커널 스레드와 타이머 인터럽트가 함께 접근한다.

따라서 문서 요구대로, 필요한 아주 짧은 구간만 interrupt off로 보호해야 한다. lock으로 해결하려고 하면 인터럽트 핸들러 쪽과 맞지 않는다.

## 6. 구현 완료 후 기대되는 성질

정상 구현이면 아래가 만족되어야 한다.

- `ticks <= 0`이면 바로 return
- `ticks > 0`이면 현재 스레드는 진짜로 block 됨
- 깨어날 시간 전까지는 ready list에 없어야 함
- 깨어날 시간이 되면 unblock 되어 다시 ready 상태가 됨
- 같은 시각에 깨어나는 여러 스레드는 모두 적절히 깨워짐
- 이후 스케줄링 결과는 시스템의 ready queue 정책을 따름

## 7. 지금 단계에서 주의할 점

`alarm-priority`는 이름 때문에 priority scheduling 전체를 먼저 구현해야 할 것처럼 보일 수 있다. 하지만 이 테스트는 "같이 깨어난 뒤 어떤 스레드가 먼저 실행되느냐"도 본다는 뜻에 가깝다.

즉 alarm 구현이 깨어나는 순서와 ready 상태 전환을 너무 엉성하게 처리하면 이 테스트도 깨질 수 있다.

다만 priority donation까지 필요한 단계는 아니다. 그건 이후 priority scheduling 쪽 범위다.

## 8. Alarm 테스트만 빠르게 실행하는 명령

이 저장소에는 alarm clock만 반복 실행하기 편하도록 `pintos/threads/Makefile`에 전용 타겟을 추가해 두었다.

작업 전:

```bash
cd /workspaces/pintos_22-04_lab
source pintos/activate
cd pintos/threads
```

### 전체 alarm 테스트만 실행

```bash
make alarm-check
```

이 명령은 아래 6개 테스트 결과만 모아서 보여준다.

- `alarm-single`
- `alarm-multiple`
- `alarm-simultaneous`
- `alarm-priority`
- `alarm-zero`
- `alarm-negative`

### 개별 테스트만 실행

```bash
make alarm-single
make alarm-multiple
make alarm-simultaneous
make alarm-priority
make alarm-zero
make alarm-negative
```

### 결과 파일만 정리

```bash
make alarm-clean
```

### 캐시 무시하고 다시 실행

```bash
make -B alarm-check
```

## 9. 빠른 작업 순서 제안

Alarm Clock만 먼저 끝내려면 아래 순서가 가장 단순하다.

1. `timer_sleep()`의 busy wait 제거
2. sleep list와 wakeup tick 설계
3. `timer_interrupt()`에서 wakeup 처리 추가
4. `alarm-zero`, `alarm-negative` 먼저 확인
5. `alarm-single`, `alarm-multiple`, `alarm-simultaneous` 확인
6. 마지막으로 `alarm-priority` 확인

## 10. 체크리스트

- `timer_sleep()` 안에서 busy loop가 사라졌는가
- 스레드가 sleep 중일 때 CPU를 계속 소비하지 않는가
- 인터럽트 비활성화 범위가 너무 크지 않은가
- `ticks <= 0` 처리했는가
- 같은 시각에 깨울 스레드를 빠뜨리지 않는가
- alarm 전용 테스트 6개를 반복 실행해 확인했는가
