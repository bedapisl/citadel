#!/bin/bash

for NAME in `ls | grep conf`;
do 
	cp "$NAME" zaloha_"$NAME"
	
	#echo -e '[carrier_output]\nnumber_of_carriers = 2\ncapacity = 2\n' >> "$NAME"

	#echo -e '[first upgrade]\nfirst_wood = 0\nfirst_stone = 0\nfirst_marble = 0\nfirst_bricks = 0\n\n' >> "$NAME"
	#echo -e '[second upgrade]\nsecond_wood = 0\nsecond_stone = 0\nsecond_marble = 0\nsecond_bricks = 0\n\n' >> "$NAME"
	#echo -e '[third upgrade]\nthird_wood = 0\nthird_stone = 0\nthird_marble = 0\nthird_bricks = 0\n\n' >> "$NAME"


	#echo -e '\n[drawing]\nheight_of_life_bar = 128\nnumber_of_floors = 1\n' >> $NAME

	#cat "$NAME" | sed 's@honour_price = 10@honour_price = 10\ncan_be_upgraded = true@g' > tmp_"$NAME"
	#mv tmp_"$NAME" "$NAME"

	cat "$NAME" | sed 's@upgrade_text@upgrade_info@g' > tmp_"$NAME"
	mv tmp_"$NAME" "$NAME"

	#cat "$NAME" | sed 's@name = "\(.\)@name = "\U\1@' > tmp"$NAME"
	#mv tmp"$NAME" "$NAME"
	

	#cat $NAME | sed 's@image = "/home/images/"@image = "/data/images/'$NAME'.png"@g' > tmp$NAME
	#mv tmp$NAME $NAME

done
	
rm zaloha_zaloha*



