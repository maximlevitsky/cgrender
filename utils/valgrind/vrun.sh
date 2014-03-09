#! /bin/bash

export G_SLICE=always-malloc
export QT_NO_GLIB=1

VALGRIND_SUPPRESSION_FILES_LIST=""
for SUPPRESSION_FILE in ./vg_suppressions/*.supp; do
  VALGRIND_SUPPRESSION_FILES_LIST+=" --suppressions=$SUPPRESSION_FILE"
done


valgrind --leak-check=full --show-reachable=no --error-limit=no \
	$VALGRIND_SUPPRESSION_FILES_LIST  $@
