include ../Makefile.kernel

ALARM_TESTS = tests/threads/alarm-single \
	tests/threads/alarm-multiple \
	tests/threads/alarm-simultaneous \
	tests/threads/alarm-priority \
	tests/threads/alarm-zero \
	tests/threads/alarm-negative

ALARM_RESULTS = $(addprefix build/,$(addsuffix .result,$(ALARM_TESTS)))

SYNC_TESTS = tests/threads/priority-sema \
	tests/threads/priority-condvar \
	tests/threads/priority-donate-sema

SYNC_RESULTS = $(addprefix build/,$(addsuffix .result,$(SYNC_TESTS)))

.PHONY: alarm alarm-tests alarm-check alarm-clean \
	alarm-single alarm-multiple alarm-simultaneous \
	alarm-priority alarm-zero alarm-negative \
	Sync sync sync-tests sync-check sync-clean \
	priority-sema priority-condvar priority-donate-sema

alarm:
	$(MAKE) alarm-clean
	$(MAKE) alarm-check

# Build only the result files for the alarm clock tests.
alarm-tests: $(ALARM_RESULTS)

# Print PASS/FAIL output for the alarm clock tests only and fail if any test fails.
alarm-check: alarm-tests
	@failures=0; count=0; \
	for test in $(ALARM_TESTS); do \
		count=$$((count + 1)); \
		if echo PASS | cmp -s build/$$test.result -; then \
			echo "pass $$test"; \
		else \
			echo "FAIL $$test"; \
			failures=$$((failures + 1)); \
		fi; \
	done; \
	if [ $$failures -eq 0 ]; then \
		echo "All $$count alarm tests passed."; \
	else \
		echo "$$failures of $$count alarm tests failed."; \
		exit 1; \
	fi

# Remove only the generated artifacts for the alarm clock tests.
alarm-clean:
	rm -f $(addprefix build/,$(addsuffix .result,$(ALARM_TESTS)))
	rm -f $(addprefix build/,$(addsuffix .output,$(ALARM_TESTS)))
	rm -f $(addprefix build/,$(addsuffix .errors,$(ALARM_TESTS)))

alarm-single:
	$(MAKE) build/tests/threads/alarm-single.result

alarm-multiple:
	$(MAKE) build/tests/threads/alarm-multiple.result

alarm-simultaneous:
	$(MAKE) build/tests/threads/alarm-simultaneous.result

alarm-priority:
	$(MAKE) build/tests/threads/alarm-priority.result

alarm-zero:
	$(MAKE) build/tests/threads/alarm-zero.result

alarm-negative:
	$(MAKE) build/tests/threads/alarm-negative.result

Sync: sync

sync:
	$(MAKE) sync-clean
	$(MAKE) sync-check

# Build only the result files for the synchronization priority tests.
sync-tests: $(SYNC_RESULTS)

# Print PASS/FAIL output for the synchronization priority tests only.
sync-check: sync-tests
	@failures=0; count=0; \
	for test in $(SYNC_TESTS); do \
		count=$$((count + 1)); \
		if echo PASS | cmp -s build/$$test.result -; then \
			echo "pass $$test"; \
		else \
			echo "FAIL $$test"; \
			failures=$$((failures + 1)); \
		fi; \
	done; \
	if [ $$failures -eq 0 ]; then \
		echo "All $$count synchronization tests passed."; \
	else \
		echo "$$failures of $$count synchronization tests failed."; \
		exit 1; \
	fi

# Remove only the generated artifacts for the synchronization priority tests.
sync-clean:
	rm -f $(addprefix build/,$(addsuffix .result,$(SYNC_TESTS)))
	rm -f $(addprefix build/,$(addsuffix .output,$(SYNC_TESTS)))
	rm -f $(addprefix build/,$(addsuffix .errors,$(SYNC_TESTS)))

priority-sema:
	$(MAKE) build/tests/threads/priority-sema.result

priority-condvar:
	$(MAKE) build/tests/threads/priority-condvar.result

priority-donate-sema:
	$(MAKE) build/tests/threads/priority-donate-sema.result
