SERVER_DIR=~/git/bzrflag-server
GRID_DIR=~/git/bzrflag/Grid
$SERVER_DIR/bin/bzrflag --default-true-positive=.97 --default-true-negative=.9 --occgrid-width=100 --no-report-obstacles --world=$SERVER_DIR/maps/four_ls.bzw  --red-port=50100 --green-port=50101 --purple-port=50102 --blue-port=50103  $@&
sleep 2
$GRID_DIR/Debug/Grid
