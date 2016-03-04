# AVR_projects  
  
## ds18x20_demo - http://homepage.hispeed.ch/peterfleury/avr-software.html  
- cd default
- make
- sudo make flash
  
## uart_lib - http://homepage.hispeed.ch/peterfleury/avr-software.html  
- make -f Makefile.uart  
- sudo make -f Makefile.uart program
  
## prototype2012_board  
- make  
- sudo make program  
- Sending commands through serial port  (RS232 or RS485)
`stty -F /dev/ttyUSB0 9600`  
`echo '7f00010200aaaa0000' | xxd -r -p > /dev/ttyUSB0`  







