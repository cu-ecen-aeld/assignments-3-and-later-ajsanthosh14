#! /bin/sh

# Reference:
#	1. Mastering Embedded Linux Programming: Chapter 10 (Adding a new daemon)
#	2. https://man7.org/linux/man-pages/man8/start-stop-daemon.8.html (Example)

case "$1" in
  start)
    echo "Starting aesdsocket"
    start-stop-daemon -S -n aesdsocket -x /usr/bin/aesdsocket -- -d
    ;;
  stop)
    echo "Stopping aesdsocket"
    start-stop-daemon -K -n aesdsocket -s SIGTERM 
    ;;
  *)
    echo "Usage: $0 {start|stop}"
  exit 1
esac

exit 0
