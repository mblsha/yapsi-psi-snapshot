unix {
	mac:app_bundle {
		EXEC_TARGET = $${TARGET}.app/Contents/MacOS/$${TARGET}
	}
	else {
		EXEC_TARGET = $$TARGET
	}

	VALGRIND_SUPPRESSIONS = $$system(for x in $$PWD/suppressions/*.supp; do echo "--suppressions=$x"; done)

	# valgrind target (only shows valgrind output)
	VALGRIND_OPTIONS = -q --num-callers=40 --leak-check=full --show-reachable=yes $$VALGRIND_SUPPRESSIONS
	QMAKE_EXTRA_TARGETS += valgrind
	valgrind.depends = $$EXEC_TARGET
	valgrind.commands = YACHATDATADIR=~/.yachat-test valgrind $$VALGRIND_OPTIONS ./$$EXEC_TARGET | grep -E '==\d+=='

	# valgrind_supp target (generate suppressions)
	QMAKE_EXTRA_TARGETS += valgrind_supp
	valgrind_supp.depends = $$EXEC_TARGET
	valgrind_supp.commands = YACHATDATADIR=~/.yachat-test valgrind $$VALGRIND_OPTIONS --gen-suppressions=all ./$$EXEC_TARGET

	# callgrind profiling
	QMAKE_EXTRA_TARGETS += callgrind
	callgrind.depends = $$EXEC_TARGET
	callgrind.commands = YACHATDATADIR=~/.yachat-test valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./$$EXEC_TARGET
}
