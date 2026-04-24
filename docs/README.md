# Docs

이 디렉터리는 Pintos 실습 중 남기는 문서들을 모아두는 곳이다.

## 구조

```text
docs/
├── README.md
├── project1/
│   └── alarm-clock.md
└── troubleshooting/
    ├── threads/
    ├── userprog/
    ├── vm/
    └── shared/
```

## 규칙

- 시행착오, 에러 원인 분석, 해결 과정은 `troubleshooting/` 아래에 둔다.
- 구현 대상 분석, 테스트 목록, 작업 계획은 `project1/`, `project2/` 같은 프로젝트별 디렉터리에 둔다.
- 문서는 영향 범위에 따라 `threads`, `userprog`, `vm`, `shared` 중 하나에 둔다.
- 파일명은 `YYYY-MM-DD-주제.md` 형식으로 만든다.
- 한 문서는 "증상 -> 원인 -> 확인 방법 -> 해결 방법" 흐름으로 짧고 재사용 가능하게 쓴다.
- 같은 문제를 다시 겪을 가능성이 높으면 실행 명령과 판별 기준을 함께 적는다.

## 현재 문서

- [project1: Alarm Clock 요구사항, 테스트, 실행 명령 정리](./project1/alarm-clock.md)
- [threads: `make check` 실행 오류와 장시간 테스트 상태 확인](./troubleshooting/threads/2026-04-24-make-check-path-and-long-running-tests.md)
