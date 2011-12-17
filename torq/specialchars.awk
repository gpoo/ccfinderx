{
	hexVal = $4;
	hexVal = substr(hexVal, 4, length(hexVal) - 4)
	printf "STD pair<STD string, MYWCHAR_T>(\"%s\", 0x%s),\n", $2, hexVal
}
