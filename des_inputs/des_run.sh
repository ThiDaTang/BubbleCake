
./des_display &

display_pid=$!

./des_controller $display_pid &

controller_pid=$!

./des_inputs $controller_pid

input_pid=$!