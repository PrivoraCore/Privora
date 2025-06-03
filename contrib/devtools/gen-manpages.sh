#!/bin/sh

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

PRIVORAD=${PRIVORAD:-$SRCDIR/privorad}
PRIVORACLI=${PRIVORACLI:-$SRCDIR/privora-cli}
PRIVORATX=${PRIVORATX:-$SRCDIR/privora-tx}
PRIVORAQT=${PRIVORAQT:-$SRCDIR/qt/privora-qt}

[ ! -x $PRIVORAD ] && echo "$PRIVORAD not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
VORAVER=($($PRIVORACLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for privorad if --version-string is not set,
# but has different outcomes for privora-qt and privora-cli.
echo "[COPYRIGHT]" > footer.h2m
$PRIVORAD --version | sed -n '1!p' >> footer.h2m

for cmd in $PRIVORAD $PRIVORACLI $PRIVORATX $PRIVORAQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${VORAVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${VORAVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
