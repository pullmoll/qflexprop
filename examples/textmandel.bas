'
' translated to BASIC from mandelbrot16-20180406-fds.spin
'-------------------------------------------------------------------------------

'++
  const xmin = -2.1
  const xmax =  0.7

  const ymin = -1.2
  const ymax =  1.2

  const maxiter = 32

  const MPX = 79 ' 0..79
  const MPY = 24 ' 0..24

  const dx = (xmax-xmin)/MPX
  const dy = (ymax-ymin)/MPY

  const c4 = 4.0 ' square of escape radius
'--

'++
' was main
'
  dim as single x,y,x2,y2,cx,cy
  dim as integer iter

  cy = ymin
  for py = 0 to MPY
    cx = xmin
    for px = 0 to MPX
      x = 0.0
      y = 0.0
      x2 = 0.0
      y2 = 0.0
      iter = 0
      while iter < maxiter and x2+y2 <= c4
	y = 2.0*x*y+cy
	x = x2-y2+cx
	iter = iter+1
	x2 = x*x
	y2 = y*y
      end while
      cx = cx+dx
      print \(iter+32);
    next px
    cy = cy+dy
    print
  next py
'--