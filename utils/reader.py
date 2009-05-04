
def analyze (text, dct, per):
	decoded = ""
	for j in range(len(text)/per):
		t = 0
		for i in range(0,per - 1):
			# print (ord(text[i + j * per]) - ord('0')), "*", 2 ** i
			t += (ord(text[i + j * per]) - ord('0')) * (2 ** i)
		if(not t in dct):
			# print t, ":", "***"
			decoded += "*"
		else:
			# print t, ":", dct[t]
			decoded += dct[t]
	decoded +=" |:"
	decoded += text[(len(text)/per) * per :]
	decoded +=":|"
	return decoded

ph = int(raw_input("Phase:"))
track = int(raw_input("Track:"))
if(track == 1):
	per = 7
	dct = dict()
	dct[0] = " "
	dct[1] = "!"
	dct[2] = "\""
	dct[3] = "#"	
	dct[4] = "$"	
	dct[5] = "%"	
	dct[6] = "&"
	dct[7] = "\'"
	dct[8] = "("	
	dct[9] = ")"	
	dct[10] = "*"
	dct[11] = "+"
	dct[12] = ","
	dct[13] = "-"	
	dct[14] = "."	
	dct[15] = "/"	
	for i in range(16,25):
		dct[i] = str(i - 16)

	dct[26] = ":"	
	dct[27] = ";"	
	dct[28] = "<"	
	dct[29] = "="	

	dct[30] = ">"	
	dct[31] = "?"	
	dct[32] = "@"	
	dct[33] = "A"	
	dct[34] = "B"	
	dct[35] = "C"	
	dct[36] = "D"	
	dct[37] = "E"	
	dct[38] = "F"	
	dct[39] = "G"	

	dct[40] = "H"	
	dct[41] = "I"	
	dct[42] = "J"	
	dct[43] = "K"	
	dct[44] = "L"	
	dct[45] = "M"	
	dct[46] = "N"	
	dct[47] = "O"	
	dct[48] = "P"	
	dct[49] = "Q"	

	dct[50] = "R"	
	dct[51] = "S"	
	dct[52] = "T"	
	dct[53] = "U"	
	dct[54] = "V"	
	dct[55] = "W"	
	dct[56] = "X"	
	dct[57] = "Y"	
	dct[58] = "Z"	
	dct[59] = "["	

	dct[60] = "\\"	
	dct[61] = "]"	
	dct[62] = "^"	
	dct[63] = "_"	

if(track == 2):
	per = 5
	dct = dict()
	for i in range(10):
		dct[i] = str(i)
	dct[10] = ":"
	dct[11] = ";"
	dct[12] = "<"
	dct[13] = "="
	dct[14] = ">"
	dct[15] = "?"

text = ""
text = raw_input("Text:")
text = text[ph:]
# print ph,"|", per,"|", text
print
print analyze(text,dct,per)
print 
print 
