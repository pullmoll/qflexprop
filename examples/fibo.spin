'' simple fibonacci program

#ifndef _BAUD
#define _BAUD 115_200
#endif

CON
#ifdef __P2__
  _clkfreq = 200_000_000
#else
  _clkmode = xtal1 + pll16x
  _clkfreq = 80_000_000
#endif
  baud = _BAUD
  
OBJ
#ifdef __P2__
  ser: "spin/SmartSerial"
#else
  ser: "spin/FullDuplexSerial"
#endif

PUB demo | i, n, t
  ser.start_default(baud)
  _waitms(100)
  repeat i from 1 to 10 step 1
    t := CNT
    n := fiborec(i)
    t := CNT - t
    ser.printf( "fibo(%d) = %d; cycles = %d\n", i, n, t )

'' iterative version
PUB fibolp(n) : r | lastr
  r := 1
  lastr := 0
  repeat n-1
    (lastr,r) := (r, r+lastr)

'' recursive version
PUB fiborec(n)
  return (n < 2) ? n : fiborec(n-1)+fiborec(n-2)
