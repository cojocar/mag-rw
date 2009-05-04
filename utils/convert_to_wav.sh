#convertgeste un fisier cu date raw (impachetate) intr-un wav
if test $# -ne 1; then
	echo "numar invalid de parametri"
	exit 1
fi

if !(test -e ${1}); then
	echo "Fisier inexistent ${1}"
	exit 1
fi
./unpack < ${1} > ${1}_tmp.unpacked
echo "; Sample Rate 44100" > ${1}_tmp.dat
echo "; Channels 1" >> ${1}_tmp.dat
awk '{printf("%d %d\n", NR, $1)}' < ${1}_tmp.unpacked >> ${1}_tmp.dat
sox ${1}_tmp.dat -c 1 ${1}.wav
rm ${1}_tmp.dat
rm ${1}_tmp.unpacked
