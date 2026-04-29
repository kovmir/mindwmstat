#!/usr/bin/env bash
# Unit tests for mindwmstat.
# Usage: ./test.sh /path/to/batt /path/to/ac /path/to/meminfo

rm -f "$1"
rm -f "$2"
cat <<EOF > "$3"
MemTotal:       32782588 kB
MemFree:        24724076 kB
MemAvailable:   27492784 kB
Buffers:          203028 kB
Cached:          2917864 kB
SwapCached:            0 kB
Active:          4416308 kB
Inactive:        2154364 kB
Active(anon):    3512792 kB
Inactive(anon):        0 kB
EOF

if ./mindwmstat 2>/dev/null | perl -ne 'exit 1 unless /^\d+% \| \d\.\d\d \d\.\d\d \d\.\d\d \| \d+-\d+-\d+ \w\w\w \d\d:\d\d$/'; then
	echo "1 OK"
else
	echo "1 NOT OK"
	exit 1
fi

echo 69 > "$1"
echo 1  > "$2"

if ./mindwmstat | perl -ne 'exit 1 unless /^\d+% \| \d\.\d\d \d\.\d\d \d\.\d\d \| \d+-\d+-\d+ \w\w\w \d\d:\d\d \| \+\d+%$/'; then
	echo "2 OK"
else
	echo "2 NOT OK"
	exit 1
fi

echo 0  > "$2"

if ./mindwmstat | perl -ne 'exit 1 unless /^\d+% \| \d\.\d\d \d\.\d\d \d\.\d\d \| \d+-\d+-\d+ \w\w\w \d\d:\d\d \| -\d+%$/'; then
	echo "3 OK"
else
	echo "3 NOT OK"
	exit 1
fi

rm -rf "$1" "$2" "$3"
