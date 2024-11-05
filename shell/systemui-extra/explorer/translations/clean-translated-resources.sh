echo "clean qm files..."
for file in $(dirname $0)/*/*.qm
do
	echo "delete $file"
	rm -f ${file}
done
echo "done"
