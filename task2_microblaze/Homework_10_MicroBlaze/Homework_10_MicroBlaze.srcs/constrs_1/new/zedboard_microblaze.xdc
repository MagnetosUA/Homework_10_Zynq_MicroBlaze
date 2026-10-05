## 100 MHz system clock
set_property PACKAGE_PIN Y9 [get_ports sys_clock]
set_property IOSTANDARD LVCMOS33 [get_ports sys_clock]
create_clock -period 10.000 -name sys_clock [get_ports sys_clock]

## LEDs
set_property PACKAGE_PIN T22 [get_ports {GPIO_0_tri_o[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_0_tri_o[0]}]

set_property PACKAGE_PIN T21 [get_ports {GPIO_0_tri_o[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_0_tri_o[1]}]

set_property PACKAGE_PIN U22 [get_ports {GPIO_0_tri_o[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_0_tri_o[2]}]

set_property PACKAGE_PIN U21 [get_ports {GPIO_0_tri_o[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_0_tri_o[3]}]

## Buttons
set_property PACKAGE_PIN P16 [get_ports {GPIO_1_tri_i[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_1_tri_i[0]}]

set_property PACKAGE_PIN R16 [get_ports {GPIO_1_tri_i[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_1_tri_i[1]}]

set_property PACKAGE_PIN N15 [get_ports {GPIO_1_tri_i[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO_1_tri_i[2]}]

## SW0
set_property PACKAGE_PIN F22 [get_ports {GPIO2_0_tri_i[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {GPIO2_0_tri_i[0]}]