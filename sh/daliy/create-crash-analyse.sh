#!/bin/sh

function split
{
    OLD_IFS="$IFS"
    IFS="|"
    array=($1)
    IFS="$OLD_IFS"
    echo ${array[*]}
}

if [ ! -n "$1" ]; then
    currdate=$(date +%m-%d)
else
    currdate=$1
fi
rootpath=/data/www/report/crash/$currdate

if [ ! -d "$rootpath" ]; then
    echo path ${rootpath} not exist!
    exit 0
fi

{
echo "
<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"> 
<html xmlns=\"http://www.w3.org/1999/xhtml\"> 
	<head> 
		<meta http-equiv=\"Content-Type\" mrc=\"text/html; charset=utf-8\" /> 
		<title>${currdate}客户端Crash日报</title> 
		<style type=\"text/css\"> 
		* {margin:0;padding:0} 
		#main {margin:0px 0 0 5px} 
		#main ul {width:600px;list-style:none} 
		/*/*/
		#main li {border-top:1px solid #ccc;float:left;height:40px;text-align:left;line-height:40px;padding-left:10px} 
		#main li.t {background-color:#ddd;} 
		#main li.b {border-top:0px;border-bottom:1px solid #ccc}
		</style> 
	</head> 

	<body> 
		<div style=\"width:600px;height:50px;text-align:center;line-height:50px\"><h3>${currdate} 客户端脚本Crash日报</h3></div>
		
		<div id=\"main\"> 
			<ul> 
				<li class=\"t\" style=\"width:250px\">项目</li>
				<li class=\"t\" style=\"width:100px\">Crash数量</li>
				<li class=\"t\" style=\"width:100px\">影响玩家(次)</li>
				<li class=\"t\" style=\"width:100px\"></li>
				$(
					for file in ${rootpath}/*
					do
						if [ -d $file ]; then
							a=`analyse ${file} -r`
							b=(`split ${a}`)
							echo "<li class=\"b\" style=\"width:250px\">${b[0]}</li>"
							echo "<li class=\"b\" style=\"width:100px\">${b[1]}</li>"
							echo "<li class=\"b\" style=\"width:100px\">${b[2]}</li>"
							echo "<li class=\"b\" style=\"width:100px\"><a href=\"${currdate}/${b[0]}/当日汇总.txt\" target=\"_blank\">查看详细</a></li>"
						fi
					done
				)
			</ul> 
		</div> 
	</body> 
</html> 
"
} > /data/www/report/crash/${currdate}-客户端Crash日报.html
