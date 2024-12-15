#time 7z a chrome_7z.zip /opt/google/chrome >> chrome_test.txt

#time zip -r chrome_zip.zip /opt/google/chrome/

function collect_7z_time()
{
	time 7z a $3_7z.$1 $2 > null
	get_file_size $3_7z.$1
	compressed_size=`get_file_size $3_7z.$1`
	before_size=`get_file_size $2`
	percent=`echo "scale=2; $compressed_size *100 / $before_size" | bc`
	echo 7z"->"$1 "$compressed_size"Mb "$before_size"Mb "$percent"%
}

function collect_zip_time()
{
	time zip -r $3_zip.$1 $2 > null
	compressed_size=`get_file_size $3_zip.$1`
	before_size=`get_file_size $2`
	percent=`echo "scale=2; $compressed_size *100 / $before_size" | bc`
	echo zip"->"$1 "$compressed_size"Mb "$before_size"Mb "$percent"%
}

function collect_tar_time()
{
	time tar -cvf $3_tar.$1 $2 > null
	compressed_size=`get_file_size $3_tar.$1`
	before_size=`get_file_size $2`
	percent=`echo "scale=2; $compressed_size *100 / $before_size" | bc`
	echo tar"->"$1 "$compressed_size"Mb "$before_size"Mb "$percent"%
}

function collect_targz_time()
{
	time tar -zcvf $3_tar.$1 $2 > null
	compressed_size=`get_file_size $3_tar.$1`
	before_size=`get_file_size $2`
	percent=`echo "scale=2; $compressed_size *100 / $before_size" | bc`
	echo tar"->"$1 "$compressed_size"Mb "$before_size"Mb "$percent"%
}

function get_file_size()
{
	#file_size=$(du -m $1)
	file_size=$(du -m $1 | sed -n '$p' | awk '{print $1}')
	echo $file_size
}

function testAllTool()
{
	collect_7z_time zip $1 $2
	collect_zip_time zip $1 $2
	collect_7z_time tar $1 $2
	collect_tar_time tar $1 $2
	collect_targz_time tar.gz $1 $2	
}

#testAllTool chrome/ chrome
#testAllTool GoogleEarth/ GoogleEarth
testAllTool /home/archermind/Qt5.13.1/5.13.1/Src/qtbase qtbase
#testAllTool /home/archermind/Qt5.13.1/5.13.1/Src qt513src
#testAllTool pictures/ pictures
#testAllTool office/ office
#testAllTool Avatar.mp4 Avatar