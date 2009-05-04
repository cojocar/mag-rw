	# Gnuplot script file for plotting data in file "force.dat"
	# This file is called   force.p
	set   autoscale                        # scale axes automatically
	unset log                              # remove any log-scaling
	unset label                            # remove any previous labels
	set xtic auto                          # set xtics automatically
	set ytic auto                          # set ytics automatically
	set title "PM"
	set xlabel "Time"
	set ylabel "Voltage(V)"
	set xr [0:20000]
	set yr [0:2]
	plot "f7000" using 1 title 'Comp' with linespoints;
		
