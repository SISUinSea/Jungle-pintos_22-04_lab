# `make check` 실행 오류와 장시간 테스트 상태 확인

작성일: 2026-04-24
범위: `pintos/threads`

## 상황 요약

아직 `threads` 과제를 구현하지 않은 상태에서, 일단 빌드와 테스트가 어떻게 도는지 확인하려고 `make`, `make check`를 실행했다.

진행 중 두 가지 포인트에서 헷갈리기 쉬웠다.

- `make`는 되는데 `make check`는 실패하는 경우
- `make check` 실행 후 프롬프트가 돌아오지 않아 멈춘 것처럼 보이는 경우

## 증상 1: `make`는 되는데 `make check`는 `Error 127`로 실패

실제 로그:

```text
cd build && make check
...
pintos -v -k -T 60 -m 20 -- -q run alarm-single ...
make[1]: *** [../../tests/Make.tests:75: tests/threads/alarm-single.output] Error 127
```

## 원인

`make`와 `make check`가 하는 일이 다르다.

- `make`
  - 커널을 컴파일해서 `build/os.dsk` 같은 산출물을 만든다.
- `make check`
  - 필요한 빌드를 확인한 뒤 테스트 실행기 `pintos`를 호출해 실제 테스트를 돌린다.

즉 `make`가 성공했다는 것은 "컴파일은 됐다"는 뜻이고, `make check`가 실패했다는 것은 "테스트 실행 환경까지는 아직 준비되지 않았다"는 뜻일 수 있다.

이번 경우의 직접 원인은 `pintos` 명령을 셸이 찾지 못한 것이다. 저장소의 `pintos/activate`는 `pintos/utils`를 `PATH`에 추가한다.

## 확인 방법

저장소 루트에서 아래 명령으로 확인한다.

```bash
cd /workspaces/pintos_22-04_lab
source pintos/activate
which pintos
```

정상이라면 대략 아래처럼 나와야 한다.

```text
/workspaces/pintos_22-04_lab/pintos/utils/pintos
```

`which pintos`가 비어 있거나 다른 경로를 가리키면, 테스트 실행기가 제대로 잡히지 않은 상태다.

## 해결 방법

```bash
cd /workspaces/pintos_22-04_lab
source pintos/activate
cd pintos/threads
make check
```

필요하면 아래도 같이 본다.

```bash
echo $PATH
which pintos
ls -l /workspaces/pintos_22-04_lab/pintos/utils/pintos
```

## 증상 2: 테스트 로그가 계속 나오고 프롬프트가 안 돌아옴

실제 상황:

- 여러 `priority-*` 테스트가 `FAIL`
- `mlfqs-load-1`도 실패
- 마지막으로 아래 줄이 출력된 뒤 프롬프트가 아직 안 돌아오지 않음

```text
pintos -v -k -T 480 -m 20 -- -q -mlfqs run mlfqs-load-60 ...
```

## 원인

이건 멈춘 게 아니라 아직 테스트가 돌고 있는 상태일 가능성이 높다.

특히 `mlfqs` 테스트는 타임아웃이 길다. 위 로그의 `-T 480`은 최대 480초, 즉 8분까지 기다릴 수 있다는 뜻이다. 그래서 바로 프롬프트가 돌아오지 않아도 이상한 상황이 아닐 수 있다.

## 판별 기준

아래 둘 중 하나면 아직 실행 중일 가능성이 높다.

- 셸 프롬프트가 아직 돌아오지 않았다.
- 마지막 로그가 `pintos ... run <test-name>` 형태로 끝나 있다.

반대로 테스트가 끝나면 보통 다음 중 하나가 나온다.

- `pass ...`
- `FAIL ...`
- `All N tests passed.`
- `X of N tests failed.`
- 셸 프롬프트 복귀

## 중간에 멈추고 싶을 때

```bash
Ctrl + C
```

장시간 테스트를 중단하고 현재까지 확인한 로그만 보는 데에는 이 방법이면 충분하다.

## 이번 로그에서 읽을 수 있었던 구현 상태

현재 코드는 `threads`의 우선순위 스케줄링과 donation 관련 구현이 아직 비어 있거나 미완성일 가능성이 높다.

대표적으로 실패한 테스트:

- `priority-fifo`
- `priority-preempt`
- `priority-sema`
- `priority-condvar`
- `priority-donate-lower`
- `priority-donate-chain`

이런 실패 패턴이면 보통 아래 영역을 먼저 본다.

- `thread_unblock()`
- `thread_yield()`
- `next_thread_to_run()`
- `sema_up()`
- `cond_signal()`
- lock priority donation 관련 로직

## 짧은 정리

- `make` 성공은 컴파일 성공이다.
- `make check`는 테스트 실행까지 포함이라 `pintos` 명령이 `PATH`에 있어야 한다.
- `source pintos/activate`를 먼저 해두면 테스트 실행기가 잡힌다.
- 프롬프트가 늦게 돌아오는 것은 장시간 테스트 때문일 수 있다.
- 마지막 로그의 `-T 480` 같은 숫자를 보면 얼마나 기다릴 수 있는지 가늠할 수 있다.
