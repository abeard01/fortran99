        subroutine outputwi ( windowno )
        implicit integer(a-z)
c
c  outputwin - output to a certain window
c
        integer imsg(11), length, rmessage(6), status, dummy
	integer color1, color2, registers(8)
c
        imsg(1) = 1                     ! output window
        imsg(2) = identp(dummy)         ! my page zero
        imsg(3) = windowno              ! window number
c
        serverpag = getserv(dummy)
        status = sendme ( serverpag, 22, imsg )
        if ( status .eq. 0 ) then
	  length = getmes ( rmessage )
	  if ( rmessage(1) .ne. 0 ) then
	    color1 = rmessage(1) * 256
	    color2 = iand ( rmessage(1) / 256, 255 ) + color1
	    registers(1) = 41		! set text color
	    registers(2) = color2
	    status = xop ( 6, registers )
	 endif
	endif
        return
        end
