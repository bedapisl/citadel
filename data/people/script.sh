#!/bin/bash

for NAME in `ls | grep conf`;
do 
	cp "$NAME" zaloha_"$NAME"

	SHORT_NAME=`echo $NAME | sed 's@.conf@@g'`
	
	cat "$NAME" | sed 's@image = "/data@image = "data@g' > tmp_"$NAME"
	mv tmp_"$NAME" "$NAME"


	#echo -e '[description]\nname = "'$SHORT_NAME'"\ntext = ""\nimage = data/images/"'$SHORT_NAME'"\n\n[cost]\nweapons = \nbows = \n plate_armour = \n\n[stats]\nlife = \narmor = \nattack = ' >> $NAME

	

	#cat "$NAME" | sed 's@name = "\(.\)@name = "\U\1@' > tmp"$NAME"
	#mv tmp"$NAME" "$NAME"
	

	#cat $NAME | sed 's@image = "/home/images/"@image = "/data/images/'$NAME'.png"@g' > tmp$NAME
	#mv tmp$NAME $NAME

done
	
rm zaloha_zaloha*



