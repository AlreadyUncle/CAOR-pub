set term postscript eps enhanced color
set output "09.eps"
set size ratio -1
set xrange [-68:570]
set yrange [-414:224]
set xlabel "x [m]"
set ylabel "y [m]"
plot "09.txt" using 1:2 lc rgb "#FF0000" title 'Ground Truth' w lines,"09.txt" using 3:4 lc rgb "#0000FF" title 'Visual Odometry' w lines,"< head -1 09.txt" using 1:2 lc rgb "#000000" pt 4 ps 1 lw 2 title 'Sequence Start' w points
