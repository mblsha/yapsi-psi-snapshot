#!/bin/sh

yapsi_revision="../yapsi_revision"
revision_file="tools/yastuff/yapsi_revision.h"

update_revision_file() {
cat > $revision_file <<EOF
static QString YAPSI_VERSION="`cat ../yapsi_version`";
static const int YAPSI_REVISION=$1;
EOF
	# echo "static const int YAPSI_REVISION=$1;" > $revision_file
	echo "'$revision_file' successfully updated."
}

get_revision() {
	echo "Getting current revision information from '$1'..."
	revision=`LANG=C svn info $1 | grep 'Last Changed Rev' | awk {'print \$4;'}`
	if [ "x$revision" = "x" ]; then
		echo -e "\tFailed."
	else
		update_revision_file $revision
		exit
	fi
}

if [ -f $yapsi_revision ]; then
	revision=`cat $yapsi_revision`

	if [ "x$revision" != "x" ]; then
		update_revision_file $revision
		exit
	fi
fi

get_revision "."
get_revision "https://svn.yandex.ru/online/trunk/yapsi/"
update_revision_file "0"
