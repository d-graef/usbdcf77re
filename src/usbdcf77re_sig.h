/*

License : GPL-2.0

https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

(c) 2024 Detlef Graef
(c) 2024 Dipl.-Ing. (FH) Rainer Reusch, Reusch Elektronik

 signal handler

*/


void handle_sigint(int sig) 
{ 

	exit_flag = sig;				// setting global var for exit

} 


void handle_sigterm(int sig) 
{ 

	exit_flag = sig;				// setting global var for exit

} 


