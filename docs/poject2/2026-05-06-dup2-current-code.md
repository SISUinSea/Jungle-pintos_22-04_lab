# dup2 current code guide

## 목적

이 문서는 현재 코드 기준으로 `dup2`가 왜 필요한지, 기존 `fd_table`과
무엇이 다른지 정리한다.

현재 코드에는 기본 file descriptor table은 구현되어 있다. 하지만
`dup2`가 요구하는 "여러 fd가 같은 열린 파일 상태를 공유한다"는 구조는
아직 들어가 있지 않다.

## 현재 fd_table 구조

현재 fd entry는 `pintos/include/userprog/fd.h`에 있다.

```c
struct fd_entry {
	int fd;
	struct file *file;
	struct list_elem file_elem;
};
```

각 thread는 `struct list fd_table`을 가지고 있다.

```c
struct list fd_table;
```

그리고 thread 초기화 시 `fd_table`이 초기화된다.

```c
list_init (&t->fd_table);
```

즉 현재 구조는 다음과 같다.

```text
thread
  fd_table
    fd_entry(fd=2) -> struct file *
    fd_entry(fd=3) -> struct file *
    fd_entry(fd=4) -> struct file *
```

이 구조는 `open`, `read`, `write`, `seek`, `tell`, `close`를 구현하기에는
충분하다.

## 현재 syscall 흐름

`open`은 새 `fd_entry`를 만들고, `fd_table`에서 가장 큰 fd보다 1 큰
값을 새 fd로 사용한다.

```text
open("sample.txt")
  filesys_open()
  fd_entry malloc
  fd = max_fd + 1
  fd_entry->file = file
  list_push_back(&cur->fd_table, ...)
```

`read`, `write`, `seek`, `tell`, `close`는 모두 `find_fd_entry(fd)`로
fd를 찾는다.

```text
read(fd)
  find_fd_entry(fd)
  file_read(fd_entry->file, ...)
```

```text
write(fd)
  find_fd_entry(fd)
  file_write(fd_entry->file, ...)
```

```text
close(fd)
  find_fd_entry(fd)
  list_remove()
  file_close(fd_entry->file)
  free(fd_entry)
```

여기서 중요한 점은 `close(fd)`가 곧바로 `file_close()`를 호출한다는
것이다. fd 하나가 file 하나를 단독 소유한다는 가정이다.

## dup2가 요구하는 개념

`dup2(oldfd, newfd)`는 `oldfd`가 가리키는 열린 파일 상태를 `newfd`도
가리키게 만든다.

예를 들어 다음 상황을 생각한다.

```text
fd1 = open("sample.txt")
read(fd1, ..., 10)
dup2(fd1, fd2)
read(fd2, ...)
```

`fd2`는 파일 처음부터 다시 읽으면 안 된다. `fd1`이 이미 10바이트 읽은
상태를 이어받아야 한다.

즉 `dup2` 이후에는 이렇게 되어야 한다.

```text
fd1 ┐
    ├── 같은 열린 파일 상태
fd2 ┘
```

여기서 "같은 열린 파일 상태"에는 최소한 다음이 포함된다.

- 같은 `struct file *`
- 같은 file offset
- 같은 deny write 상태
- close 시점까지 유지되는 같은 file object

따라서 `file_duplicate()`로 새 `struct file *`을 만드는 것은 `fork`에는
쓸 수 있지만, `dup2`의 공유 의미와는 다르다. `file_duplicate()`는 현재
offset을 복사한 새 file object를 만들 뿐이고, 이후 offset은 서로 따로
움직인다.

## 현재 코드에서 부족한 부분

현재 `SYS_DUP2` case는 비어 있다.

```c
case SYS_DUP2:
{

}
```

그리고 현재 `fd_entry`가 `struct file *`을 직접 들고 있기 때문에 다음
문제가 생긴다.

```text
fd1 -> file
fd2 -> 같은 file
```

이렇게 만들었다고 가정하면, `close(fd1)`에서 바로 `file_close(file)`을
호출한다. 그러면 `fd2`가 아직 같은 file을 쓰고 있어도 file object가
닫혀 버린다.

즉 현재 구조 그대로는 `dup2`를 안전하게 구현하기 어렵다.

## 필요한 구조 변화

`dup2`를 구현하려면 fd 번호와 실제 열린 file object의 책임을 분리하는
편이 좋다.

예시 구조는 다음과 같다.

```c
struct fd_entry {
	int fd;
	struct fd_file *fd_file;
	struct list_elem file_elem;
};

struct fd_file {
	struct file *file;
	int ref_cnt;
};
```

구조적으로는 이렇게 된다.

```text
thread
  fd_table
    fd_entry(fd=2) ┐
                   ├── fd_file(ref_cnt=2) -> struct file *
    fd_entry(fd=5) ┘
```

이제 `close(fd)`는 `fd_entry`만 제거하고, `ref_cnt`를 1 줄인다.
`ref_cnt`가 0이 될 때만 실제 `file_close()`를 호출한다.

```text
close(fd)
  fd_entry 제거
  fd_file->ref_cnt--
  if ref_cnt == 0:
    file_close(fd_file->file)
    free(fd_file)
  free(fd_entry)
```

## dup2 동작 규칙

현재 테스트 기준으로 `dup2`는 다음 규칙을 만족해야 한다.

### oldfd가 유효하지 않으면 실패

`oldfd`가 `fd_table`에 없으면 `-1`을 반환해야 한다.

```text
dup2(bad_fd, fd) -> -1
```

### oldfd와 newfd가 같으면 그대로 성공

`dup2(fd, fd)`는 아무것도 바꾸지 않고 `fd`를 반환해야 한다.

```text
dup2(fd3, fd3) -> fd3
```

`dup2-complex.c`에는 이 형태가 직접 나온다.

```c
dup2 (dup2 (fd3, fd3), dup2 (fd1, fd2));
```

### newfd가 이미 열려 있으면 먼저 닫는다

`dup2(oldfd, newfd)`에서 `newfd`가 이미 사용 중이면, 먼저 `close(newfd)`
와 같은 효과가 나야 한다.

그 다음 `newfd`가 `oldfd`와 같은 열린 file object를 가리키게 한다.

```text
before:
  oldfd -> A
  newfd -> B

dup2(oldfd, newfd)

after:
  oldfd -> A
  newfd -> A
  B는 ref_cnt가 0이면 close
```

### newfd가 크거나 비어 있어도 지정한 번호를 사용한다

현재 `open`은 `max_fd + 1` 방식으로 fd를 만든다. 하지만 `dup2`는
`newfd`로 들어온 값을 그대로 사용해야 한다.

`dup2-simple.c`는 일부러 큰 fd를 사용한다.

```c
int fd2 = 0x1CE;
CHECK (dup2 (fd1, fd2) > 1, "first dup2()");
```

따라서 `dup2`는 비어 있는 fd 중 가장 작은 값을 고르는 syscall이 아니다.
사용자가 지정한 `newfd`를 만드는 syscall이다.

## stdout과 stdin

테스트에서는 `dup2`를 stdout에도 사용한다.

```c
dup2 (1, fd5);
write (fd5, magic, sizeof magic - 1);
```

또 다른 부분에서는 stdout 자체를 file로 바꾼다.

```c
dup2 (fd6, 1);
msg ("%d", byte_cnt);
```

따라서 `dup2` 구현은 fd 0, fd 1도 특별히 조심해야 한다.

현재 코드는 fd 0과 fd 1을 `fd_table`에 기본 entry로 넣지 않는다.
대신 syscall에서 직접 처리한다.

```text
read(0)  -> input_getc()
write(1) -> putbuf()
```

그러나 `dup2(fd6, 1)` 이후에는 fd 1이 더 이상 콘솔 stdout이 아니라
파일을 가리킬 수 있어야 한다.

즉 `dup2`를 제대로 구현하려면 다음 정책 중 하나가 필요하다.

```text
방법 A:
  fd_table에 fd 0, fd 1 entry도 넣고 stdin/stdout을 일반 fd처럼 다룬다.

방법 B:
  fd_table에 fd 1 entry가 있으면 write(1)는 그 entry를 우선 사용하고,
  없을 때만 putbuf()를 사용한다.
```

현재 코드와 가장 적게 충돌하는 방향은 B에 가깝다.

## fork와 dup2의 차이

현재 fork에서는 부모의 fd entry를 돌면서 `file_duplicate()`를 사용한다.

```c
new_fde->file = file_duplicate (fde->file);
```

이것은 부모와 자식이 서로 다른 `struct file *`을 가지게 한다.
따라서 fork 이후 부모와 자식의 file offset은 독립적으로 움직인다.

반면 dup2는 같은 process 안에서 두 fd가 같은 열린 file object를 공유해야
한다.

```text
fork fd copy:
  parent fd3 -> file A
  child  fd3 -> file B

dup2:
  fd3 -> file A
  fd5 -> file A
```

그래서 `dup2` 구현에 `file_duplicate()`를 그대로 쓰면 `dup2-simple`의
offset 공유 요구를 만족하지 못한다.

## 구현 순서 추천

현재 코드에서 구현한다면 순서는 다음이 좋다.

1. `struct fd_entry`가 직접 `struct file *`을 들지 않도록 공유 구조를
   추가한다.
2. `open`에서 새 공유 구조를 만들고 `ref_cnt = 1`로 시작한다.
3. `read`, `write`, `seek`, `tell`, `filesize`가 공유 구조 안의 file을
   사용하도록 바꾼다.
4. `close`를 helper로 분리하고, ref count가 0일 때만 `file_close()`를
   호출하게 한다.
5. process exit의 fd cleanup도 같은 close helper를 사용하게 한다.
6. `dup2`에서 `oldfd`를 찾고, 필요하면 `newfd`를 먼저 닫은 뒤 새
   `fd_entry`를 만들어 같은 공유 구조를 가리키게 한다.
7. `newfd == oldfd`는 바로 `newfd`를 반환한다.
8. fd 1이 fd_table에 존재하는 경우 `write(1)`이 putbuf가 아니라 그 file에
   쓰도록 처리한다.

## 확인할 테스트

최소 확인 대상은 다음이다.

```text
tests/userprog/dup2/dup2-simple
tests/userprog/dup2/dup2-complex
```

회귀 확인으로는 기존 fd syscall 테스트도 같이 보는 것이 좋다.

```text
read-normal
write-normal
seek-simple
tell-simple
close-normal
close-twice
read-stdout
write-stdin
```

## 한 줄 결론

현재 fd_table은 구현되어 있지만, fd 하나가 file 하나를 단독 소유하는
구조다. `dup2`는 여러 fd가 같은 열린 file object와 offset을 공유해야
하므로, `fd_entry`와 실제 file object 사이에 ref count가 있는 공유
구조가 필요하다.
