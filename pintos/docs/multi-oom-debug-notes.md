# multi-oom 디버깅 복기 노트

## 테스트 목적

`tests/userprog/no-vm/multi-oom.c`는 `fork()`를 반복해서 커널 자원이 부족해지는 상황을 만든다.

핵심 확인점:

- 더 이상 자식 생성이 불가능하면 `fork()`는 `-1`을 반환해야 한다.
- 비정상 종료한 자식에 대해 `wait()`는 `-1`을 반환해야 한다.
- 비정상 종료 중 열린 파일, fd entry, page table 등 커널 자원이 누수되면 안 된다.
- 같은 테스트를 여러 번 반복해도 fork 가능한 깊이가 줄어들면 안 된다.

## 코드 이해

`main()`은 먼저 `make_children()`을 한 번 실행해 기준 depth를 얻고, 이후 10번 더 반복한다.

`make_children()`은 사실상 무한 루프다.

```c
for (; ; random_init (i), i++)
```

조건식이 없으므로 `break` 또는 `exit(i)`로만 끝난다.

`fork()` 결과별 의미:

- `pid < 0`: 더 이상 fork 실패. 현재 깊이 `i`를 종료 코드로 반환한다.
- `pid == 0`: 자식. 자원을 소비하고 다음 루프를 계속 돈다.
- `pid > 0`: 부모. 루프를 `break`하고 자식을 `wait(pid)` 한다.

따라서 여러 프로세스가 체인처럼 이어지고, 각 부모는 바로 아래 자식을 기다린다.

## consume_some_resources

`consume_some_resources()`는 최대 126번 `open(test_name)`을 호출한다.

여기서 `test_name`은 `"multi-oom"`이고, 테스트 실행 파일 자체를 여는 것이다.

목적은 파일 내용을 쓰는 것이 아니라 열린 파일 디스크립터와 관련 커널 자원을 많이 잡아두는 것이다.

`open()`은 파일이 없으면 새로 만들지 않는다. 파일 생성은 `create()`가 담당한다.

## 비정상 종료 자식

`i > EXPECTED_DEPTH_TO_PASS / 2`, 즉 `i == 6`부터는 추가로 비정상 종료용 자식을 만든다.

그 자식은:

- 파일을 많이 열고
- NULL 포인터 접근
- 커널 주소 접근
- 잘못된 포인터를 `open()`에 전달

같은 방식으로 일부러 죽는다.

목적은 비정상 종료 경로에서도 자원 정리가 되는지 확인하는 것이다.

## random_init(i)

`random_init(i)`는 `i` 값을 바꾸지 않는다.

난수 생성기의 seed를 `i`로 고정한다.

그래서 같은 `i`에서는 `random_ulong()` 결과가 재현 가능하다. 테스트를 여러 번 돌려도 비정상 종료 패턴이 일정하게 나오는 이유가 된다.

## wait 관련 이해

`wait-simple`은 비정상 종료 테스트가 아니다.

`child-simple`이 `main()`에서 `return 81;`만 해도 `_start()` 흐름에서 결국 `exit(81)` 시스템콜이 호출된다.

따라서 부모의 `wait(pid)`는 `81`을 받아야 한다.

`wait-twice`는 같은 자식을 두 번 기다릴 때 두 번째 `wait()`가 즉시 `-1`을 반환해야 함을 확인한다.

## ffs 가설

작업 중 세운 가설 이름:

`ffs`: false fork success

의미:

> 실제로는 fork가 실패했는데 부모에게 양수 pid가 반환되어, 부모가 존재하지 않거나 완료되지 못한 자식을 기다리며 timeout에 빠지는 상황

이 가설을 검증할 때 핵심 질문은 다음이었다.

- `fork()`가 양수 pid를 반환했다면 `struct thread`는 어디까지 생성되었는가?
- 부모의 `wait(pid)`는 실제 thread를 기다리는가, child metadata를 기다리는가?
- 자식 스레드가 아예 실행되지 않았는가, 아니면 실행됐지만 부모를 깨우기 전에 죽었는가?
- 실패 경로에서 부모에게 실패 상태를 남기고 `sema_up()` 하는가?

## 해결된 timeout 원인

timeout 문제는 `__do_fork()`의 `goto error` 경로에서 예외 처리를 빠뜨린 것이 원인이었다.

자식 fork 처리 중 실패했을 때 부모가 `sema_down()`으로 기다리고 있다면, 실패 경로에서도 반드시:

- child status에 실패 상태를 남기고
- 부모를 깨우고
- 부분 생성된 자원을 정리하고
- 자식 스레드를 종료해야 한다.

이 처리를 넣은 뒤 timeout이 사라졌다면, `ffs` 가설의 핵심 문제는 해결된 것으로 볼 수 있다.

## 다음에 볼 것

timeout이 사라진 뒤에는 남은 실패가 무엇인지 로그를 기준으로 다시 좁혀야 한다.

우선순위:

- `multi-oom.output`
- `multi-oom.errors`
- `process_fork()` 실패 반환 경로
- `__do_fork()` 실패 정리 경로
- fd table 복제 실패 시 이미 복제한 fd entry 정리 여부
- page table 복제 실패 시 이미 할당한 user page 정리 여부
