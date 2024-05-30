#include "usart.hpp"
#include <iostream>

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdarg.h>


//串口初始化
Usart::Usart(const char* name, int bud)
{
    device_name = name;
    bud_rate = bud;
    serialOpen();
}

Usart::~Usart()
{
    close(fd);
}

//打开串口
int Usart::serialOpen()
{
  struct termios options ;
  speed_t myBaud ;
  int     status;

  switch (bud_rate)
  {
    case     50:	myBaud =     B50 ; break ;
    case     75:	myBaud =     B75 ; break ;
    case    110:	myBaud =    B110 ; break ;
    case    134:	myBaud =    B134 ; break ;
    case    150:	myBaud =    B150 ; break ;
    case    200:	myBaud =    B200 ; break ;
    case    300:	myBaud =    B300 ; break ;
    case    600:	myBaud =    B600 ; break ;
    case   1200:	myBaud =   B1200 ; break ;
    case   1800:	myBaud =   B1800 ; break ;
    case   2400:	myBaud =   B2400 ; break ;
    case   4800:	myBaud =   B4800 ; break ;
    case   9600:	myBaud =   B9600 ; break ;
    case  19200:	myBaud =  B19200 ; break ;
    case  38400:	myBaud =  B38400 ; break ;
    case  57600:	myBaud =  B57600 ; break ;
    case 115200:	myBaud = B115200 ; break ;
    case 230400:	myBaud = B230400 ; break ;
    default:
      return -2 ;
  }

  if ((fd = open (device_name, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1)
    return -1 ;

  fcntl (fd, F_SETFL, O_RDWR) ;

// Get and modify current options:
  tcgetattr (fd, &options) ;

    cfmakeraw   (&options) ;
    cfsetispeed (&options, myBaud) ;
    cfsetospeed (&options, myBaud) ;

    options.c_cflag |= (CLOCAL | CREAD) ;
    options.c_cflag &= ~PARENB ;
    options.c_cflag &= ~CSTOPB ;
    options.c_cflag &= ~CSIZE ;
    options.c_cflag |= CS8 ;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ;
    options.c_oflag &= ~OPOST ;

    options.c_cc [VMIN]  =   0 ;
    options.c_cc [VTIME] = 100 ;	// Ten seconds (100 deciseconds)

  tcsetattr (fd, TCSANOW | TCSAFLUSH, &options) ;

  ioctl (fd, TIOCMGET, &status);

  status |= TIOCM_DTR ;
  status |= TIOCM_RTS ;

  ioctl (fd, TIOCMSET, &status);

  usleep (1000) ;	// 1mS

  return 0 ;
}

int Usart::serialWrite(const char* data, int length)
{
    write(fd, data, length);
    return 0;
}

void Usart::serialPrintf (const char *message, ...)
{
  va_list argp;
  char buffer [1024];
  
  va_start (argp, message);
    vsnprintf (buffer, 1023, message, argp);
  va_end (argp);

  serialWrite(buffer, strlen(buffer));
}

int Usart::serialDataReady()
{
    int data_length; //缓冲区中待读取待数据长度
    if (ioctl (fd, FIONREAD, &data_length) == -1)
            return -1;
    
    return data_length;
}

int Usart::serialRead(void* data_read, int length)
{
    if (read (fd, data_read, length) != 1)
        return -1 ;

    return 0;
}

int Usart::serialGetChar()
{
    uint8_t x ;

    if (read (fd, &x, 1) != 1)
        return -1 ;

    return ((int)x);
}




