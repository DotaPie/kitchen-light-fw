#ifndef CONSOLE_H
#define CONSOLE_H

#define CONSOLE_SERIAL Serial

#define CONSOLE(x) {CONSOLE_SERIAL.print(x);}
#define CONSOLE_CRLF(x) {CONSOLE_SERIAL.println(x);}

#endif