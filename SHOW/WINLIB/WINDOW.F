        subroutine window ( windowno, topx, topy, xpixels, ypixels,
     +                      fgcolor, bgcolor, title, fontname )
        implicit integer(a-z)
c
c  window   - define a window
c
        integer imsg(11), length, rmessage(6), status, dummy
        integer registers(8)
	integer *1 windowname(30), zerost(40)
c
        imsg(1) = 0                     ! define window
        imsg(2) = identp(dummy)         ! my page zero
        imsg(3) = windowno              ! window number
        imsg(4) = topx
        imsg(5) = topy
        imsg(6) = xpixels
        imsg(7) = ypixels
        imsg(8) = fgcolor
        imsg(9) = bgcolor
	call zero ( title, windowname )
        imsg(10) = adr(windowname)
        call zero ( fontname, zerost )
        imsg(11) = adr(zerost)
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) then 
	  length = getmes ( rmessage )
          registers(1) = 41             ! SETTEXTCOLOR
          registers(2) = ((bgcolor-1) * 256 ) + (fgcolor-1)
          call xop ( 6, registers )     ! VIDEOXOP
        endif
        return
        end
