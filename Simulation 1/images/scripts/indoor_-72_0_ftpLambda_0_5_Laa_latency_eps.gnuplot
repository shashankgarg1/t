 
 set style line 1 pt 4 lt 1 
 set style line 2 pt 7 lt 2 
 set style increment 
 
 set pointsize 2 
 set grid
 
 set key bottom right 
 set term postscript eps enhanced   color   
 set output "images/ps/indoor_-72_0_ftpLambda_0_5_Laa_latency.eps" 
    set xlabel "Latency [ms]"
 set ylabel "CDF"
 set title "EDThresh=-72.0, FtpLambda=0.5, CellA=Laa, UDP" 
  
 unset title
 plot [0:500][0:1] "results/cdf_latency_eD_-72.0_ftpLambda_0.5_cellA_Laa_A" using ($1):($2)  with linespoints ls 1  title "operator A (LAA)"  , "results/cdf_latency_eD_-72.0_ftpLambda_0.5_cellA_Laa_B" using ($1):($2)  with linespoints ls 2  title "operator B (Wi-Fi)"  
