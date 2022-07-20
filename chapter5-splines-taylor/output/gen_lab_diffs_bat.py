out_filename = "gen_lab_diffs.bat"

file_prefix = "image_original_"

ref_method_name = "measurements"
methods_names = ["bruneton", "elek", "haber", 
                 "libradtran", "nishita93", "nishita96", 
				 "oneal", "spline", "taylor", "trapezoidal"]
				 
times = ["09h30", "09h45", "10h00", "10h15", "10h30", 
         "10h45", "11h00", "11h15", "11h30", "11h45", 
         "12h00", "12h15", "12h30", "12h45", "13h00", 
		 "13h15", "13h30"]


f = open(out_filename, "w")

for m in range(len(methods_names)):
	for t in range(len(times)):
		ref_filename = "figures/" + file_prefix + times[t] + "_" + ref_method_name + ".png"
		src_filename = "figures/" + file_prefix + times[t] + "_" + methods_names[m] + ".png"
		out_filename = "lab_diffs/lab_diff_" + times[t] + "_" + methods_names[m]
		
		f.write("colorimgdiff " + ref_filename + " " + src_filename + " -o " + out_filename + " -m Lab -p\n")
		
f.write("python.exe gen_avg_delta_e.py")
f.close()