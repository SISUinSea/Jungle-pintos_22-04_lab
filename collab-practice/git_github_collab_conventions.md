# Git/GitHub 협업 컨벤션 정리

이 문서는 팀원들과 Git/GitHub 협업을 연습할 때 사용할 규칙과 템플릿을 정리한 문서입니다.

목표는 복잡한 규칙을 외우는 것이 아니라, 아래 흐름을 익히는 것입니다.

```text
issue → branch → commit → push → PR → review → approve → merge
```

---

## 1. 기본 협업 규칙

1. `main` 브랜치에 직접 push하지 않는다.
2. 모든 작업은 issue에서 시작한다.
3. issue 번호를 포함해서 branch를 만든다.
4. 작업은 반드시 별도 branch에서 진행한다.
5. 작업이 끝나면 Pull Request를 만든다.
6. 최소 1명 이상의 review/approve를 받고 merge한다.
7. PR이 merge되면 관련 issue가 닫히는지 확인한다.
8. merge 후에는 각자 `main` 브랜치를 최신화한다.

---

## 2. 작업 흐름

### 1단계: issue 생성

예시 제목:

```text
[DOCS] 해건 자기소개 추가
[NOTE] 해건 학습 메모 추가
[DOCS] README 제목 수정
[CHORE] 협업 연습 폴더 추가
```

issue 내용 예시:

```md
## 작업 내용

- `collab-practice/team.md`에 자기소개를 추가합니다.

## 완료 조건

- 이름이 추가되어 있다.
- 관심사가 추가되어 있다.
- 한 줄 소개가 추가되어 있다.
- `pintos/` 폴더는 수정하지 않는다.
```

---

### 2단계: branch 생성

항상 최신 `main`에서 새 branch를 만든다.

```bash
git checkout main
git pull origin main
git checkout -b issue-2-add-haegeon-profile
```

브랜치 이름 규칙:

```text
issue-이슈번호-작업내용
```

예시:

```text
issue-1-init-collab-practice
issue-2-add-haegeon-profile
issue-3-add-minsu-profile
issue-4-add-jiyoung-profile
issue-5-add-collaboration-note
issue-10-update-readme-title
```

---

### 3단계: 작업 후 commit

변경 확인:

```bash
git status
git diff
```

커밋:

```bash
git add 파일명
git commit -m "docs: add haegeon profile"
```

---

### 4단계: push

```bash
git push origin 브랜치명
```

예시:

```bash
git push origin issue-2-add-haegeon-profile
```

---

### 5단계: PR 생성

GitHub에서 `Compare & pull request` 버튼을 눌러 PR을 만든다.

base와 compare는 보통 이렇게 둔다.

```text
base: main
compare: 내가 작업한 브랜치
```

---

### 6단계: review 요청

PR 오른쪽에서 reviewer를 지정한다.

예시:

```text
해건 PR → 민수 리뷰
민수 PR → 지영 리뷰
지영 PR → 수빈 리뷰
수빈 PR → 해건 리뷰
```

---

### 7단계: review 반영

리뷰어가 수정 요청을 남기면, 같은 branch에서 수정한다.

```bash
git add 파일명
git commit -m "fix: apply review feedback"
git push origin 브랜치명
```

기존 PR이 자동으로 업데이트된다. PR을 새로 만들 필요 없다.

---

### 8단계: approve 후 merge

리뷰어가 `Approve`하면 merge한다.

실습에서는 보통 아래 방식을 추천한다.

```text
Squash and merge
```

merge 후에는 로컬 main을 최신화한다.

```bash
git checkout main
git pull origin main
```

작업 branch는 삭제해도 된다.

```bash
git branch -d 브랜치명
```

---

## 3. Commit Message Convention

커밋 메시지는 아래 형식을 사용한다.

```text
타입: 작업 내용
```

예시:

```text
docs: add haegeon profile
fix: apply review feedback
chore: add collab practice folder
feat: add profile card layout
refactor: reorganize practice notes
```

---

## 4. Commit Type 정리

| 타입 | 의미 | 예시 |
|---|---|---|
| `docs` | 문서 수정 | `docs: add haegeon profile` |
| `fix` | 잘못된 내용 수정 | `fix: correct typo in team profile` |
| `chore` | 설정, 폴더 생성, 기타 작업 | `chore: add collab practice folder` |
| `feat` | 기능 추가 | `feat: add profile card layout` |
| `refactor` | 동작은 같지만 구조 개선 | `refactor: reorganize notes section` |
| `style` | 코드/문서 포맷 변경 | `style: format team markdown` |
| `test` | 테스트 추가/수정 | `test: add sample validation note` |

---

## 5. 좋은 커밋 메시지 예시

```text
chore: add collab practice folder
docs: add haegeon profile
docs: add collaboration note
fix: update profile description
fix: resolve readme title conflict
```

---

## 6. 나쁜 커밋 메시지 예시

```text
수정
작업함
asdf
update
fix
진짜최종
```

이런 메시지는 나중에 히스토리를 봤을 때 무엇을 바꿨는지 알기 어렵다.

---

## 7. PR Title Convention

PR 제목도 커밋 메시지와 비슷하게 쓴다.

```text
타입: 작업 내용
```

예시:

```text
chore: add collab practice folder
docs: add haegeon profile
docs: add collaboration note
fix: resolve readme title conflict
```

---

## 8. PR Description Convention

PR 본문에는 최소한 아래 내용을 적는다.

1. 변경 사항
2. 테스트 방법
3. 리뷰어가 확인할 것
4. 관련 이슈

---

## 9. PR 템플릿

아래 내용을 복사해서 PR 본문에 사용한다.

```md
## 변경 사항

- 

## 테스트 방법

1. 
2. 
3. `pintos/` 폴더가 수정되지 않았는지 확인합니다.

## 리뷰어가 확인할 것

- [ ] 변경 범위가 의도한 폴더 안으로 제한되어 있는가?
- [ ] 작업 내용이 issue와 맞는가?
- [ ] 문서 내용이 읽기 좋은가?
- [ ] 오타나 이상한 표현이 없는가?

## 관련 이슈

Closes #
```

---

## 10. PR 작성 예시

```md
## 변경 사항

- `collab-practice/team.md`에 해건 자기소개를 추가했습니다.

## 테스트 방법

1. `collab-practice/team.md` 파일을 엽니다.
2. 해건 자기소개가 정상적으로 추가되었는지 확인합니다.
3. `pintos/` 폴더가 수정되지 않았는지 확인합니다.

## 리뷰어가 확인할 것

- [ ] 자기소개 형식이 다른 팀원들과 일관적인가?
- [ ] 오타가 없는가?
- [ ] 변경 범위가 `collab-practice/` 안으로 제한되어 있는가?

## 관련 이슈

Closes #2
```

---

## 11. Issue 템플릿

```md
## 작업 내용

- 

## 완료 조건

- [ ] 
- [ ] 
- [ ] `pintos/` 폴더를 수정하지 않는다.

## 참고 사항

- 
```

---

## 12. Issue 작성 예시

```md
## 작업 내용

- `collab-practice/notes/haegeon.md` 파일을 추가합니다.
- GitHub 협업 실습에서 배운 점과 헷갈리는 점을 정리합니다.

## 완료 조건

- [ ] 개인 메모 파일이 추가되어 있다.
- [ ] 배운 점이 1개 이상 작성되어 있다.
- [ ] 헷갈리는 점이 1개 이상 작성되어 있다.
- [ ] `pintos/` 폴더를 수정하지 않는다.

## 참고 사항

- 파일 경로: `collab-practice/notes/haegeon.md`
```

---

## 13. Review Convention

리뷰는 단순히 “좋아요”만 남기는 것이 아니라, 변경사항을 확인하고 피드백을 주는 과정이다.

리뷰할 때는 가능하면 아래 세 가지 중 하나 이상을 남긴다.

1. 좋은 점
2. 궁금한 점
3. 수정 제안

---

## 14. 좋은 리뷰 예시

```text
자기소개 형식이 다른 팀원들과 맞아서 좋습니다.
다만 "이번 주 목표"를 조금 더 구체적으로 적으면 나중에 보기 좋을 것 같아요.
```

```text
pintos/ 폴더가 수정되지 않은 점 확인했습니다.
문서 변경만 있어서 merge해도 괜찮아 보입니다.
```

```text
README 제목 변경 의도는 좋습니다.
다만 기존 제목과 새 제목의 의미를 합쳐서 "GitHub Collaboration & PR Review Practice"로 바꾸는 건 어떨까요?
```

---

## 15. 나쁜 리뷰 예시

```text
별로임
```

```text
다시 하세요
```

```text
뭔가 이상함
```

문제가 있다면 무엇이 문제인지, 왜 문제인지, 어떻게 바꾸면 좋을지 함께 적는다.

---

## 16. Review Checklist

리뷰어는 아래 항목을 확인한다.

```md
## 리뷰 체크리스트

- [ ] `pintos/` 폴더를 수정하지 않았는가?
- [ ] 작업 내용이 issue와 맞는가?
- [ ] PR 설명이 충분한가?
- [ ] 변경 파일이 너무 많지 않은가?
- [ ] 오타나 이상한 문장이 없는가?
- [ ] merge해도 main이 망가지지 않는가?
```

---

## 17. Merge 전 확인할 것

merge하기 전에 아래를 확인한다.

```md
## Merge 전 체크리스트

- [ ] 최소 1명 이상 approve 했는가?
- [ ] unresolved conversation이 없는가?
- [ ] conflict가 없는가?
- [ ] 관련 issue가 연결되어 있는가?
- [ ] `pintos/` 폴더가 실수로 수정되지 않았는가?
```

---

## 18. Issue 자동 닫기 키워드

PR 본문에 아래처럼 적으면, PR이 merge될 때 issue가 자동으로 닫힌다.

```md
Closes #2
```

사용 가능한 표현 예시:

```text
Closes #2
Fixes #2
Resolves #2
```

이번 실습에서는 통일해서 아래 표현을 사용한다.

```md
Closes #이슈번호
```

---

## 19. Conflict 해결 기본 흐름

main의 최신 변경을 내 branch에 합치다가 conflict가 날 수 있다.

```bash
git checkout main
git pull origin main
git checkout 내브랜치명
git merge main
```

충돌이 나면 파일 안에 이런 표시가 생긴다.

```text
<<<<<<< HEAD
내 브랜치의 내용
=======
main 브랜치의 내용
>>>>>>> main
```

원하는 최종 내용만 남기고 conflict 표시를 지운다.

그다음:

```bash
git add 충돌해결한파일
git commit -m "fix: resolve conflict"
git push origin 내브랜치명
```

---

## 20. 자주 쓰는 명령어 모음

### 현재 상태 확인

```bash
git status
```

### 변경 내용 확인

```bash
git diff
```

### main 최신화

```bash
git checkout main
git pull origin main
```

### branch 생성

```bash
git checkout -b issue-번호-작업내용
```

### commit

```bash
git add 파일명
git commit -m "docs: add profile"
```

### push

```bash
git push origin 브랜치명
```

### branch 목록 확인

```bash
git branch
```

### 로컬 branch 삭제

```bash
git branch -d 브랜치명
```

---

## 21. 이번 실습에서 사용할 최종 규칙 요약

```text
1. main에 직접 push하지 않는다.
2. issue를 먼저 만든다.
3. issue 번호가 들어간 branch를 만든다.
4. 작업 후 의미 있는 commit message를 작성한다.
5. PR을 만들고 관련 issue를 연결한다.
6. reviewer를 지정한다.
7. 최소 1명 approve 후 merge한다.
8. merge 후 모두 main을 pull한다.
9. pintos/ 폴더는 건드리지 않는다.
```

---

## 22. 한 줄 요약

```text
issue로 작업을 정의하고, branch에서 작업하고, PR로 공유하고, review를 받은 뒤 main에 merge한다.
```
