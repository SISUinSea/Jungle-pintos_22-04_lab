# Pintos Code Convention

이 문서는 Pintos를 팀으로 구현하면서 지킬 코드 컨벤션을 정리한 문서입니다.

핵심 원칙은 **기존 Pintos 코드 스타일을 최대한 따라가면서, 팀원이 읽고 리뷰하기 쉬운 코드를 작성하는 것**입니다.

---

## 1. 기본 원칙

- 기존 Pintos 코드 스타일을 최우선으로 따른다.
- 개인 취향의 포맷팅을 섞지 않는다.
- Pintos 실제 코드 수정은 필요한 범위만 최소화한다.
- 한 PR에는 하나의 논리적 변경만 담는다.
- 리뷰어가 변경 의도를 쉽게 이해할 수 있도록 코드를 작게 나누고 설명한다.

---

## 2. 최우선 기준: 기존 Pintos 코드 스타일

Pintos 코드는 이미 나름의 스타일을 가지고 있다.  
따라서 새로운 스타일을 만들기보다, **주변 코드와 비슷하게 쓰는 것**을 우선한다.

예시:

```c
static bool
is_valid_fd (int fd)
{
  return fd >= 0 && fd < FD_MAX;
}
```

Pintos 코드에서 자주 보이는 특징:

- 함수명과 변수명은 `snake_case`를 사용한다.
- 함수 호출 시 함수명과 괄호 사이에 공백을 둔다.

```c
func (arg);
```

- `if`, `while`, `for` 뒤에도 공백을 둔다.

```c
if (condition)
  return;
```

- 기존 파일의 들여쓰기와 중괄호 스타일을 따른다.
- 긴 줄은 가능하면 79자 이하로 줄인다.
- 주변 코드와 다른 포맷을 섞지 않는다.

---

## 3. 네이밍 규칙

### 3.1 함수명과 변수명

- 함수명과 변수명은 `snake_case`를 사용한다.
- 의미 없는 축약어를 피한다.
- 너무 짧은 이름을 남발하지 않는다.
- 반복문 인덱스처럼 범위가 아주 짧은 경우에는 `i`, `j` 정도는 허용한다.

좋지 않은 예:

```c
struct file *f;
int n;
void *b;
```

더 나은 예:

```c
struct file *file;
int size;
void *buffer;
```

### 3.2 bool 함수 이름

`bool`을 반환하는 함수는 가능하면 의미가 드러나게 작성한다.

예시:

```c
static bool is_valid_fd (int fd);
static bool has_child_process (tid_t tid);
static bool can_access_user_memory (const void *uaddr);
```

### 3.3 helper 함수 이름

helper 함수는 역할이 드러나야 한다.

예시:

```c
static struct file *get_file_by_fd (int fd);
static void close_all_files (struct thread *t);
static bool validate_user_buffer (const void *buffer, unsigned size);
```

---

## 4. 함수 작성 규칙

### 4.1 함수 하나는 한 가지 책임만 가진다

나쁜 방향:

```c
void
syscall_handler (struct intr_frame *f)
{
  /* syscall 번호 읽기, 인자 검증, fd 처리, 파일 처리,
     read/write/exit 로직을 모두 한 함수에서 처리 */
}
```

좋은 방향:

```c
static void
handle_read (struct intr_frame *f)
{
  /* SYS_READ 처리 */
}

static void
handle_write (struct intr_frame *f)
{
  /* SYS_WRITE 처리 */
}

void
syscall_handler (struct intr_frame *f)
{
  switch (syscall_number)
    {
    case SYS_READ:
      handle_read (f);
      break;
    case SYS_WRITE:
      handle_write (f);
      break;
    }
}
```

### 4.2 큰 함수는 helper 함수로 분리한다

다음과 같은 로직은 별도 함수로 빼는 것을 고려한다.

- user pointer 검증
- file descriptor 조회
- process wait 처리
- stack setup
- supplemental page table 탐색
- frame eviction
- lock 획득/해제 패턴이 복잡한 로직

### 4.3 내부에서만 쓰는 함수는 `static`으로 선언한다

파일 내부에서만 사용하는 함수는 외부에 노출하지 않는다.

```c
static bool
validate_user_pointer (const void *uaddr)
{
  ...
}
```

---

## 5. 포맷팅 규칙

- 기존 Pintos 파일의 포맷을 따른다.
- C 소스 파일은 한 줄 79자 이하를 권장한다.
- 불필요한 공백 변경만 있는 commit은 만들지 않는다.
- 자동 포맷터를 전체 파일에 무작정 적용하지 않는다.
- 기존 코드 전체를 재포맷하지 않는다.

### 5.1 공백만 바꾸는 수정 금지

나쁜 예:

```text
작업 내용: syscall 구현
실제 변경: 파일 전체 들여쓰기 변경 + syscall 구현
```

이렇게 하면 리뷰어가 진짜 변경 사항을 보기 어렵다.

좋은 예:

```text
작업 내용: syscall 구현
실제 변경: syscall 구현에 필요한 부분만 수정
```

---

## 6. 주석 규칙

주석은 코드가 **무엇을 하는지**보다, **왜 그렇게 하는지**를 설명해야 한다.

### 6.1 좋지 않은 주석

```c
/* fd를 증가시킨다. */
fd++;
```

코드를 보면 바로 알 수 있는 내용이므로 필요 없는 주석이다.

### 6.2 좋은 주석

```c
/* File descriptors 0 and 1 are reserved for stdin and stdout. */
next_fd = 2;
```

```c
/* The user buffer may cross a page boundary, so each byte must be
   validated before copying. */
```

이런 주석은 코드만 봐서는 알기 어려운 정책과 이유를 설명한다.

### 6.3 주석을 남기면 좋은 부분

- user pointer 검증
- user buffer가 page boundary를 넘을 수 있는 경우
- lock을 잡는 이유
- lock 획득 순서
- race condition 방지 로직
- process wait/exit의 동기화 로직
- file descriptor table 관리 정책
- page fault 처리 정책
- eviction 대상 선택 기준
- 실패 시 자원 해제 로직

---

## 7. Doxygen 사용 기준

Doxygen은 필수 표준이 아니라, **복잡한 helper 함수에 선택적으로 사용하는 함수 주석 스타일**로 삼는다.

즉, 모든 함수에 Doxygen 주석을 강제하지 않는다.

### 7.1 Doxygen 스타일을 사용하면 좋은 경우

- 함수의 전제 조건이 있는 경우
- 반환값의 의미가 중요한 경우
- 부작용이 있는 경우
- caller가 지켜야 할 규칙이 있는 경우
- 동시성이나 lock과 관련된 주의사항이 있는 경우

예시:

```c
/**
 * Reads SIZE bytes from FD into BUFFER.
 *
 * Returns the number of bytes read, or -1 on failure.
 * The caller must pass a valid user buffer.
 */
static int
sys_read (int fd, void *buffer, unsigned size)
{
  ...
}
```

### 7.2 Doxygen 주석이 굳이 필요 없는 경우

너무 단순한 함수에는 달지 않아도 된다.

```c
static bool
is_stdin (int fd)
{
  return fd == 0;
}
```

---

## 8. 에러 처리 규칙

Pintos에서는 실패 경로 처리가 매우 중요하다.

- 실패 가능성이 있는 함수 호출은 반환값을 확인한다.
- 잘못된 user pointer 접근 가능성을 항상 고려한다.
- lock을 잡은 뒤 return하는 경우 unlock 누락을 조심한다.
- 실패 경로에서 할당한 자원을 해제한다.
- file을 열었다면 실패 시 닫아야 하는지 확인한다.
- page/frame을 할당했다면 실패 시 해제해야 하는지 확인한다.

예시 체크 포인트:

```text
lock_acquire 이후 모든 return 경로에서 lock_release가 호출되는가?
malloc 이후 실패 경로에서 free가 호출되는가?
file_open 이후 실패 경로에서 file_close가 호출되는가?
user pointer를 역참조하기 전에 검증했는가?
```

---

## 9. 동시성 규칙

Pintos는 thread, file system, process wait/exit 등에서 동시성 문제가 쉽게 생긴다.

- 공유 자료구조 접근 시 lock 필요 여부를 먼저 판단한다.
- lock을 잡는 순서를 팀 내에서 통일한다.
- lock을 잡은 상태에서 오래 걸리는 작업을 하지 않는다.
- lock을 잡은 상태에서 sleep 가능성이 있는 함수를 호출하는지 주의한다.
- interrupt context에서 하면 안 되는 작업을 구분한다.
- race condition이 생길 수 있는 로직에는 주석을 남긴다.

### 9.1 lock 사용 시 확인할 것

```text
이 자료구조는 여러 thread가 동시에 접근할 수 있는가?
읽기만 해도 lock이 필요한가?
lock을 잡고 return하는 경로가 있는가?
중첩 lock이 필요한가?
lock 획득 순서가 꼬일 가능성은 없는가?
```

---

## 10. PR 단위 규칙

한 PR에는 하나의 논리적 변경만 담는다.

좋은 PR 예시:

```text
- fd table 구조 추가
- SYS_OPEN 구현
- SYS_READ 구현
- user pointer validation 추가
```

좋지 않은 PR 예시:

```text
- fd table 추가
- read/write 구현
- wait 구현
- README 수정
- 주석 정리
- 포맷팅 변경
```

PR이 커지면 리뷰가 어려워지고, 버그가 들어갔을 때 추적하기 어렵다.

---

## 11. 코드 리뷰 시 확인할 것

리뷰어는 단순히 코드가 돌아가는지만 보지 말고, 다음을 확인한다.

- 변경 범위가 issue와 맞는가?
- 기존 Pintos 코드 스타일을 따르는가?
- 함수가 너무 길지 않은가?
- helper 함수로 분리할 수 있는가?
- 변수명이 의미를 잘 드러내는가?
- user pointer 검증이 빠지지 않았는가?
- lock/unlock 짝이 맞는가?
- 실패 경로에서 자원 해제가 되는가?
- 불필요한 파일이나 공백 변경이 섞이지 않았는가?
- 테스트 결과가 PR에 적혀 있는가?

---

## 12. PR 전 체크리스트

PR을 올리기 전에 작성자는 아래 항목을 확인한다.

```md
## Pintos PR 체크리스트

- [ ] 수정 범위가 issue와 맞는가?
- [ ] `pintos/` 안에서 필요한 파일만 수정했는가?
- [ ] 불필요한 공백 변경이나 전체 포맷팅이 섞이지 않았는가?
- [ ] 기존 Pintos 코드 스타일을 따랐는가?
- [ ] 함수가 너무 길어지지 않았는가?
- [ ] 복잡한 로직은 helper 함수로 분리했는가?
- [ ] user pointer 검증이 필요한 곳에서 빠지지 않았는가?
- [ ] lock/unlock 짝이 맞는가?
- [ ] 실패 경로에서 자원을 해제하는가?
- [ ] 주석이 필요한 복잡한 로직에 설명을 달았는가?
- [ ] 관련 테스트를 실행했는가?
- [ ] 테스트 결과를 PR 본문에 적었는가?
```

---

## 13. 추천 함수 주석 템플릿

복잡한 함수에만 선택적으로 사용한다.

```c
/**
 * Briefly describes what this function does.
 *
 * Returns ... on success, or ... on failure.
 * The caller must ...
 * This function assumes ...
 */
static int
function_name (int arg)
{
  ...
}
```

예시:

```c
/**
 * Validates that the user buffer [BUFFER, BUFFER + SIZE) is accessible.
 *
 * Returns true if every byte in the range is valid user memory.
 * Terminates the current process if an invalid address is found.
 */
static bool
validate_user_buffer (const void *buffer, unsigned size)
{
  ...
}
```

---

## 14. 최종 기준 요약

팀의 Pintos 코드 컨벤션 우선순위는 다음과 같다.

```text
1순위: 기존 Pintos 코드 스타일
2순위: GNU C 스타일 느낌 유지
3순위: 함수 분리, 변수명, 주석으로 가독성 확보
4순위: 복잡한 함수에만 Doxygen식 주석 선택 적용
```

Doxygen을 팀의 핵심 코드 컨벤션으로 삼기보다는, 다음 정도로 사용하는 것이 적절하다.

```text
복잡한 helper 함수에는 Doxygen 느낌의 함수 주석을 달자.
단순한 함수에는 형식적인 주석을 강제하지 말자.
```

---

## 15. 한 줄 결론

Pintos에서는 멋진 새 스타일보다 **기존 코드와 자연스럽게 이어지는 코드**, 그리고 팀원이 리뷰하기 쉬운 **작고 명확한 코드**가 가장 중요하다.
