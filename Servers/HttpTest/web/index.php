<!DOCTYPE html>
<html>
<body>

<?php
echo "我的第一 php 文件";
foreach($_SERVER as $k=>$v)
{
	echo "key=".$k." val=".$v;
	echo "<br>";
}
?>

</body>
</html>