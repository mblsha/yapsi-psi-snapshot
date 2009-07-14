#!/bin/bash

copy_qt_translations() {
	qtdir=$1
	for lang in "ru"; do
		file="$qtdir/translations/qt_$lang.qm"
		if test -a $file; then
			echo "Copying '$file'..."
			cp $file ../lang
		fi
	done
}

# copy_qt_translations "/usr/share/qt4"
# copy_qt_translations $QTDIR
