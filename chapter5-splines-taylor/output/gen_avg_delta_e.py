methods_names = ["bruneton", "elek", "haber", 
                 "libradtran", "nishita93", "nishita96", 
				 "oneal", "spline", "taylor", "trapezoidal"]
				 
times = ["09h30", "09h45", "10h00", "10h15", "10h30", 
         "10h45", "11h00", "11h15", "11h30", "11h45", 
         "12h00", "12h15", "12h30", "12h45", "13h00", 
		 "13h15", "13h30"]

for m in range(len(methods_names)):
	
	avg_delta_e = 0.0
	for t in range(len(times)):
		filename = "lab_diffs/lab_diff_" + times[t] + "_" + methods_names[m] + "_delta_e.txt"
		f = open(filename, "r")
		delta_e = f.read()
		f.close()
		
		avg_delta_e = avg_delta_e + float(delta_e)
		
	# Calc avg delta e and save to a file
	avg_delta_e = avg_delta_e / float(len(times))
	
	filename = "lab_diffs/lab_diff_" + methods_names[m] + "_avg_delta_e.txt"
	f = open(filename, "w")
	f.write(str("%.3f" % avg_delta_e))
	f.close()